/**
 ****************************************************************************************************
 * @file        diskio.c
 * @brief       FatFs 底层(diskio) 驱动代码 —— 本工程只支持 SD 卡
 ****************************************************************************************************
 * 说明：
 *   1. 正点原子例程里的 diskio.c 同时支持 SD卡 / SPI Flash / NAND FLASH，
 *      本工程只用到 SD 卡，所以把其他分支删掉，只保留 SD 卡部分，便于阅读和排错。
 *   2. 下面这 4 个函数是 FatFs(ff.c) 唯一会调用的底层接口：
 *        disk_initialize()  初始化驱动器    ——> 对应 sd_init()
 *        disk_read()        读扇区          ——> 对应 sd_read_disk()
 *        disk_write()       写扇区          ——> 对应 sd_write_disk()
 *        disk_ioctl()       告诉 FatFs 参数 ——> 扇区大小、扇区总数等
 *   3. 1 个扇区 = 512 字节，与 SD 卡的物理块大小一致
 *      （ffconf.h 里 FF_MIN_SS = FF_MAX_SS = 512）。
 ****************************************************************************************************
 */

#include "./BSP/SDIO/sdio_sdcard.h"
#include "./FATFS/source/diskio.h"


#define SD_CARD     0       /* SD卡, 定义为 0 */


/**
 * @brief       获取驱动器状态
 * @param       pdrv : 驱动器编号
 * @retval      0:正常; STA_NOINIT:未初始化
 */
DSTATUS disk_status (BYTE pdrv)
{
    if (pdrv != SD_CARD)
    {
        return STA_NOINIT;      /* 本工程只实现了 0 号驱动器 */
    }

    return 0;
}

/**
 * @brief       初始化驱动器
 * @param       pdrv : 驱动器编号
 * @retval      0:成功; STA_NOINIT:失败
 */
DSTATUS disk_initialize (BYTE pdrv)
{
    if (pdrv != SD_CARD)
    {
        return STA_NOINIT;
    }

    if (sd_init() != 0)         /* sd_init() 返回 0 表示成功 */
    {
        return STA_NOINIT;
    }

    return 0;
}

/**
 * @brief       读扇区
 * @param       pdrv   : 驱动器编号
 * @param       buff   : 数据缓冲区
 * @param       sector : 起始扇区号
 * @param       count  : 要读取的扇区数
 * @retval      RES_OK:成功; RES_ERROR:失败; RES_PARERR:参数错误
 */
DRESULT disk_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != SD_CARD)
    {
        return RES_PARERR;
    }

    if (count == 0)
    {
        return RES_PARERR;      /* count 不能为 0 */
    }

    if (sd_read_disk(buff, sector, count) == 0)
    {
        return RES_OK;
    }

    return RES_ERROR;
}

/**
 * @brief       写扇区
 * @param       pdrv   : 驱动器编号
 * @param       buff   : 数据缓冲区
 * @param       sector : 起始扇区号
 * @param       count  : 要写入的扇区数
 * @retval      RES_OK:成功; RES_ERROR:失败; RES_PARERR:参数错误
 */
DRESULT disk_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != SD_CARD)
    {
        return RES_PARERR;
    }

    if (count == 0)
    {
        return RES_PARERR;
    }

    if (sd_write_disk((uint8_t *)buff, sector, count) == 0)
    {
        return RES_OK;
    }

    return RES_ERROR;
}

/**
 * @brief       其他控制命令
 * @param       pdrv : 驱动器编号
 * @param       cmd  : 控制命令
 * @param       buff : 数据缓冲区
 * @retval      RES_OK:成功; 其他:失败
 */
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void *buff)
{
    DRESULT res;

    if (pdrv != SD_CARD)
    {
        return RES_ERROR;
    }

    switch (cmd)
    {
        case CTRL_SYNC:                                     /* 等待写入完成 */
            res = RES_OK;
            break;

        case GET_SECTOR_SIZE:                               /* 扇区大小(字节) */
            *(WORD *)buff = 512;
            res = RES_OK;
            break;

        case GET_BLOCK_SIZE:                                /* 擦除块大小(扇区数), 只有 f_mkfs 会用到 */
            *(WORD *)buff = g_sd_card_info_handle.LogBlockSize;
            res = RES_OK;
            break;

        case GET_SECTOR_COUNT:                              /* 总扇区数 */
            *(DWORD *)buff = g_sd_card_info_handle.LogBlockNbr;
            res = RES_OK;
            break;

        default:
            res = RES_PARERR;
            break;
    }

    return res;
}
