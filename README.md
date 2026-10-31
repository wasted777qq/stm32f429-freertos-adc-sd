# STM32F429 FreeRTOS 数据记录仪（ADC → SD 卡 CSV）

> 基于 STM32F429IGT6 + FreeRTOS 的嵌入式数据记录仪：每 500 ms 采集一次 ADC 电压，通过 FatFs 追加写入 SD 卡上的 CSV 文件，同时在 LCD 上实时显示；每写满 10 行调用一次 `f_sync` 落盘，保证**掉电最多只丢 5 秒数据**。

An STM32F429/FatFs data logger that samples ADC every 500 ms, streams rows into a CSV file on SD card with periodic `f_sync` for power-loss safety, and shows live values on an LCD.

---

## 功能特性

- **周期采样**：使用 `vTaskDelayUntil` 固定 500 ms 周期，避免累积漂移
- **模拟量转换**：12 位 ADC（0–4095）→ 电压值，换算公式 `V = adc * 3.3 / 4096`
- **阈值报警**：超过 2200 mV 进入报警，低于 2000 mV 解除，带 200 mV 回差（迟滞），防止在阈值附近抖动
- **CSV 记录**：FatFs 写入 `0:/single_task_data_over.csv`，含表头；每条记录包含序号、系统毫秒、ADC 原始值、电压 mV、报警状态
- **掉电保护**：每 10 行执行一次 `f_sync`，把缓冲区数据刷进 SD 卡
- **暂停 / 恢复**：KEY0 关闭文件（`f_close`，可安全拔卡），KEY1 以追加方式重新打开（`FA_OPEN_APPEND`），序号继续累加
- **实时显示**：LCD 显示 ADC 值、电压、记录序号、报警状态、Recording / Stopped
- **运行指示**：LED0 报警时翻转，LED1 指示记录是否停止
- **调试信息**：USART1 115200 输出启动流程、写入进度、任务栈高水位
- **健壮性钩子**：`vApplicationStackOverflowHook` / `vApplicationMallocFailedHook` 在栈溢出、堆耗尽时关中断并打印定位信息，避免跑飞后无从排查

## 硬件平台

| 项目 | 说明 |
|---|---|
| MCU | STM32F429IGT6（Cortex-M4F，180 MHz，1 MB Flash / 192 KB RAM + 外扩 SDRAM） |
| 开发板 | 正点原子 阿波罗 F429 开发板 |
| 存储 | MicroSD 卡，SDIO 4 位接口，FAT32 文件系统 |
| 显示 | 板载 RGB LCD（LTDC + SDRAM 显存） |
| 输入 | KEY0 / KEY1 按键 |
| 内核 | FreeRTOS 10.x，抢占式调度，`configTICK_RATE_HZ = 1000`，堆 `configTOTAL_HEAP_SIZE = 24 KB` |
| 工具链 | arm-none-eabi-gcc（CMake）或 Keil MDK |

## 数据格式

文件：`0:/single_task_data_over.csv`（每次上电以 `FA_CREATE_ALWAYS` 重建）

```csv
seq,ms,adc,mv,alarm
0,12345,2048,1650,0
1,12845,3072,2475,1
```

| 字段 | 含义 |
|---|---|
| `seq` | 记录序号（从 0 递增，暂停/恢复后继续累加） |
| `ms` | `HAL_GetTick()`，上电后的毫秒数 |
| `adc` | ADC 原始采样值（0–4095） |
| `mv` | 换算后的电压，单位 mV |
| `alarm` | 报警状态：0 正常，1 报警 |

## 系统结构

```
        ┌──────────────────────── main_task（优先级 4，栈 1024 words）────────────────────────┐
        │                                                                                      │
  ADC ──┼──▶ ADC_Get() ──▶ 换算电压 ──▶ 阈值判断（2200/2000 mV 回差）──▶ snprintf 组行          │
        │                                              │                        │              │
        │                                              ▼                        ▼              │
        │                                        LCD 刷新 / LED            f_write(CSV)        │
        │                                                                       │              │
        │                                                   每 10 行 ──▶ f_sync 落盘            │
        └──────────────────────────── vTaskDelayUntil(500 ms) ◀────────────────────────────────┘
```

单任务轮询结构，任务内顺序完成「按键 → 采样 → 报警 → 显示 → 写文件」，没有跨任务共享数据，因此不需要互斥量或队列。

