#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while(*src) {
    *p = *src;
    p++;
    src++;
  }
  *p = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  while(*p) {
    p++;
  }
  while(*src) {
    *p = *src;
    p++;
    src++;
  }
  *p = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  char *p1 = (char *)s1, *p2 = (char *)s2;
  while(*(p1) && *(p2) && (*p1 == *p2)) {
    p1++;
    p2++;
  }
  return (int)(*p1 - *p2);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  char *p = (char *)s;
  while(n > 0) {
    *p = (char)c;
    p++;
    n--;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  while(n> 0){
    *(char *)out = *(char *)in;
    out = (char *)out + 1;
    in = (char *)in + 1;
    n--;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  char *p1 = (char *)s1, *p2 = (char *)s2;
  while(n > 0) {
    if(*p1 != *p2) {
      return (int)(*p1 - *p2);
    }
    p1++;
    p2++;
    n--;
  }
  return 0;
}

#endif
