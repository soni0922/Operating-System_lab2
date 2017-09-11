#include <xinu.h>

syscall pipconnect(pipid32 pip, pid32 writer, pid32 reader){

	intmask mask; /* Saved interrupt mask */
	mask = disable(); /* Disable interrupts at start of function */
	
	if ( isbadpipid(pip)) {
		restore(mask); /* Restore interrupts before error return */
		return SYSERR;
	}

	struct pipe *pipeptr;
	pipeptr = &pipetab[pip];
	if((pipeptr->pipe_state != PIPE_USED) || (writer == reader) || isbadpid(writer) || isbadpid(reader) ) {
	        restore(mask); /* Restore interrupts before error return */
		return SYSERR;
	}
					
	pipeptr->pipe_reader_id = reader;
	pipeptr->pipe_writer_id = writer;
	pipeptr->pipe_state = PIPE_CONNECTED;
	pipeptr->pipe_read_pos=0;	//CHECK
	pipeptr->pipe_write_pos=0;

	restore(mask); /* Restore interrupts before normal return*/
	return OK ;

}
