/* GCC/newlib adapters. This file is deliberately not part of the Keil build. */
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

extern unsigned char __HeapBase;
extern unsigned char __HeapLimit;

/* newlib calls _write, not the Keil fputc hook directly. Reuse its UART output. */
int _write(int fd, char *buffer, int length)
{
    if (fd != 1 && fd != 2) {
        errno = EBADF;
        return -1;
    }
    for (int i = 0; i < length; ++i) {
        fputc((unsigned char)buffer[i], stdout);
    }
    return length;
}

/* Never let libc allocations grow into the main/interrupt stack. Not ISR-safe. */
void *_sbrk(ptrdiff_t increment)
{
    static uintptr_t current;
    const uintptr_t base = (uintptr_t)&__HeapBase;
    const uintptr_t limit = (uintptr_t)&__HeapLimit;
    if (current == 0) {
        current = base;
    }
    if ((increment >= 0 && (uintptr_t)increment > limit - current) ||
        (increment < 0 && (uintptr_t)(-(increment + 1)) + 1 > current - base)) {
        errno = ENOMEM;
        return (void *)-1;
    }
    void *previous = (void *)current;
    current += increment;
    return previous;
}

int _fstat(int fd, struct stat *status)
{
    if (fd < 0 || fd > 2) { errno = EBADF; return -1; }
    status->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    if (fd >= 0 && fd <= 2) { return 1; }
    errno = EBADF;
    return 0;
}

int _close(int fd) { (void)fd; errno = EBADF; return -1; }
int _lseek(int fd, int offset, int whence)
{
    (void)fd; (void)offset; (void)whence;
    errno = ESPIPE;
    return -1;
}
/* Receive remains interrupt-driven through the existing USART driver. */
int _read(int fd, char *buffer, int length)
{
    (void)fd; (void)buffer; (void)length;
    errno = ENOSYS;
    return -1;
}
int _getpid(void) { return 1; }
int _kill(int pid, int signal)
{
    (void)pid; (void)signal;
    errno = EINVAL;
    return -1;
}
void _exit(int status)
{
    (void)status;
    for (;;) { }
}
