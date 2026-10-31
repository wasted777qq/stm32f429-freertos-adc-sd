# GCC + CMake 使用说明

## 当前方案

- 主编译：CMake + Ninja + ARM GCC，Debug 为 `-O0 -g3`。
- 工具通过 PATH 查找；本机已安装 STM32CubeCLT 1.18.0。
- Keil 工程和原有编译任务保留，不与 CMake 自动同步。
- 修改前配置备份：`.vscode/migration-backup-20261015/`。
- `User/gcc/syscalls.c` 仅参加 GCC 构建，不要加入 Keil 工程。

## 日常操作

1. 首次在 VS Code 打开后执行 `Developer: Reload Window`，让 C/C++ 重载配置。
2. `Ctrl+Shift+B`：默认 GCC Debug 编译，不烧录。
3. 命令面板 → `Tasks: Run Task` → `GCC: 编译并下载 (ST-Link)`：先 GCC 编译，成功后 ST-Link 写入、校验并复位。
4. `GCC: 清理后重新编译`：清理旧构建产物，再全量编译，不下载。普通修改只需要 `GCC: 编译 (Debug)`。
5. `STM32: 串口监视 (GBK / 115200)`：GCC、Keil 均可使用；输入实际 COM 口，115200、8N1。用任务终止按钮关闭后再用其他串口软件。
6. Keil 备用保留“编译”“重新编译”“编译并下载”三个入口，明确使用 Keil 工具链和产物。
7. 菜单共 7 个可见任务；CMake Configure 已隐藏，由 GCC 编译任务自动执行。重复下载、单独清理、收集更多错误和下载检查不再单独占用菜单。

需要收集更多错误或只检查下载路径时，可在终端运行：

```powershell
cmake --preset debug
cmake --build --preset debug -- -k 0
powershell -NoProfile -ExecutionPolicy Bypass -File .vscode/flash_stlink.ps1 -Check
```

`ninja: no work to do.` 表示无需重编译；`warning:` 是警告；`error:`、`FAILED:`、非零退出代码表示失败。失败时不要使用旧 HEX 判断修改效果。`Ctrl+Shift+M` 可打开问题面板，以实际编译终端为最终依据。

终端编译：

```powershell
cmake --preset debug
cmake --build --preset debug
```

GCC 产物：`build/atk_f429_FreeRTOS.elf`、`.hex`、`.bin`、`.map`。
Keil 产物：`Output/atk_f429.hex`。两者不能混淆。

## 添加和移除文件

- 现有源码内容变化：保存并重新编译即可。
- 新增 `.c`：在资源管理器创建后，加入 `CMakeLists.txt` 对应的 `USER_SRC` / `BSP_SRC` 等列表。
- 新增 `.h`：通常无需加入源码列表；如果要直接按文件名 include，在 `INCLUDE_DIRS` 加入其目录。
- 不参加编译：从 CMake 源码列表移除，不需要删除磁盘文件。
- 宏定义：当前 `CMAKE_C_FLAGS` 内的 `-D...`；Debug 专用定义位于 `CMAKE_C_FLAGS_DEBUG`。
- 优化：当前 Debug `-O0`，Release 参数 `-Os`。当前只提供 Debug 预设，不要在本地混用不同输出目录的 HEX。

## 实时错误提示

- 保留红波浪线，C/C++ 直接读取 `build/compile_commands.json` 的真实参数。
- 移除递归搜索整个工作区的头文件路径，避免误搜 FreeRTOS 的其他编译器移植层。
- 使用工程独立的新索引和预编译头缓存路径，不删除其他工程缓存。
- 截图中 `main.c` 两处 `lcd_show_num` 都有 6 个参数；磁盘上的声明和实现也是 6 个，GCC 实编译通过。不能因此删除第六个颜色参数。
- 如重载后仍提示参数太多：保存全部文件，执行 `C/C++: Reset IntelliSense Database`；对函数按 F12，核实跳转到本工程 `Drivers/BSP/LCD/lcd.h` 的 6 参数声明。然后提供 `C/C++: Log Diagnostics` 输出，以便继续定位。

