#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h" // struct procinfo header file we made for process attributes

extern struct proc proc[NPROC];
 				/* array of processes of struct proc type,
				so we will be able to access proc functions*/
				/* we will need the array of processes as we go
				through each process and collect their info,*/
				// max 64 process, NPROC = 64 in param.h


extern struct spinlock wait_lock; /*creating a wait_lock variable
					 of struct spinlock type
					to ensure consistent data
					using this synchronization method */

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
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
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if(t == SBRK_EAGER || n < 0) {
    if(growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if(addr + n < addr)
      return -1;
    if(addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
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

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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

uint64
sys_getcourseno(void)
{
	return 3221;
}

uint64
sys_getprocs(void)
{
	//initializing the parameters
	uint64 addr; // struct procinfo *pinfo, procinfo array
	int max;	// int max_procs


	// defining the arguments passed through
	argaddr(0, &addr); /* 1st argument in system call,
				address of process info array */
	argint(1, &max);	/*2nd argument in system call,
				 max processes to be  printed*/


	int infoCollected = 0; // number of proccesses information collected 

	// immediately check if max is a valid positive number
	if(max < 0){
	return -1;
	}
	// Return -1 on error (e.g., invalid pointers or invalid max_procs)


	struct proc *p; /* pointer variable p of struct proc type
			used to access the proc array defined at start of file*/

	struct proc *current = myproc(); /* pointer variable of the
					 current process running to access
					 its pagetable (needed for copyout
					function) */

	struct procinfo pi; /*procinfo variable pi to write procinfo
				 to the procinfo array argument*/ 


	for(p = proc; p < &proc[NPROC] && infoCollected < max; p++){
	/* ( p points to the first process of proc array;
	 p has not yet reached the last process's address in the array
	&& number of processes information is less than the max number of info;
	p pointer moves to the next process in the array) */

	/* as defined in proc.h, wait_lock must be held to access parent process
	and p->lock must be aquired to access process attruibutes to avoid any
	race conditions and early wakeups */
	acquire(&wait_lock);
	acquire(&p->lock);
	/* takes address (&) as the input argument,
	 as release and acquire parameter for struct spinlock input
	 in spinlock.c is set as a pointer */

	if(p->state == UNUSED){
		release(&p->lock);
		release(&wait_lock);
		continue;
	}
	/* attributes of p pointer are not being accessed,
	state becomes UNUSED,so lock is released*/

	// assigning process info to struct pi variable
	pi.pid = p->pid;
	pi.ppid = p->parent ? p->parent->pid : 0;
	// does process have parent? access parent pid if true:return 0 if false
	pi.state = p->state;
	pi.sz = p->sz;
	safestrcpy(pi.name, p->name, sizeof(pi.name));



	release(&p->lock);
	release(&wait_lock);
	//release locks when done accessing process info


	if(copyout(current->pagetable, addr + infoCollected * sizeof(pi),
		 (char*)&pi, sizeof(pi)) < 0){

		return -1; //failure
	}

	infoCollected++; // incr process info collected by 1


} /* end of for loop (n has reached max or
	 info of all running processes have been collected)*/ 


return infoCollected; // return number of information sets collected

}
