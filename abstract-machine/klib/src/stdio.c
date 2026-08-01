#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    while (*fmt != '\0') {
        if (*fmt == '%') {
            fmt++;  // 跳过 '%'
            
            // 解析宽度和填充字符
            char pad = ' ';      // 默认用空格填充
            int width = 0;
            
            if (*fmt == '0') {
                pad = '0';
                fmt++;
            }
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            
            // 此时 *fmt 指向格式字符
            switch (*fmt) {
                case 'd': {
                    int num = va_arg(ap, int);
                    
                    // 处理负数
                    if (num < 0) {
                        putch('-');
                        num = -num;
                        if (width > 0) width--;
                    }
                    
                    // 计算数字位数
                    int tmp = num;
                    int digits = 1;
                    while (tmp >= 10) {
                        digits++;
                        tmp /= 10;
                    }
                    
                    // 补齐前导字符
                    while (digits < width) {
                        putch(pad);
                        width--;
                    }
                    
                    // 打印数字
                    if (num == 0) {
                        putch('0');
                    } else {
                        int divisor = 1;
                        tmp = num;
                        while (tmp >= 10) {
                            divisor *= 10;
                            tmp /= 10;
                        }
                        while (divisor > 0) {
                            putch('0' + (num / divisor));
                            num %= divisor;
                            divisor /= 10;
                        }
                    }
                    break;
                }
                
                case 's': {
                    char *str = va_arg(ap, char *);
                    if (str == NULL) {
                        str = "(null)";
                    }
                    int len = 0;
                    char *tmp = str;
                    while (*tmp != '\0') {
                        len++;
                        tmp++;
                    }
                    // 补齐宽度（字符串用空格填充）
                    while (len < width) {
                        putch(' ');
                        width--;
                    }
                    while (*str != '\0') {
                        putch(*str++);
                    }
                    break;
                }
                
                case 'c': {
                    char ch = (char)va_arg(ap, int);
                    // 补齐宽度
                    while (width > 1) {
                        putch(' ');
                        width--;
                    }
                    putch(ch);
                    break;
                }
                
                case 'x':
                case 'X': {
                    unsigned int num = va_arg(ap, unsigned int);
                    char hex_chars[] = "0123456789abcdef";
                    char hex_upper[] = "0123456789ABCDEF";
                    char *chars = (*fmt == 'X') ? hex_upper : hex_chars;
                    
                    // 计算位数
                    unsigned int tmp = num;
                    int digits = 1;
                    while (tmp >= 16) {
                        digits++;
                        tmp /= 16;
                    }
                    
                    // 补齐前导字符
                    while (digits < width) {
                        putch(pad);
                        width--;
                    }
                    
                    // 打印十六进制
                    if (num == 0) {
                        putch('0');
                    } else {
                        char buf[32];
                        int i = 0;
                        while (num > 0) {
                            buf[i++] = chars[num % 16];
                            num /= 16;
                        }
                        while (i > 0) {
                            putch(buf[--i]);
                        }
                    }
                    break;
                }
                
                case '%': {
                    putch('%');
                    break;
                }
                
                default:
                    // 未知格式字符，直接输出
                    putch('%');
                    putch(*fmt);
                    break;
            }
        } else {
            putch(*fmt);
        }
        fmt++;
    }
    
    va_end(ap);
    return 0;
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
