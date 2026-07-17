#include "eth_lwip.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "common.h"
#include <stdio.h>
#include <stdarg.h>
#include "lwip.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/ip_addr.h"
#include "lan8742.h"

extern ETH_HandleTypeDef heth;
extern lan8742_Object_t LAN8742;
extern struct netif gnetif;
static uint8_t dhcp_assigned_flag = 0; // 标记是否成功获取 IP

void check_dhcp_status(void)
{
    // 1. 检查网卡是否已连接且处于活动状态
    if (netif_is_up(&gnetif))
    {

        // 2. 检查 DHCP 模块是否分配成功 (IP 不能为空/0)
        if (gnetif.ip_addr.addr != 0)
        {

            if (dhcp_assigned_flag == 0)
            {
                dhcp_assigned_flag = 1; // 避免重复打印

                // 3. 提取 IP 地址、子网掩码和网关

                printf("DHCP Success!\r\n");

                uint8_t ip0 = ip4_addr1_16(&gnetif.ip_addr);
                uint8_t ip1 = ip4_addr2_16(&gnetif.ip_addr);
                uint8_t ip2 = ip4_addr3_16(&gnetif.ip_addr);
                uint8_t ip3 = ip4_addr4_16(&gnetif.ip_addr);
                printf("IP Address : %d.%d.%d.%d\r\n", ip0, ip1, ip2, ip3);

                uint8_t mask0 = ip4_addr1_16(&gnetif.netmask);
                uint8_t mask1 = ip4_addr2_16(&gnetif.netmask);
                uint8_t mask2 = ip4_addr3_16(&gnetif.netmask);
                uint8_t mask3 = ip4_addr4_16(&gnetif.netmask);
                printf("Netmask : %d.%d.%d.%d\r\n", mask0, mask1, mask2, mask3);

                uint8_t gw0 = ip4_addr1_16(&gnetif.gw);
                uint8_t gw1 = ip4_addr2_16(&gnetif.gw);
                uint8_t gw2 = ip4_addr3_16(&gnetif.gw);
                uint8_t gw3 = ip4_addr4_16(&gnetif.gw);
                printf("Gateway : %d.%d.%d.%d\r\n", gw0, gw1, gw2, gw3);

                // 在这里可以安全地启动你的 TCP/UDP 业务（例如：tcp_server_init()）
                eth_lwip_start();
            }
        }
        else
        {
            // 还在等待 DHCP 分配
            if (dhcp_assigned_flag == 1)
            {
                printf("IP Address Lost...\r\n");
                dhcp_assigned_flag = 0;
            }
        }
    }
}

static void lwiperf_report_cb(void *arg, enum lwiperf_report_type report_type,
                              const ip_addr_t *local_addr, u16_t local_port,
                              const ip_addr_t *remote_addr, u16_t remote_port,
                              u32_t bytes_transferred, u32_t ms_duration,
                              u32_t bandwidth_kbitpersec)
{
    printf("\r\n--- iPerf Test Report ---\r\n");
    printf("Local address: %s port %d\r\n", ipaddr_ntoa(local_addr), local_port);
    printf("Remote address: %s port %d\r\n", ipaddr_ntoa(remote_addr), remote_port);
    printf("Bytes transferred: %d bytes\r\n", bytes_transferred);
    printf("Duration: %d ms\r\n", ms_duration);
    // 打印带宽（Mbps）
    printf("Bandwidth: %d.%03d Mbps\r\n", bandwidth_kbitpersec / 1000, bandwidth_kbitpersec % 1000);
    printf("------------------------\r\n");
}

void eth_lwip_reset(void)
{
    // HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_SET);
    // HAL_Delay(100);
    HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(55);
    HAL_GPIO_WritePin(ETH_RST_GPIO_Port, ETH_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(55);
    printf("eth_lwip_reset\n");
}

void eth_lwip_init(void)
{
    eth_lwip_reset();

    printf("eth_lwip_init\n");
}

void eth_lwip_start(void)
{
    uint32_t PHYAddr = LAN8742.DevAddr;

    printf("PHYAddr: 0x%08X\r\n", PHYAddr);
    uint16_t phy_id1 = 0, phy_id2 = 0;
    // 假设你的 HAL 库句柄为 heth，PHY 地址为 0
    HAL_ETH_ReadPHYRegister(&heth, PHYAddr, 2, &phy_id1);
    HAL_ETH_ReadPHYRegister(&heth, PHYAddr, 3, &phy_id2);
    printf("PHY ID1: 0x%04X, ID2: 0x%04X\r\n", phy_id1, phy_id2);

    int32_t PHYLinkState = LAN8742_GetLinkState(&LAN8742);
    if (PHYLinkState > LAN8742_STATUS_LINK_DOWN)
    {
        switch (PHYLinkState)
        {
        case LAN8742_STATUS_100MBITS_FULLDUPLEX:
            printf("ethernet_link_check_state 100Mbps Full Duplex\n");
            break;
        case LAN8742_STATUS_100MBITS_HALFDUPLEX:
            printf("ethernet_link_check_state 100Mbps Half Duplex\n");
            break;
        case LAN8742_STATUS_10MBITS_FULLDUPLEX:
            printf("ethernet_link_check_state 10Mbps Full Duplex\n");
            break;
        case LAN8742_STATUS_10MBITS_HALFDUPLEX:
            printf("ethernet_link_check_state 10Mbps Half Duplex\n");
            break;
        default:
            printf("ethernet_link_check_state unknown\n");
            break;
        }

        void *iperf_session = lwiperf_start_tcp_server(IP_ADDR_ANY, LWIPERF_TCP_PORT_DEFAULT, lwiperf_report_cb, NULL);

        if (iperf_session != NULL)
        {
            printf("iPerf TCP Server started successfully on port %d...\r\n", LWIPERF_TCP_PORT_DEFAULT);
        }
        else
        {
            printf("Failed to start iPerf TCP Server!\r\n");
        }
        printf("eth_lwip_start\n");
    }
}

void eth_lwip_update(void)
{
    check_dhcp_status();
    MX_LWIP_Process();
}
