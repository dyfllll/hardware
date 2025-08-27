#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <stdint.h>
#include <poll.h>
#include <signal.h>
/*
 *argc:应用程序参数个数
 *argv[]:具体的参数内容，字符串形式
 *./keyinputAPP  <filename>
 * ./keyinputAPP /dev/input/event6
 */

/* input_event结构体变量 */
static struct input_event inputevent;

const char *key_names[] = {"1", "2", "3", "4", "5", "6", "7", "8",
                           "9", "0", "*", "#", "^", "<", "v", ">",
                           "OK"};
const uint32_t key_codes[] = {KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
                              KEY_NUMERIC_STAR, KEY_NUMERIC_POUND,
                              KEY_UP, KEY_LEFT, KEY_DOWN, KEY_RIGHT,
                              KEY_ENTER};

static const char *code_to_name(uint32_t rc)
{
    for (int i = 0; i < sizeof(key_codes) / sizeof(key_codes[0]); i++)
    {
        if (rc == key_codes[i])
        {
            return key_names[i];
        }
    }
    return "Error";
}

#if 0
//阻塞
int main(int argc, char *argv[])
{
    int fd, err;
    char *filename;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", filename);
        return -1;
    }

    while (1)
    {
        const char *key_name = "";
        err = read(fd, &inputevent, sizeof(inputevent));
        if (err > 0)
        {

            /* 数据读取成功 */
            switch (inputevent.type)
            {
            case EV_KEY:
                /* KEY */
                key_name = code_to_name(inputevent.code);
                printf("key %d %s %s\r\n", inputevent.code, key_name, inputevent.value ? "press" : "release");
                break;
            case EV_SYN:
                break;
            case EV_REL:
                break;
            case EV_ABS:
                break;
            }
        }
        else
        {
            printf("读取数据失败\r\n");
        }

        if (key_name == "OK")
            break;
    }

    close(fd);

    return 0;
}
#endif

#if 0
// poll
int main(int argc, char *argv[])
{
    int fd, err;
    char *filename;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\r\n", filename);
        return -1;
    }

    struct pollfd fds[1];

    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (1)
    {
        const char *key_name = "";
        int ret = poll(fds, 1, 1000); // 1s超时
        if (ret > 0 && (fds[0].revents & POLLIN))
        {
            err = read(fd, &inputevent, sizeof(inputevent));
            if (err > 0)
            {

                /* 数据读取成功 */
                switch (inputevent.type)
                {
                case EV_KEY:
                    /* KEY */
                    key_name = code_to_name(inputevent.code);
                    printf("key %d %s %s\r\n", inputevent.code, key_name, inputevent.value ? "press" : "release");
                    break;
                case EV_SYN:
                    break;
                case EV_REL:
                    break;
                case EV_ABS:
                    break;
                }
            }
            else
            {
                printf("读取数据失败\r\n");
            }

            if (key_name == "OK")
                break;
        }
        else if (ret == 0)
        {
            // printf("timeout, no event\n");
        }
    }

    close(fd);

    return 0;
}
#endif

// 异步
uint8_t flag;
int async_fd;
void sigio_handler(int sig)
{
    int n;
    do
    {
        const char *key_name = "";
        n = read(async_fd, &inputevent, sizeof(inputevent));
        if (n == sizeof(inputevent))
        {
            /* 数据读取成功 */
            switch (inputevent.type)
            {
            case EV_KEY:
                /* KEY */
                key_name = code_to_name(inputevent.code);
                printf("key %d %s %s\r\n", inputevent.code, key_name, inputevent.value ? "press" : "release");
                break;
            case EV_SYN:
                break;
            case EV_REL:
                break;
            case EV_ABS:
                break;
            }
        }
        else
        {
            // printf("读取数据失败\r\n");
        }

        if (key_name == "OK")
            flag = 1;
    } while (n > 0);
}

int main(int argc, char *argv[])
{
    int fd, err;
    char *filename;

    if (argc != 2)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    fd = open(filename, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    async_fd = fd;

    // 设置异步通知
    fcntl(fd, F_SETOWN, getpid()); // 设置当前进程接收 SIGIO
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_ASYNC | O_NONBLOCK);

    // 注册 SIGIO 信号处理函数
    struct sigaction sa;
    sa.sa_handler = sigio_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGIO, &sa, NULL);

    printf("Waiting for input events...\n");
    while (!flag)
    {
        pause(); // 挂起，直到有信号（SIGIO）唤醒
        printf("SIGIO...\n");
        // sleep(2);
    }

    close(fd);
    return 0;
}