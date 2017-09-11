#include <xinu.h>

local void cleanup(struct pipe *);

syscall pipdisconnect(pipid32 pip){

        intmask mask; /* Saved interrupt mask */
        mask = disable(); /* Disable interrupts at start of function */

        if(isbadpipid(pip)){
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }
        struct pipe* pipeptr;
        pipeptr = &pipetab[pip];

        if ((currpid != pipeptr->pipe_reader_id) && (currpid != pipeptr->pipe_writer_id)) {
	//	kprintf("\nSYSERR of pipdisconnect,pipe state: %d, reader: %d, writer: %d, owner: %d, currpid: %d\n",pipeptr->pipe_state,pipeptr->pipe_reader_id,pipeptr->pipe_writer_id,pipeptr->pipe_owner,currpid);
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }
	if(pipeptr->pipe_state != PIPE_CONNECTED && pipeptr->pipe_state != PIPE_OTHER){
         //       kprintf("\nSYSERR of pipdisconnect,pipe state: %d, reader: %d, writer: %d, owner: %d, currpid: %d\n",pipeptr->pipe_state,pipeptr->pipe_reader_id,pipeptr->pipe_writer_id,pipeptr->pipe_owner,currpid);
                restore(mask); /* Restore interrupts before error return */
                return SYSERR;
        }

	
	if(pipeptr->pipe_state == PIPE_OTHER){
		cleanup(pipeptr);
	}

	if(currpid == pipeptr->pipe_reader_id){
	
		pipeptr -> pipe_state = PIPE_OTHER;
		struct  sentry *semptr;         /* Ptr to semaphore table entry */
        	qid16   semqueue;               /* Semaphore's process queue ID */
        	pid32   pid;                    /* ID of a waiting process      */

	        if (semtab[pipeptr->pipe_sem_write].sstate!=S_FREE) {

	        	semptr = &semtab[pipeptr->pipe_sem_write];
		        semqueue = semptr->squeue;      /* Free any waiting processes */
	        	resched_cntl(DEFER_START);
		        while ((pid=getfirst(semqueue)) != EMPTY)
		                ready(pid);
			//semptr->scount = count;         /* Reset count as specified */
        		resched_cntl(DEFER_STOP);
		}
	}
	
	if(currpid == pipeptr->pipe_writer_id){

		pipeptr -> pipe_state = PIPE_OTHER;
        	struct  sentry *semptr;         /* Ptr to semaphore table entry */
                qid16   semqueue;               /* Semaphore's process queue ID */
                pid32   pid;                    /* ID of a waiting process      */

                if (semtab[pipeptr->pipe_sem_read].sstate!=S_FREE) {

                        semptr = &semtab[pipeptr->pipe_sem_read];
                        semqueue = semptr->squeue;      /* Free any waiting processes */
                        resched_cntl(DEFER_START);
                        while ((pid=getfirst(semqueue)) != EMPTY)
                                ready(pid);
                        //semptr->scount = count;         /* Reset count as specified */
                        resched_cntl(DEFER_STOP);
                }	
	}

        restore(mask); /* Restore interrupts before normal return*/
        return OK ;

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


