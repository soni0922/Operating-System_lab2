#include <xinu.h>

local void cleanup();


syscall pipwrite(pipid32 pip, char *buf, uint32 len){ 

	intmask mask; /* Saved interrupt mask */
        mask = disable(); /* Disable interrupts at start of function */

        if ( isbadpipid(pip) ) {
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }

	struct pipe * pipeptr;
	pipeptr = &pipetab[pip];

	if((pipeptr->pipe_state != PIPE_CONNECTED && pipeptr->pipe_state != PIPE_OTHER) || (pipeptr->pipe_writer_id != currpid) || buf==NULL || len<0 ) {
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
	}				        

	uint32 i;
	int32 no_of_bytes_written =0;
	
	if((pipeptr->pipe_state) == PIPE_OTHER){
		cleanup(pipeptr);
		return no_of_bytes_written;
	}

	for(i = 0;i < len;i++){			//if len > avl_space then write till avl_space and return

		if((pipeptr->pipe_state)==PIPE_FREE || (pipeptr->pipe_state)==PIPE_OTHER){
			cleanup(pipeptr);
			break;
		}

		wait(pipeptr->pipe_sem_write);

		if((pipeptr->pipe_state)==PIPE_FREE || (pipeptr->pipe_state)==PIPE_OTHER){
                        cleanup(pipeptr);
                        break;
                }	
		
		//write to pipe
		
		pipeptr->pipe_buf[pipeptr->pipe_write_pos] = buf[i];
		pipeptr->pipe_write_pos++;
		pipeptr->pipe_write_pos %= PIPE_SIZE;
		no_of_bytes_written++;
	
	
		signal(pipeptr->pipe_sem_read);
	}


        restore(mask); /* Restore interrupts before normal return*/
        return no_of_bytes_written;

}

local void cleanup(struct pipe * pipeptr){

	pipeptr -> pipe_state = PIPE_USED;
        //pipeptr -> pipe_owner = currpid;
        pipeptr -> pipe_read_pos = 0;
        pipeptr -> pipe_write_pos = 0;
        pipeptr -> pipe_reader_id = -1;
        pipeptr -> pipe_writer_id = -1;
        semreset(pipeptr -> pipe_sem_read,0);
        semreset(pipeptr -> pipe_sem_write,PIPE_SIZE);	
	
}

