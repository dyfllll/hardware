#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <asm/io.h>
#include <linux/device.h>

#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#define KEYINPUT_NAME "red_nec_input"
/* HX1838 */
typedef enum
{
    RN_Init,
    RN_Start_High_9ms,
    RN_Start_Low_4_5ms,
    RN_Read_State0,
    RN_Read_State1,
    RN_Repeat,
    RN_End,

} red_nec_rec_state;

struct red_nec_rec_struct
{
    red_nec_rec_state state;
    uint8_t value;
    uint64_t time;
    uint32_t receive_buffer;
    uint8_t receive_count;
    uint8_t key_addr, key_data;
};

#define RED_NEC_REC_MAXCOUNT 32

const uint8_t key_red_codes[] = {162, 98, 226, 34, 2, 194, 224, 168,
                                 144, 152, 104, 176, 24, 16, 74, 90,
                                 56};
// const char *key_names[] = {"1", "2", "3", "4", "5", "6", "7", "8",
//                            "9", "0", "*", "#", "^", "<", "v", ">",
//                            "OK"};
const uint32_t key_codes[] = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
                              KEY_NUMERIC_STAR, KEY_NUMERIC_POUND,
                              KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
                              KEY_ENTER};

/* HX1838 */
struct red_nec_dev
{
    struct device_node *nd;
    struct gpio_desc *red_nec_gpio;
    uint32_t irq_num;
    struct input_dev *inputdev;
    struct timer_list timer;
    struct red_nec_rec_struct rec_buffer;
};

struct red_nec_dev red_nec_obj;
struct tasklet_struct red_nec_tasklet;

static int red_to_code(uint8_t rc, uint32_t *code)
{
    for (int i = 0; i < ARRAY_SIZE(key_red_codes); i++)
    {
        if (rc == key_red_codes[i])
        {
            *code = key_codes[i];
            return 0;
        }
    }
    return -1;
}

// static int red_to_name(uint8_t rc, const char **name)
// {
//     for (int i = 0; i < ARRAY_SIZE(key_red_codes); i++)
//     {
//         if (rc == key_red_codes[i])
//         {
//             *name = key_names[i];
//             return 0;
//         }
//     }
//     return -1;
// }

static int red_nec_open(struct input_dev *i_dev)
{
    pr_info("input device opened()\n");
    return 0;
}

static void red_nec_close(struct input_dev *i_dev)
{
    pr_info("input device closed()\n");
}

static irqreturn_t red_nec_irq_hander(int irq, void *dev_id)
{
    tasklet_schedule(&red_nec_tasklet);
    return IRQ_HANDLED;
}

uint8_t Appro(uint64_t cur, uint64_t last, int32_t target)
{
    return abs((int32_t)(cur - last) - target) < 300;
}

static void key_press(struct red_nec_dev *dev)
{
    if (dev->rec_buffer.key_data == 0)
        return;
    uint32_t code;
    if (red_to_code(dev->rec_buffer.key_data, &code))
        return;
    input_event(dev->inputdev, EV_KEY, code, 1);
    input_sync(dev->inputdev);
}

static void key_release(struct red_nec_dev *dev)
{
    if (dev->rec_buffer.key_data == 0)
        return;
    uint32_t code;
    if (red_to_code(dev->rec_buffer.key_data, &code))
        return;
    input_event(dev->inputdev, EV_KEY, code, 0);
    input_sync(dev->inputdev);

    dev->rec_buffer.key_data = 0;
}

void red_nec_tasklet_hander(unsigned long data)
{
    // printk("red_nec_tasklet_hander-----------inter\n");
    struct red_nec_dev *dev = (struct red_nec_dev *)data;
    uint64_t currentTime = ktime_get_ns() / 1000;

    switch (dev->rec_buffer.state)
    {
    case RN_Init:
        dev->rec_buffer.state = RN_Init;
        dev->rec_buffer.receive_buffer = 0;
        dev->rec_buffer.receive_count = 0;
        if (dev->rec_buffer.value == 0 && Appro(currentTime, dev->rec_buffer.time, 9000))
        {
            dev->rec_buffer.state = RN_Start_High_9ms;
        }
        break;
    case RN_Start_High_9ms:
        dev->rec_buffer.state = RN_Init;
        if (dev->rec_buffer.value == 1 && Appro(currentTime, dev->rec_buffer.time, 4500))
        {
            dev->rec_buffer.state = RN_Start_Low_4_5ms;
        }
        else if (dev->rec_buffer.value == 1 && Appro(currentTime, dev->rec_buffer.time, 2250))
        {
            dev->rec_buffer.state = RN_Repeat;
        }
        break;
    case RN_Start_Low_4_5ms:
    case RN_Read_State1:
        dev->rec_buffer.state = RN_Init;
        if (dev->rec_buffer.value == 0 && Appro(currentTime, dev->rec_buffer.time, 560))
        {
            dev->rec_buffer.state = RN_Read_State0;
        }
        break;
    case RN_Read_State0:
        dev->rec_buffer.state = RN_Init;

        if (dev->rec_buffer.value == 1)
        {
            if (Appro(currentTime, dev->rec_buffer.time, 560)) // 接收到0
            {
                dev->rec_buffer.receive_count++;
                if (dev->rec_buffer.receive_count >= RED_NEC_REC_MAXCOUNT)
                    dev->rec_buffer.state = RN_End;
                else
                    dev->rec_buffer.state = RN_Read_State1;
            }
            else if (Appro(currentTime, dev->rec_buffer.time, 1680)) // 接收到1
            {
                //|addr|~addr|data|~data|
                dev->rec_buffer.receive_buffer |= 0x80000000 >> dev->rec_buffer.receive_count;
                dev->rec_buffer.receive_count++;
                if (dev->rec_buffer.receive_count >= RED_NEC_REC_MAXCOUNT)
                    dev->rec_buffer.state = RN_End;
                else
                    dev->rec_buffer.state = RN_Read_State1;
            }
        }
        break;

    case RN_End:
        dev->rec_buffer.state = RN_Init;
        if (dev->rec_buffer.value == 0)
        {
            uint8_t *data_array = (uint8_t *)&dev->rec_buffer.receive_buffer;
            uint8_t addr, data;
            if (data_array[3] == (uint8_t)(~data_array[2]) && data_array[1] == (uint8_t)(~data_array[0]))
            {

                key_release(dev);

                addr = data_array[3];
                data = data_array[1];
                dev->rec_buffer.key_addr = addr;
                dev->rec_buffer.key_data = data;
                
                key_press(dev);

                //自动释放按键
                mod_timer(&dev->timer, jiffies + msecs_to_jiffies(200));
                printk("key addr:%d data:%d\n", addr, data);
            }
        }
        break;
    case RN_Repeat:
        dev->rec_buffer.state = RN_Init;
        if (dev->rec_buffer.value == 0)
        {
            //延长时间
            mod_timer(&dev->timer, jiffies + msecs_to_jiffies(200));
            printk("key repeat\n");
        }
        break;

    default:
        break;
    }

    dev->rec_buffer.time = currentTime;
    dev->rec_buffer.value = !dev->rec_buffer.value;

    // printk("red_nec_tasklet_hander-----------exit\n");
}

