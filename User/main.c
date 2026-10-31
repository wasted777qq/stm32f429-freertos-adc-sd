/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-4-20
 * @brief       内存管理 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 F429开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SDRAM/sdram.h"
#include "./MALLOC/malloc.h"
#include "freertos_demo.h"
#include "./BSP/ADC/adc.h"

int main(void)
{
    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(360, 25, 2, 8);        /* 设置时钟,180Mhz */
    delay_init(180);                            /* 延时初始化 */
    usart_init(115200);                         /* 串口初始化为115200 */
    led_init();                                 /* 初始化LED */
    key_init();                                 /* 初始化按键 */
    sdram_init();                               /* SRAM初始化 */
    lcd_init();                                 /* 初始化LCD */

    lcd_clear(WHITE);                            /* 清屏为白色 */
    lcd_show_string(20, 20, 400, 32, 24, "Free RTOS",BLACK);
    lcd_show_string(20, 50, 400, 32, 24, "STM32F429IGT6",BLUE);

    my_mem_init(SRAMIN);                        /* 初始化内部内存池 */
    my_mem_init(SRAMEX);                        /* 初始化外部内存池 */
    my_mem_init(SRAMCCM);                       /* 初始化CCM内存池 */
    
    ADC_GPIO_init();
    ADC_Init();
    
    lcd_show_string(100,110,400,32,30,"Data Logger",BLACK);
    lcd_show_string(20,140,400,32,24,"ADC_Value:",BLACK);
    lcd_show_string(20,180,400,32,24,"Voltage_mv:",BLACK);
    lcd_show_string(200,180,400,32,24,"mV",BLACK);
    lcd_show_string(20,220,400,32,24,"seq:",BLACK);
    lcd_show_string(20,260,400,32,24,"alarm_flag:",BLACK);
    
    freertos_demo();
}
