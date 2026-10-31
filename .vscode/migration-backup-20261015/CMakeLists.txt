cmake_minimum_required(VERSION 3.16)
project(atk_f429_FreeRTOS C ASM)

# Export compile_commands.json so clangd / C/C++ IntelliSense can use the real flags
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==================================
# 1. Toolchain Configuration
# ==================================
# ==================================
# 2. MCU Configuration
# ==================================
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CPU_DEFINES " -DARM_MATH_CM4 -D__FPU_PRESENT=1U")

# ==================================
# 3. Linker Script
# ==================================
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F429IGTx_FLASH.ld")

# ==================================
# 4. Compiler Flags
# ==================================
set(COMMON_FLAGS "${CPU_FLAGS} ${CPU_DEFINES}")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11 -Wall -Wextra -ffunction-sections -fdata-sections -Os -g -DDEBUG -DSTM32F429xx -DUSE_HAL_DRIVER -Wno-unused-parameter")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "${COMMON_FLAGS} -T${LINKER_SCRIPT} -Wl,--gc-sections -Wl,-Map=${CMAKE_PROJECT_NAME}.map -specs=nano.specs -specs=nosys.specs -u _printf_float -u _scanf_float -Wl,--cref -Wl,--print-memory-usage")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")

# ==================================
# 5. Include Paths
# ==================================
set(INCLUDE_DIRS
    # Drivers root (for Keil-style relative includes like ./BSP/LED/led.h)
    ${CMAKE_SOURCE_DIR}/Drivers
    # CMSIS Core
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    # CMSIS Device (STM32F4xx)
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    # HAL Driver
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    # BSP (Board Support Package)
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/LED
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/LCD
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/KEY
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/SDRAM
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/TIMER
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/ADC
    # SYSTEM
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/sys
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/usart
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/delay
    # MALLOC
    ${CMAKE_SOURCE_DIR}/Middlewares/MALLOC
    # USMART
    ${CMAKE_SOURCE_DIR}/Middlewares/USMART
    # Middlewares root (for Keil-style relative includes)
    ${CMAKE_SOURCE_DIR}/Middlewares
    # FreeRTOS
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/include
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F
    # User
    ${CMAKE_SOURCE_DIR}/User
)

include_directories(${INCLUDE_DIRS})

# ==================================
# 6. Source Files
# ==================================

# 6.1 Startup (GCC version)
set(STARTUP_SRC
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f429xx.s
)

# 6.2 CMSIS System
set(CMSIS_SRC
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c
)

# 6.3 HAL Source Files (only those actually used by the project)
set(HAL_SRC
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c
)

# 6.4 BSP Source Files
set(BSP_SRC
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/LED/led.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/LCD/lcd.c
    # lcd_ex.c is included by lcd.c via #include, do NOT compile separately
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/LCD/ltdc.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/KEY/key.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/SDRAM/sdram.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/TIMER/btim.c
    ${CMAKE_SOURCE_DIR}/Drivers/BSP/ADC/adc.c
)

# 6.5 SYSTEM Source Files
set(SYSTEM_SRC
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/sys/sys.c
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/usart/usart.c
    ${CMAKE_SOURCE_DIR}/Drivers/SYSTEM/delay/delay.c
)

# 6.6 Middleware - MALLOC
set(MALLOC_SRC
    ${CMAKE_SOURCE_DIR}/Middlewares/MALLOC/malloc.c
)

# 6.7 Middleware - USMART
set(USMART_SRC
    ${CMAKE_SOURCE_DIR}/Middlewares/USMART/usmart.c
    ${CMAKE_SOURCE_DIR}/Middlewares/USMART/usmart_config.c
    ${CMAKE_SOURCE_DIR}/Middlewares/USMART/usmart_port.c
    ${CMAKE_SOURCE_DIR}/Middlewares/USMART/usmart_str.c
)

# 6.8 Middleware - FreeRTOS
set(FREERTOS_SRC
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/tasks.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/queue.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/list.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/croutine.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/event_groups.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/timers.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/stream_buffer.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/portable/GCC/ARM_CM4F/port.c
    ${CMAKE_SOURCE_DIR}/Middlewares/FreeRTOS/portable/MemMang/heap_4.c
)

# 6.9 User Source Files
set(USER_SRC
    ${CMAKE_SOURCE_DIR}/User/main.c
    ${CMAKE_SOURCE_DIR}/User/stm32f4xx_it.c
    ${CMAKE_SOURCE_DIR}/User/freertos_demo.c
)

# ==================================
# 7. Aggregate All Sources
# ==================================
set(ALL_SOURCES
    ${STARTUP_SRC}
    ${CMSIS_SRC}
    ${HAL_SRC}
    ${BSP_SRC}
    ${SYSTEM_SRC}
    ${MALLOC_SRC}
    ${USMART_SRC}
    ${FREERTOS_SRC}
    ${USER_SRC}
)

# ==================================
# 8. Executable Target
# ==================================
add_executable(${CMAKE_PROJECT_NAME} ${ALL_SOURCES})

# 9. Set linker language to C (ASM startup uses assembler-with-cpp)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE C
    SUFFIX ".elf"
)

# 10. Post-build: generate .hex and .bin files
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    COMMENT "Generating .hex, .bin and size report..."
)

# ==================================
# 11. Print Build Info
# ==================================
message(STATUS "======================================")
message(STATUS "Project: ${CMAKE_PROJECT_NAME}")
message(STATUS "MCU: STM32F429IGTx (Apollo Board)")
message(STATUS "Core: Cortex-M4 with FPU (hard float)")
message(STATUS "Linker Script: ${LINKER_SCRIPT}")
message(STATUS "FreeRTOS: heap_4, ARM_CM4F port (GCC)")
message(STATUS "======================================")