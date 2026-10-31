/**
 ****************************************************************************************************
 * @file        freertos.c
 * @author      77
 * @version     V2.0
 * @date        2026.10.27
 * @brief       FreeRTOS 移植
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention

 ****************************************************************************************************
 */
#include "stdint.h"
#include "freertos_demo.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
/*FreeRTOS*********************************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "./MALLOC/malloc.h"
#include <string.h>
#include "./BSP/ADC/adc.h"
#include "./BSP/SDIO/sdio_sdcard.h"
#include "./FATFS/source/ff.h"
/******************************************************************************************************/
/*FreeRTOS配置*/

/* TASK1 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
void main_task( void * pvParameters );
#define MAIN_TASK_STACK_SIZE 1024
#define MAIN_TASK_PRIORITY 4
TaskHandle_t main_task_handler;

#define ALARM_TRIG_MV 2200
#define ALARM_REL_MV  2000

static uint16_t ADC_value;
static float Voltage = 0;
static uint16_t Voltage_mv=0;

static FATFS fs;             /* 文件系统对象 */
static FIL fil;              /* 文件对象 */
static UINT bw;              /* 文件写入长度 */
static FRESULT res;          /* 文件操作结果 */
#define DATA_FILE "0:/single_task_data_over.csv"

static uint8_t file_ok = 0;     /* 文件操作是否成功标志位,0:失败 1:成功 */
static char str_buf[64] = {0};  /* 字符串缓冲区 */
static uint16_t seq = 0;
static uint8_t line_cnt = 0;    /* 当前写入的行数 */

static uint8_t alarm = 0;
static uint8_t alarm_prev = 0;


/******************************************************************************************************/

/**
 * @brief       FreeRTOS例程入口函数
 * @param       无
 * @retval      无
 */
void freertos_demo(void)
{    
	 BaseType_t ret = xTaskCreate((TaskFunction_t) 			main_task,            //任务函数
					 		      (char *)                  "main_task",          //任务名称
					 			  (configSTACK_DEPTH_TYPE)  MAIN_TASK_STACK_SIZE, //堆栈大小
					 			  (void *)                  NULL,                 //参数传递
					 			  (UBaseType_t)             MAIN_TASK_PRIORITY,   //优先级
					 			  (TaskHandle_t *)          &main_task_handler ); //任务句柄
    if(ret != pdPASS)
    {
        printf("[ERROR] 创建 main_task 任务失败\r\n");
		while(1);
	}
	vTaskStartScheduler();
	while(1); 
}

