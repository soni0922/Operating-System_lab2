#include <xinu.h>

syscall pipdelete(pipid32 pip){

        intmask mask; /* Saved interrupt mask */
        mask = disable(); /* Disable interrupts at start of function */

	if(isbadpipid(pip)){
		restore(mask); /* Restore interrupts before error return */
		return SYSERR;
	}
	struct pipe* pipeptr;
	pipeptr = &pipetab[pip];

        if (pipeptr->pipe_owner != currpid) {
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }

	pipeptr -> pipe_state = PIPE_FREE;
        pipeptr -> pipe_owner = -1;
        pipeptr -> pipe_read_pos = 0;
        pipeptr -> pipe_write_pos = 0;
        pipeptr -> pipe_reader_id = -1;
        pipeptr -> pipe_writer_id = -1;
        semdelete(pipeptr -> pipe_sem_read);
        semdelete(pipeptr -> pipe_sem_write);

        restore(mask); /* Restore interrupts before normal return*/
        return OK ;

}