static void timer_func(struct timer_list *t)
{
    struct red_nec_dev *dev = from_timer(dev, t, timer);
    key_release(dev);
}

static int red_nec_probe(struct platform_device *dev)
{
    int ret = 0;

    red_nec_obj.nd = dev->dev.of_node;

    // 获取gpio并改成输入模式
    red_nec_obj.red_nec_gpio = devm_gpiod_get(&dev->dev, "button", GPIOD_IN);
    if (IS_ERR(red_nec_obj.red_nec_gpio))
    {
        ret = PTR_ERR(red_nec_obj.red_nec_gpio);
        goto fail_device;
    }
    printk("red_nec gpio num = %d\r\n", desc_to_gpio(red_nec_obj.red_nec_gpio));

    // 获取中断号
    red_nec_obj.irq_num = gpiod_to_irq(red_nec_obj.red_nec_gpio);

    printk("platform_get_irq =  %d \n", red_nec_obj.irq_num);

    if (red_nec_obj.irq_num < 0)
        goto fail_irq;
    // 申请中断
    ret = devm_request_irq(&dev->dev, red_nec_obj.irq_num, red_nec_irq_hander, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "red_nec_interrupt", &red_nec_obj);
    if (ret)
    {
        printk("irq %d request failed!\r\n", red_nec_obj.irq_num);
        goto fail_device;
    }

    /*初始化red_nec_tasklet*/
    tasklet_init(&red_nec_tasklet, red_nec_tasklet_hander, (unsigned long)(&red_nec_obj));

    timer_setup(&red_nec_obj.timer, timer_func, 0);

    // 输入子系统
    red_nec_obj.inputdev = input_allocate_device();
    if (red_nec_obj.inputdev == NULL)
    {
        ret = -EINVAL;
        goto fail_keyinit;
    }

    red_nec_obj.inputdev->open = red_nec_open;
    red_nec_obj.inputdev->close = red_nec_close;

    red_nec_obj.inputdev->name = KEYINPUT_NAME;
    __set_bit(EV_KEY, red_nec_obj.inputdev->evbit); /* 按键事件 */
    __set_bit(EV_REP, red_nec_obj.inputdev->evbit); /* 重复事件 */
    for (size_t i = 0; i < ARRAY_SIZE(key_codes); i++)
    {
        __set_bit(key_codes[i], red_nec_obj.inputdev->keybit);
    }

    ret = input_register_device(red_nec_obj.inputdev);
    if (ret)
    {
        goto fail_input_register;
    }

    red_nec_obj.rec_buffer.state = RN_Init;
    red_nec_obj.rec_buffer.value = 1; // 默认上拉
    red_nec_obj.rec_buffer.time = 0;
    red_nec_obj.rec_buffer.key_addr = 0;
    red_nec_obj.rec_buffer.key_data = 0;

    return 0;
fail_irq:
fail_device:

fail_input_register:
    input_free_device(red_nec_obj.inputdev);
fail_keyinit:
    return ret;
}

static int red_nec_remove(struct platform_device *dev)
{
    /* 删除定时器 */
    del_timer_sync(&red_nec_obj.timer);

    /*注销input_dev */
    input_unregister_device(red_nec_obj.inputdev);
    input_free_device(red_nec_obj.inputdev);

    printk("red_nec_remove\n");
    return 0;
}

static const struct of_device_id pwm_ids[] = {
    {.compatible = "test,red_nec"},
    {/* sentinel */}};

/*定义平台设备结构体*/
struct platform_driver red_nec_platform_driver = {
    .probe = red_nec_probe,
    .remove = red_nec_remove,
    .driver = {
        .name = "red-nec-input",
        .owner = THIS_MODULE,
        .of_match_table = pwm_ids,
    }};

static int __init red_nec_init(void)
{
    printk("red_nec_init\n");
    return platform_driver_register(&red_nec_platform_driver);
}

static void __exit red_nec_exit(void)
{
    platform_driver_unregister(&red_nec_platform_driver);
    printk("red_nec_exit\n");
}

module_init(red_nec_init);
module_exit(red_nec_exit);
MODULE_LICENSE("GPL");
