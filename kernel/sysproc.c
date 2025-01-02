#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}



int
sys_pgaccess(void) {
  uint64 start;
  int npages;
  uint64 abitsaddr;
  uint64 va;
  uint64 mask;
  uint64 abits;

  struct proc *p = myproc();

  // Kiểm tra và lấy tham số từ syscall
  // if (argaddr(0, &start) < 0 || argint(1, &npages) < 0 || argaddr(2, &abitsaddr) < 0) {
  //   return -1;
  // }
  if (argaddr(0, &start), start == 0)
    return -1;
  if (argint(1, &npages), npages == 0)
    return -1;
  if(argaddr(2, &abitsaddr),abitsaddr == 0)
    return -1;
  // Kiểm tra số lượng trang hợp lệ
  if (npages <= 0 || npages > 64) {
    return -1;
  }

  mask = 1;
  abits = 0;

  // Duyệt qua từng trang
  for (va = start; va < start + PGSIZE * npages; va += PGSIZE) {
    pte_t *pte = walk(p->pagetable, va, 0);

    // Kiểm tra PTE hợp lệ
    if (pte == 0 || (*pte & PTE_V) == 0) {
      continue; // Trang không hợp lệ, bỏ qua
    }

    // Kiểm tra nếu bit A được đặt
    if (*pte & PTE_A) {
      abits |= mask; // Đánh dấu bit tương ứng
      *pte &= ~PTE_A; // Xóa bit A
    }

    mask <<= 1; // Dịch bitmask
  }

  // Sao chép kết quả vào không gian người dùng
  if (copyout(p->pagetable, abitsaddr, (char *)&abits, sizeof(uint64)) < 0) {
    return -1;
  }

  return 0; // Thành công
}



uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
