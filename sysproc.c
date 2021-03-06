#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"
#include "times.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// user ticks as well as total ticks since process started executing.
int 
sys_gettimes(void) {
  struct times *t;
  struct proc *p = myproc();
  uint curtick;
  if (argptr(0, (char**) &t, sizeof(struct times)) < 0)
    return -1;
  
  acquire(&tickslock);
  curtick = ticks;
  release(&tickslock);

  t->procticks = p->ticks;
  t->allticks = curtick - p->starttick;

  return 0;
}




// populate a process stat table with current proc info
int 
sys_getpstat(void)
{ 
  struct pstat *ps;
  
  // check that the user's pointer is within bounds, and if so copy the result
  // into our ps.
  if (argptr(0, (char**)&ps, sizeof( struct pstat)) < 0) {
    return -1;
  }

  // ask proc.c routine to fill in all process data
  writepstat(ps);
  return 0;
}

int 
sys_mprotect(void) // void *addr, int len
{
  int start, len;
  
  if ((argint(0, &start) < 0) || (argint(1, &len) < 0) ){
    return -1;
  }
  return mprotect((void*) start, len);
}

int
sys_munprotect(void) //void *addr, int len
{
  int start, len;

  if ((argint(0, &start) < 0) || (argint(1, &len) < 0))
  {
    return -1;
  }
  return munprotect((void*) start, len);
}

// int clone(void (*)(void *), void *, void*)
int sys_clone(void){ 

  int fn, arg;
  char * stack;
  if ((argint(0, &fn) < 0) || (argint(1, &arg) < 0) || (argptr(2, &stack, PGSIZE) < 0))
    return -1;
  return clone((void (*)(void*)) fn, (void *) arg, (void*) stack);
}

// int join(void **)
int sys_join(void) {
  char* stack;
  // pointer and the stack it points to should both be in user space
  if (argptr(0,&stack, sizeof(int) ) < 0)
    return -1;
  return join((void**) stack);
}