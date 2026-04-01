#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/procinfo.h"
#include "user/user.h"


/* this function will translate process states from
 enum values given to the pi variable from the kernel to readable string */
static char* stateName(int state){ //reads enum int value of state
	switch(state){// depending on value switch case will return its string name 
	// values were determined from enum array in kernel/proc.h
	case 0: return "UNUSED    ";
	case 1: return "EMBRYO    ";
	case 2: return "SLEEPING  ";
	case 3: return "RUNNABLE  ";
	case 4:  return "RUNNING   ";
	case 5:  return "ZOMBIE    ";
	default: return "UNKNOWN   "; // 10 characters exactly (incl. spaces)
	}
}


int
main(int argc, char *argv[]){

	struct procinfo processinfo[NPROC];
	int n;

	n = getprocs(processinfo, NPROC);

	// error handling
	// make sure getprocs didnt return -1 and process info was collected
	if(n < 0){
	fprintf(2, "getprocs failed\n"); // 2 = stderr error message file descriptor
	exit(1); // 1 = error exit code
	}


	printf("PID    PPID   STATE      SIZE    NAME\n");


	for(int i = 0; i<n; i++){
	struct procinfo *pi = &processinfo[i];

		printf("%d      %d      %s %lu   %s\n", pi->pid, pi->ppid,
		 stateName(pi->state),(uint64)pi->sz, pi->name);
	}

	exit(0);

}