## 关键实现说明

**1. 为什么用 `vTaskDelayUntil` 而不是 `vTaskDelay`**
`vTaskDelay` 的周期 = 延时 + 本次循环耗时，采样周期会随文件写入变慢而漂移；`vTaskDelayUntil` 以固定节拍为基准，保证长期平均周期严格 500 ms。

**2. 掉电保护与 `f_sync` 的取舍**
FatFs 的 `f_write` 只写进文件对象缓冲区，真正落盘取决于簇刷新时机。这里每写满 10 行调用 `f_sync`：500 ms × 10 = **最坏情况下丢 5 秒数据**。同步太频繁会显著增加 SD 卡写放大与擦写次数，10 行是在「数据完整性」和「卡寿命/吞吐」之间的折中。

**3. 暂停时可安全拔卡**
直接拔卡可能丢失 FAT 元数据。KEY0 触发 `f_close`，把目录项和数据一并写回，之后才提示“可以拔卡”；恢复时用 `FA_OPEN_APPEND` 续写而不是重建文件。

**4. 报警回差**
只用单一阈值时，电压在阈值附近抖动会导致报警状态频繁翻转、日志被刷屏。改用 2200 mV 触发 / 2000 mV 解除，形成 200 mV 迟滞。

**5. 资源与可观测性**
FreeRTOS 堆 24 KB，只创建一个任务；循环里每 10 行打印 `uxTaskGetStackHighWaterMark`，用于确认栈余量。栈溢出与堆分配失败钩子会关闭中断并打印任务名/剩余堆，把“随机死机”变成可定位的错误。

## 目录结构

```
.
├── User/                     # 应用层：main.c 初始化、freertos_demo.c 任务与业务逻辑
├── Drivers/
│   ├── BSP/                  # ADC、SDIO、LCD、LED、KEY、SDRAM、TIMER 等板级驱动
│   ├── SYSTEM/               # sys、delay、usart 基础支持
│   ├── CMSIS/                # Cortex-M4 内核与 STM32F429 头文件
│   └── STM32F4xx_HAL_Driver/ # ST HAL 库
├── Middlewares/
│   ├── FreeRTOS/             # 内核 + Cortex-M4F 移植层 + heap_4
│   ├── FATFS/                # FatFs R0.14 + diskio 移植
│   ├── MALLOC/               # 正点原子内存池管理
│   └── USMART/               # 串口调试组件
├── Projects/MDK-ARM/         # Keil 工程
├── CMakeLists.txt            # GCC / CMake 构建脚本
├── CMakePresets.json
└── toolchain-arm-none-eabi.cmake
```

## 编译

**方式一：CMake + arm-none-eabi-gcc**

```bash
cmake --preset Debug
cmake --build build
# 产物：build/atk_f429_FreeRTOS.elf / .hex / .bin
```

**方式二：Keil MDK**

打开 `Projects/MDK-ARM/atk_f429.uvprojx`，直接 Build。

## 烧录与运行

1. ST-Link 连接开发板 SWD；
2. 烧写 `build/atk_f429_FreeRTOS.hex`（STM32CubeProgrammer / ST-Link Utility / Keil 下载均可）；
3. 串口 USART1，**115200 8N1**，可看到初始化流程与写入日志：

```
SD卡万事俱备,容量: 15360000 KB
[OK]写入表头数据成功
已写入 10 行数据,stack = 812
已写入 20 行数据,stack = 812
[进入报警] adc:3072 Voltage_mv:2475
```

4. 插卡上电后 LCD 显示 `Recording...`，KEY0 停止记录，KEY1 恢复记录。

## 已知限制与后续计划

- 采样为单任务轮询，未使用 ADC + DMA 连续转换，采样点之间存在软件调度抖动
- 文件名固定、每次上电重建，未按时间分文件，也未做空间不足时的循环覆盖
- 时间戳使用 `HAL_GetTick()`，掉电后从 0 开始，未接入 RTC
- 后续计划：引入队列 + 独立写卡任务解耦采集与存储、按日期分文件、加入 RTC 时间戳、支持 U 盘导出

## 许可

本项目基于正点原子（ALIENTEK）STM32F429 开发板例程二次开发，仅用于学习与求职展示；除正点原子原始代码外的应用层逻辑版权归作者所有，未经许可请勿直接用于课程作业或竞赛提交。