void main_task(void * pvParameters)
 {
	 if(sd_init() != 0)
    {
        printf("[1/3] SD卡初始化失败\r\n");
    }
    else if((res = (f_mount(&fs,"0:",1))) != FR_OK)
    {
        printf("[2/3] SD卡挂载失败 f_mount res = %d\r\n",res);
    }
    else if((res = (f_open(&fil,DATA_FILE,FA_CREATE_ALWAYS | FA_WRITE))) != FR_OK)
    {
        printf("[3/3] SD卡文件创建失败 f_open res = %d\r\n",res);
    }else
    {
        file_ok = 1;
        printf("SD卡万事俱备,容量: %lu KB\r\n",(unsigned long)SD_TOTAL_SIZE_KB(&g_sdcard_handle));

        res = f_write(&fil,"seq,ms,adc,mv,alarm\r\n",21,&bw);
        if(res == FR_OK && bw == 21)
        {
            printf("[OK]写入表头数据成功\r\n");
        }
        else
        {
            file_ok = 0;
            printf("[ERROR]写入表头数据失败, res = %d bw = %u\r\n",res,bw);
        }
    }
    
     TickType_t LastWakeTime = xTaskGetTickCount();     
	 while(1)
    {
        uint16_t Color;
        int len = 0;
        uint8_t key = 0;

        /*按键扫描 暂停与恢复*/
        key = key_scan(0);
        if(key == KEY0_PRES && file_ok == 1)
        {
            res = f_close(&fil);
            if(res == FR_OK)
            {
                printf("[OK] 文件关闭成功 可以拔卡\r\n");
            }
            else
            {
                printf("[ERROR] 文件关闭失败 数据可能不完整， res = %d\r\n",res);
            }
            file_ok = 0;
        }else if(key == KEY1_PRES && file_ok == 0)
        {
            res = f_open(&fil,DATA_FILE,FA_OPEN_APPEND | FA_WRITE);
            if(res == FR_OK)
            {
                file_ok = 1;
                line_cnt = 0;
                printf("[OK] 文件打开成功 恢复记录,文件从%u开始继续记录\r\n", seq);
            }
            else
            {
                printf("[ERROR] 文件打开失败 恢复失败, res = %d\r\n",res);
            }
        }

        /*数据处理*/
        ADC_value = ADC_Get();
        Voltage = ADC_value * 3.3 /4096;
        Voltage_mv = Voltage * 1000;

        /*阈值判断*/
        if(alarm == 0 && Voltage_mv > ALARM_TRIG_MV)
        {
            alarm = 1;
        }else if(alarm == 1 && Voltage_mv < ALARM_REL_MV)
        {
            alarm = 0;
        }
        if(alarm != alarm_prev)
        {
            alarm_prev = alarm;
            if(alarm)
            {
                printf("[进入报警] adc:%d Voltage_mv:%u\r\n",ADC_value,Voltage_mv);
            }else
            {
                printf("[解除报警] adc:%d Voltage_mv:%u\r\n",ADC_value,Voltage_mv);
            }
        }
		
		/*文件状态显示*/
		Color = alarm ? RED : BLACK;
        lcd_show_num(140,140,ADC_value,4,24,Color);
        lcd_show_num(152,180,Voltage_mv,4,24,Color);
        lcd_show_num(68,220,seq,6,24,BLACK);
        lcd_show_num(152,260,alarm,1,24,Color);
		LED0(!alarm);
        LED1(!file_ok);
        if(file_ok)
        {
            lcd_show_string(20,300,400,32,24,"Recording...",BLACK);
        }
        else
        {
            lcd_show_string(20,300,400,32,24,"Stopped...  ",RED);
        }

        /*数据写入*/
        if(file_ok == 1)
        {
            len = snprintf(str_buf, sizeof(str_buf), "%u,%lu,%u,%u,%u\r\n",
                           seq,HAL_GetTick(),ADC_value,Voltage_mv,alarm);
            if(len > 0 && len < (int)sizeof(str_buf))
            {
                res =f_write(&fil,str_buf,len,&bw);
                if(res == FR_OK && bw == (UINT)len)
                {
                    seq ++;
                    line_cnt ++;
                    if(line_cnt >= 10)
                    {
                        line_cnt = 0;
                        res = f_sync(&fil);
                        if(res != FR_OK)
                        {
                            printf("[ERROR] 同步数据失败, res = %d\r\n",res);
                        }
                        printf("已写入 %u 行数据,stack = %lu\r\n",
							   seq,uxTaskGetStackHighWaterMark(NULL));
                    }
                }
                else
                {
                    printf("[ERROR] 写入数据失败, res = %d bw = %u\r\n",res,bw);
                }
            }else
            {
                printf("[ERROR] 字符串输入失败\r\n");
            }
        }
        vTaskDelayUntil(&LastWakeTime, pdMS_TO_TICKS(500));
    }
 }
 
 
 
/**
 * @brief       任务栈溢出钩子函数
 * @note        configCHECK_FOR_STACK_OVERFLOW = 1/2 时, 内核在任务切换时检查到栈溢出后调用本函数
 *              这里做"保命"处理: 关中断 -> 串口打印出错的任务名 -> 死循环, 方便定位问题
 * @param       xTask: 发生栈溢出的任务句柄(未使用)
 * @param       pcTaskName: 发生栈溢出的任务名
 * @retval      无
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;                            /* 避免"未使用参数"编译告警 */
	taskDISABLE_INTERRUPTS();               
	printf("[FATAL] 任务栈溢出! 任务名: %s\r\n", pcTaskName);
	while (1)          
	{
    }
}

/**
 * @brief       动态内存申请失败钩子函数
 * @note        configUSE_MALLOC_FAILED_HOOK = 1 时, pvPortMalloc() 申请失败(返回NULL)前调用本函数
 *              常见原因: configTOTAL_HEAP_SIZE 太小, 或任务/队列/信号量创建后没有删除
 * @param       无
 * @retval      无
 */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();               

    printf("[FATAL] FreeRTOS 堆内存分配失败!\r\n");
    printf("[FATAL] configTOTAL_HEAP_SIZE = %u 字节, 当前剩余 = %u 字节\r\n",
           (unsigned int)configTOTAL_HEAP_SIZE, (unsigned int)xPortGetFreeHeapSize());
	while (1)
    {
    }
}

 
