#include <xinu.h>

local void cleanup(struct pipe *);

syscall  pipread(pipid32 pip, char *buf, uint32 len){

        intmask mask; /* Saved interrupt mask */
        mask = disable(); /* Disable interrupts at start of function */

        if (isbadpipid(pip)) {
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }

	struct pipe * pipeptr;
	pipeptr = &pipetab[pip];

	if((pipeptr->pipe_state != PIPE_CONNECTED && pipeptr->pipe_state != PIPE_OTHER) || (pipeptr->pipe_reader_id != currpid) || buf==NULL || len<0) {
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }

	int32 i,no_of_bytes_read =0, bytes_to_read;
	bool8 flag = FALSE;
	
	int32 avl_data = semcount(pipeptr->pipe_sem_read);	
	
	if(avl_data == 0 && (pipeptr->pipe_state!= PIPE_OTHER) ){
		wait(pipeptr->pipe_sem_read);
		flag = TRUE;
	}
	if(flag==TRUE){
		avl_data = semcount(pipeptr->pipe_sem_read)+1;
	}else{
		avl_data = semcount(pipeptr->pipe_sem_read);
	}
	if(avl_data <= len){
		bytes_to_read = avl_data;
	}else{
		bytes_to_read = len;
	}

	for(i=0; i<bytes_to_read; i++){			//if len > avl_space then write till avl_space and return
		
		if(pipeptr->pipe_state == PIPE_FREE){
			//cleanup(pipeptr);
			break;
		}
		if(flag == FALSE){
			wait(pipeptr->pipe_sem_read);
		}

		if(pipeptr->pipe_state == PIPE_FREE){
                        //cleanup(pipeptr);
                        break;
                }
			
		//read pipe
		else{
			buf[i] = pipeptr->pipe_buf[pipeptr->pipe_read_pos];
			pipeptr->pipe_read_pos++;
			pipeptr->pipe_read_pos %= PIPE_SIZE;
			no_of_bytes_read++;
		}
	
		signal(pipeptr->pipe_sem_write);
		flag= FALSE;
	}
	if(pipeptr->pipe_state == PIPE_OTHER){
		cleanup(pipeptr);
	}

        restore(mask); /* Restore interrupts before normal return*/
        return no_of_bytes_read ;

}

local void cleanup(struct pipe * pipeptr){

        pipeptr -> pipe_state = PIPE_USED;
        //pipeptr -> pipe_owner = currpid;
        pipeptr -> pipe_read_pos = 0;
        pipeptr -> pipe_write_pos = 0;
        pipeptr -> pipe_reader_id = -1;
        pipeptr -> pipe_writer_id = -1;
        semreset(pipeptr -> pipe_sem_read,0);
        semreset(pipeptr -> pipe_sem_write,256);

}


