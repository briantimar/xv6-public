#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"


char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

//// ticket locks

void initlock_t(volatile struct lock_t *lk)
{
  lk->ticket = 0;
  lk->turn = 0;
}

void lock_t(volatile struct lock_t *lk)
{
  int turn = fetchadd(&(lk->ticket), 1);
  while (turn != lk->turn)
  {
  }
}

void unlock_t(volatile struct lock_t *lk)
{
  fetchadd(&(lk->turn), 1);
}


/// threads api

// creates a new thread; returns the pid if successful else -1
// first arg is the function in which thread runs, second is the arg
// function must exit() rather than return
int thread_create(void (*f)(void *), void *arg){
  void * stack;
  if ((stack = malloc(4096)) < 0)
    return -1;
  return clone(f, arg, stack);
}

// waits for a single child thread to exit, and returns its pid when it does so
// if no child threads are present returns -1
int thread_join(void) {
  void * stack;
  int pid = join(&stack);
  free(stack);
  return pid;
}


