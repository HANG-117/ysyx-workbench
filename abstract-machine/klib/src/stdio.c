#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *start = out;

    while (*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    int num = va_arg(ap, int);
                    if (num < 0) {
                        *out++ = '-';
                        num = -num;
                    }
                    if (num == 0) {
                        *out++ = '0';
                        break;
                    }
                    int divisor = 1;
                    int tmp = num;
                    while (tmp >= 10) {
                        divisor *= 10;
                        tmp /= 10;
                    }
                    while (divisor > 0) {
                        *out++ = '0' + (num / divisor);
                        num %= divisor;
                        divisor /= 10;
                    }
                    break;
                }
                case 's': {
                    char *str = va_arg(ap, char *);
                    if (str == NULL) {
                        str = "(null)";
                    }
                    while (*str != '\0') {
                        *out++ = *str++;
                    }
                    break;
                }
                case '%': {
                    *out++ = '%';
                    break;
                }
                default:
                    assert(0);
            }
        } else {
            *out++ = *fmt;
        }
        fmt++;
    }

    *out = '\0';
    va_end(ap);
    return out - start;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