## 运行与内存注意事项

- GCC `printf` 已通过 `_write` 接到现有 UART `fputc`。尚未上板验证，输出前必须初始化 USART。
- 保留浮点 printf/scanf 支持，不擅自删减现有功能。C 库堆暂设 4KB，主/中断栈最低保留 4KB，并为 `_sbrk` 增加边界保护；不是运行时绝不溢出的保证。
- FreeRTOS `heap_4`、各任务栈，与上述 C 库堆/主栈不是同一块内存。多任务使用 libc/malloc/printf 的互斥和栈余量仍须按实际需求验证。
- 每个 C 文件生成 `.su` 栈使用报告（在 build 下对应对象目录）；它不是完整调用链或运行时栈高水位。
- 现有分配器预留 160KB 内部池、60KB CCM 池和大块 SDRAM 池，因此静态占用较高，不等于内存都已被业务动态用完。
- 当前链接结果中帧缓冲位于 `0xC0000000`，外部内存池位于 `0xC01F4000`；这是当前链接顺序的结果，改变显示格式/源码顺序后应重新核查 MAP。
- 当前 `main.c` 的 ADC 显示无限循环位于 `freertos_demo()` 之前，因此不会启动 FreeRTOS 演示。这是现有业务流程，本次未修改。
- 原驱动/USMART 中的字符下标、格式字符串和初始化警告，以及 RWX 段警告尚未统一整改，不应把所有警告都当作假错。

## 需要上板验证

- 正确接好 ST-Link、供电和 USB 串口，确认 SWD 引脚及实际 COM 号。
- 下载 GCC 产物，验证 LCD、ADC、电压显示、按键、串口整数/中文/浮点输出。
- 如需 FreeRTOS 演示，先明确是否改变 main 中的无限循环，再验证任务和中断。
- 本次没有写入或擦除芯片，没有启动在线调试；`launch.json` 尚未配置。
- 启用原先不可达的 FreeRTOS 代码后会增加被链接保留的内核/任务内存，应重新构建并检查 RAM；当前占用不能直接代表届时占用。

## 本次验证结果

- 完成一次 `--clean-first` 全量 GCC Debug 编译：退出码 0，生成 ELF/HEX/BIN/MAP。
- Flash 90,608 字节；内部 RAM 的链接分配（包含本次堆/栈预留）185,304 字节，占 94.25%。CCM 99.61%、SDRAM 99.85%，主要为预分配池。
- 生成 55 份 C 文件 `.su` 栈使用报告。
- JSON、PowerShell 语法以及任务依赖检查通过；下载工具路径和 HEX 检查通过，但未测试真实 SWD 通信。
- 红波浪线配置已调整，不能替代 VS Code 重载后的可视确认。
- Keil 备用下载目前没有现成的 `Output/atk_f429.hex`；需先运行 Keil 编译。本次未运行 Keil，未验证备用编译能否成功。

## 网络重连

- 本次不改全局代理、ccSWITCH 配置或 API 密钥。
- 代理开关与端口可达性只能说明连接条件，不能单独证明 Codex 断线根因。
- 官方文档在本次环境返回 403，未能据此核验专用重试/超时配置；不随意添加此类设置。
- 本次只读连通性检查：系统代理已启用 `127.0.0.1:7890`，其 TCP 端口可达；中转站直连 TCP/HTTPS 超时，经该代理的 HTTPS 首页返回 200。结果支持“当前直连路径不可用”，但首页连通不等于已验证带鉴权的流式模型请求，也不是所有重连问题的完整根因。
- 检查中未发送 API Key，未验证模型请求，也未改全局网络设置；工作目录内的构建配置不影响此网络问题。
