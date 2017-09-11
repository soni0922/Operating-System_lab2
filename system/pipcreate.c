#include <xinu.h>

local int newpipid();

syscall pipcreate(){
	
//	kprintf("pipcreate is called");
	intmask mask; /* Saved interrupt mask */
	mask = disable(); /* Disable interrupts at start of function */

	pipid32 new_pipe_id;
	struct  pipe *pipeptr;

	if ((new_pipe_id=newpipid()) == SYSERR) {	/* no pipe is free */
		restore(mask); /* Restore interrupts before error return */
		return SYSERR;
	}
		pipeptr = &pipetab[new_pipe_id];		
		pipeptr -> pipe_state = PIPE_USED;
		pipeptr -> pipe_owner = currpid;
		pipeptr -> pipe_read_pos = 0;
		pipeptr -> pipe_write_pos = 0;
		pipeptr -> pipe_reader_id = -1;
		pipeptr -> pipe_writer_id = -1;
		pipeptr -> pipe_sem_read = semcreate(0);
		pipeptr -> pipe_sem_write = semcreate(256);


	restore(mask); /* Restore interrupts before normal return*/
	return new_pipe_id ;

}

local   pipid32   newpipid()
{
        uint32  i;                      /* Iterate through all processes*/
        static  pipid32 nextpid = 0;      /* Position in table to try or  */
                                        /*   one beyond end of table    */

        /* Check all NPROC slots */
//	kprintf("\n1\n");
        for (i = 0; i < NPIPE; i++) {
                nextpid %= NPIPE;       /* Wrap around to beginning */
//		kprintf("\nnew pipid: %d and state: %d\n", nextpid,pipetab[nextpid].pipe_state);
                if (pipetab[nextpid].pipe_state == PIPE_FREE) {
                        return nextpid++;
                } else {
                        nextpid++;
                }
        }
        return (pipid32) SYSERR;
}

