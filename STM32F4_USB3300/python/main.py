import serial
import serial.tools.list_ports
import time

# 列出所有可用串口
def list_ports():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        print(f"端口: {port.device}, 描述: {port.description}")
    return ports


# 查找USB CDC设备 - 列出所有USB设备供用户选择
def find_usb_cdc():
    while True:
        try:
            ports = serial.tools.list_ports.comports()
            print("\n可用串口:")
            for i, port in enumerate(ports):
                print(f"{i}: 端口: {port.device}, 描述: {port.description}")
            # usb_ports = [p for p in ports if "USB" in p.description.upper()]
            choice = input(f"请选择端口 [0-{len(ports)-1}] 或按回车使用 [0]: ").strip()
            if choice == "":
                continue
            idx = int(choice)
            if 0 <= idx < len(ports):
                return ports[idx].device
            print(f"无效选择，请输入 0-{len(ports)-1}")
        except ValueError:
            print("请输入数字")


def main():
    print("=" * 50)
    print("USB CDC 读取速度测试")
    print("=" * 50)

    port_name = find_usb_cdc()
    if not port_name:
        print("错误: 未找到可用的USB CDC设备!")
        return

    print(f"\n连接到: {port_name}")

    ser = serial.Serial(port_name, 115200, timeout=5)
    ser.set_buffer_size(rx_size=1024 * 1024, tx_size=1024 * 1024)
    print(f"串口已打开: {ser.portstr}")

    MCU_BUFFER_SIZE = 32768  # MCU buffer size
    TOTAL_DATA_SIZE = 512 * 100000
    LOG_COUNT = 20  # 共打印日志次数

    READ_SIZE = MCU_BUFFER_SIZE
    READ_COUNT = int(TOTAL_DATA_SIZE / READ_SIZE)
    print(f"READ_COUNT: {READ_COUNT}")

    DISPLAY_INTERVAL = int(READ_COUNT / LOG_COUNT)

    print(f"\n开始速度测试: 读取{TOTAL_DATA_SIZE} 字节")
    print("-" * 50)

    total_bytes = 0
    check_start_text = ""
    check_end_text = ""

    try:
        start_time = time.perf_counter()

        ser.write("test_start".encode("utf-8"))

        for i in range(READ_COUNT):

            data = ser.read(MCU_BUFFER_SIZE)
            data_len = len(data)

            if data_len > 0:
                total_bytes += data_len
                

                # 每隔一段显示进度
            if (i + 1) % DISPLAY_INTERVAL == 0:
                
                check_start_text = str(int.from_bytes(data[:4], "little")) 
                check_end_text = str(int.from_bytes(data[-4:], "little"))  

                elapsed = time.perf_counter() - start_time
                speed = (total_bytes / elapsed) / (1024 * 1024)  # MB/s
                print(
                    f"进度: {total_bytes}/{TOTAL_DATA_SIZE} | "
                    f"当前速度: {speed:.2f} MB/s | "
                    f"data_len: {data_len} | "
                    f"check_start: '{check_start_text}' | "
                    f"check_end: '{check_end_text}'" 
                    
                )

        # 结束计时
        end_time = time.perf_counter()
        elapsed = end_time - start_time

        # 计算并打印速度
        speed_bps = total_bytes / elapsed
        speed_kbps = speed_bps / 1024
        speed_mbps = speed_bps / (1024 * 1024)

        print("-" * 50)
        print("\n===== 速度测试结果 =====")
        print(f"总数据量:   {total_bytes:,} 字节 ({total_bytes / 1024:.2f} KB) ({total_bytes / 1024 / 1024:.2f} MB)")
        # print(f"check_start: {check_start_text} | check_end: {check_end_text}")
        print(f"总耗时:     {elapsed:.4f} 秒")
        print(f"平均速度:   {speed_bps:,.0f} 字节/秒")
        print(f"平均速度:   {speed_kbps:,.2f} KB/s")
        print(f"平均速度:   {speed_mbps:,.2f} MB/s")
        print("-" * 50)

    except serial.SerialException as e:
        print(f"\n串口错误: {e}")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("\n串口已关闭")


if __name__ == "__main__":
    main()
