/**
 ****************************************************************************************************
 * @file        sdio_sdcard.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-04-20
 * @brief       SD卡 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 * 实验平台:正点原子 阿波罗 F429开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20220420
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "string.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/SDIO/sdio_sdcard.h"


SD_HandleTypeDef g_sdcard_handle;             /* SD卡句柄 */
HAL_SD_CardInfoTypeDef g_sd_card_info_handle; /* SD卡信息结构体 */
DMA_HandleTypeDef hdma_sdio_rx;               /* 读卡DMA句柄(外设→内存) */
DMA_HandleTypeDef hdma_sdio_tx;               /* 写卡DMA句柄(内存→外设) */


/**
 * @brief       初始化SD卡
 * @param       无
 * @retval      返回值:0 初始化正确；其他值，初始化错误
 */
uint8_t sd_init(void)
{
    uint8_t SD_Error;

    /* 初始化时的时钟不能大于400KHZ */
    g_sdcard_handle.Instance = SDIO;
    g_sdcard_handle.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;                       /* 上升沿 */
    g_sdcard_handle.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;                  /* 不使用bypass模式，直接用HCLK进行分频得到SDIO_CK */
    g_sdcard_handle.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;           /* 空闲时不关闭时钟电源 */
    g_sdcard_handle.Init.BusWide = SDIO_BUS_WIDE_1B;                               /* 1位数据线 */
    g_sdcard_handle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE; /* 关闭硬件流控 */
    g_sdcard_handle.Init.ClockDiv = SDIO_TRANSF_CLK_DIV;                           /* SD传输时钟频率最大25MHZ */

    SD_Error = HAL_SD_Init(&g_sdcard_handle);
    if (SD_Error != HAL_OK)
    {
        return 1;
    }
    
    HAL_SD_GetCardInfo(&g_sdcard_handle, &g_sd_card_info_handle);                  /* 获取SD卡信息 */

    SD_Error = HAL_SD_ConfigWideBusOperation(&g_sdcard_handle, SDIO_BUS_WIDE_4B);  /* 使能宽总线模式 */
    if (SD_Error != HAL_OK)
    {
        return 2;
    }
    
    return 0;
}

/**
 * @brief       SDIO底层驱动，时钟使能，引脚配置
                此函数会被HAL_SD_Init()调用
 * @param       hsd  : SD卡句柄
 * @retval      无
 */
void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
    if (hsd->Instance == SDIO)
    {
        GPIO_InitTypeDef gpio_init_struct;

        __HAL_RCC_SDIO_CLK_ENABLE();                        /* 使能SDIO时钟 */

        SD_D0_GPIO_CLK_ENABLE();                            /* D0引脚IO时钟使能 */
        SD_D1_GPIO_CLK_ENABLE();                            /* D1引脚IO时钟使能 */
        SD_D2_GPIO_CLK_ENABLE();                            /* D2引脚IO时钟使能 */
        SD_D3_GPIO_CLK_ENABLE();                            /* D3引脚IO时钟使能 */
        SD_CLK_GPIO_CLK_ENABLE();                           /* CLK引脚IO时钟使能 */
        SD_CMD_GPIO_CLK_ENABLE();                           /* CMD引脚IO时钟使能 */

        /* SD_D0引脚模式设置 */
        gpio_init_struct.Pin = SD_D0_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 推挽复用 */
        gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
        gpio_init_struct.Alternate = GPIO_AF12_SDIO;        /* 复用为SDIO */
        HAL_GPIO_Init(SD_D0_GPIO_PORT, &gpio_init_struct);  /* 初始化 */
        
        /* SD_D1引脚模式设置 */
        gpio_init_struct.Pin = SD_D1_GPIO_PIN;
        HAL_GPIO_Init(SD_D1_GPIO_PORT, &gpio_init_struct);  /* 初始化 */
        /* SD_D2引脚模式设置 */
        gpio_init_struct.Pin = SD_D2_GPIO_PIN;
        HAL_GPIO_Init(SD_D2_GPIO_PORT, &gpio_init_struct);  /* 初始化 */
        /* SD_D3引脚模式设置 */
        gpio_init_struct.Pin = SD_D3_GPIO_PIN;
        HAL_GPIO_Init(SD_D3_GPIO_PORT, &gpio_init_struct);  /* 初始化 */
        /* SD_CLK引脚模式设置 */
        gpio_init_struct.Pin = SD_CLK_GPIO_PIN;
        HAL_GPIO_Init(SD_CLK_GPIO_PORT, &gpio_init_struct); /* 初始化 */
        /* SD_CMD引脚模式设置 */
        gpio_init_struct.Pin = SD_CMD_GPIO_PIN;
        HAL_GPIO_Init(SD_CMD_GPIO_PORT, &gpio_init_struct); /* 初始化 */
    }
}

/**
 * @brief       获取卡信息函数
 * @param       cardinfo   : SD卡信息句柄
 * @retval      返回值:读取卡信息状态值
 */
uint8_t get_sd_card_info(HAL_SD_CardInfoTypeDef *cardinfo)
{
    uint8_t sta;
    sta = HAL_SD_GetCardInfo(&g_sdcard_handle, cardinfo);
    return sta;
}

/**
 * @brief       判断SD卡是否可以传输(读写)数据
 * @param       无
 * @retval      返回值:SD_TRANSFER_OK      传输完成，可以继续下一次传输
                       SD_TRANSFER_BUSY SD 卡正忙，不可以进行下一次传输
 */
uint8_t get_sd_card_state(void)
{
    return ((HAL_SD_GetCardState(&g_sdcard_handle) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
 * @brief       读SD卡(fatfs/usb调用)
 * @param       pbuf  : 数据缓存区
 * @param       saddr : 扇区地址
 * @param       cnt   : 扇区个数
 * @retval      0, 正常;  其他, 错误代码(详见SD_Error定义);
 */
uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = 100000;      /* max wait-loop count (~1s); SD_TIMEOUT is far too large for this loop */
    long long lsector = saddr;
    sys_intx_disable();                                                                    /* 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!) */
    sta = HAL_SD_ReadBlocks(&g_sdcard_handle, (uint8_t *)pbuf, lsector, cnt, SD_TIMEOUT);  /* 多个sector的读操作 */

    /* 等待SD卡读完 */
    while (get_sd_card_state() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
            break;
        }
    }

    sys_intx_enable();      /* 开启总中断 */
    return sta;
}

/**
 * @brief       写SD卡(fatfs/usb调用)
 * @param       pbuf  : 数据缓存区
 * @param       saddr : 扇区地址
 * @param       cnt   : 扇区个数
 * @retval      0, 正常;  其他, 错误代码(详见SD_Error定义);
 */
uint8_t sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = 100000;      /* max wait-loop count (~1s); SD_TIMEOUT is far too large for this loop */
    long long lsector = saddr;
    sys_intx_disable();                                                                     /* 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!) */
    sta = HAL_SD_WriteBlocks(&g_sdcard_handle, (uint8_t *)pbuf, lsector, cnt, SD_TIMEOUT);  /* 多个sector的写操作 */

    /* 等待SD卡写完 */
    while (get_sd_card_state() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
            break;
        }
    }

    sys_intx_enable();     /* 开启总中断 */
    
    return sta;
}
