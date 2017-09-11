/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);

	//pipe implementation
	//if owner gets killed
	struct pipe * pipeptr;
	for(i=0;i<NPIPE;i++){
		pipeptr=&pipetab[i];
		if(pipeptr->pipe_owner == pid){
			//delete the pipe
			pipeptr -> pipe_state = PIPE_FREE;
		        pipeptr -> pipe_owner = -1;
		        pipeptr -> pipe_read_pos = 0;
		        pipeptr -> pipe_write_pos = 0;
		        pipeptr -> pipe_reader_id = -1;
		        pipeptr -> pipe_writer_id = -1;
	        	semdelete(pipeptr -> pipe_sem_read);
	        	semdelete(pipeptr -> pipe_sem_write);
		}

		else if((pid == pipeptr->pipe_reader_id || pid == pipeptr->pipe_writer_id) ){
			if(pipeptr->pipe_state == PIPE_OTHER){
				pipeptr -> pipe_state = PIPE_USED;
				//pipeptr -> pipe_owner = currpid;
				pipeptr -> pipe_read_pos = 0;
				pipeptr -> pipe_write_pos = 0;
				pipeptr -> pipe_reader_id = -1;
				pipeptr -> pipe_writer_id = -1;
				semreset(pipeptr -> pipe_sem_read,0);
				semreset(pipeptr -> pipe_sem_write,256);
			}

			if(pid == pipeptr->pipe_reader_id){
			
				pipeptr -> pipe_state = PIPE_OTHER;
				struct  sentry *semptr;         /* Ptr to semaphore table entry */
				qid16   semqueue;               /* Semaphore's process queue ID */
				pid32   pid_new;                    /* ID of a waiting process      */

				if (semtab[pipeptr->pipe_sem_write].sstate!=S_FREE) {

					semptr = &semtab[pipeptr->pipe_sem_write];
					semqueue = semptr->squeue;      /* Free any waiting processes */
					resched_cntl(DEFER_START);
					while ((pid_new=getfirst(semqueue)) != EMPTY)
						ready(pid_new);
					//semptr->scount = count;         /* Reset count as specified */
					resched_cntl(DEFER_STOP);
				}
			}
			
			if(pid == pipeptr->pipe_writer_id){
				
				pipeptr -> pipe_state = PIPE_OTHER;
				struct  sentry *semptr;         /* Ptr to semaphore table entry */
				qid16   semqueue;               /* Semaphore's process queue ID */
				pid32   pid_new;                    /* ID of a waiting process      */

				if (semtab[pipeptr->pipe_sem_read].sstate!=S_FREE) {

					semptr = &semtab[pipeptr->pipe_sem_read];
					semqueue = semptr->squeue;      /* Free any waiting processes */
					resched_cntl(DEFER_START);
					while ((pid_new=getfirst(semqueue)) != EMPTY)
						ready(pid_new);
					//semptr->scount = count;         /* Reset count as specified */
					resched_cntl(DEFER_STOP);
				}	
			}
		}

	}
	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
