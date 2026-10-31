# ARM Cortex-M4 Cross-Compilation Toolchain File
# Usage: cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm-none-eabi.cmake ...

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ARM GCC Toolchain
set(TOOLCHAIN_PREFIX "arm-none-eabi")

set(CMAKE_C_COMPILER    "${TOOLCHAIN_PREFIX}-gcc"     CACHE STRING "C compiler")
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_PREFIX}-g++"     CACHE STRING "C++ compiler")
set(CMAKE_ASM_COMPILER  "${TOOLCHAIN_PREFIX}-gcc"     CACHE STRING "ASM compiler")
set(CMAKE_OBJCOPY       "${TOOLCHAIN_PREFIX}-objcopy" CACHE STRING "objcopy")
set(CMAKE_OBJDUMP       "${TOOLCHAIN_PREFIX}-objdump" CACHE STRING "objdump")
set(CMAKE_SIZE          "${TOOLCHAIN_PREFIX}-size"    CACHE STRING "size")

# Disable compiler checks (they can't run ARM binaries on host)
set(CMAKE_C_COMPILER_WORKS TRUE CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE INTERNAL "")
set(CMAKE_ASM_COMPILER_WORKS TRUE CACHE INTERNAL "")

# Disable try_run (can't run ARM binaries on host)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)