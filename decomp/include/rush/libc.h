#ifndef RUSH_LIBC_H
#define RUSH_LIBC_H

/* MSL C library functions the game calls. MSL was built without debug info, so these are not in
   rush_protos.h; declarations follow the C standard. (bsearch and modf are game files.) */

typedef unsigned long size_t;

int sprintf(char *s, const char *format, ...);
int printf(const char *format, ...);
int vsprintf(char *s, const char *format, char *arg);

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
size_t strlen(const char *s);
char *strchr(const char *s, int c);
char *strstr(const char *s, const char *sub);

int rand(void);
void srand(unsigned int seed);
int abs(int x);
int atoi(const char *s);
long strtol(const char *s, char **end, int base);

/* CodeWarrior's PowerPC varargs (MSL stdarg.h). __va_arg's second argument is the argument class:
   1 for integers and pointers. */
typedef __va_list_struct va_list[1];
void *__va_arg(void *ap, int type);
#define va_start(ap, parm) ((void)(parm), __builtin_va_info(&(ap)))
#define va_arg_int(ap, t) (*((t *)__va_arg((ap), 1)))
#define va_end(ap) ((void)0)

#endif
