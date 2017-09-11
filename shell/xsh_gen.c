#include <xinu.h>
#include <stdio.h>
#include <string.h>
#include <wordlist.h>

/*------------------------------------------------------------------------
 * xsh_ping - shell command to ping a remote host
 *------------------------------------------------------------------------
 */
int gen_count;
int vowel_count[5];

int readerg(pipid32 pip) {
	//char *buf;
	sleep(5);
	int32 i;
	uint32 clkvar=clktime;
	//char buf[5];
	while(1){
		/*for(i=0;i<5;i++){
			buf[i]='\0';
		}*/
		char buf[5];
		if(SYSERR== pipread(pip, buf, 5)){
		//	kprintf("\nFail to read from pipe\n");
			return 1;

		}else{
			if(buf[0]=='A'){
				vowel_count[0]++;		
			}else if(buf[0]=='E'){
				vowel_count[1]++;
			}else if(buf[0]=='I'){
				vowel_count[2]++;
			}else if(buf[0]=='O'){
				vowel_count[3]++;
			}else if(buf[0]=='U'){
				vowel_count[4]++;
			}
			if(clktime-clkvar >=10){
				clkvar=clktime;
				//print
				kprintf("\nA: %d E: %d I:%d O:%d U:%d\n", vowel_count[0],vowel_count[1],vowel_count[2],vowel_count[3],vowel_count[4]);
				for(i=0;i<5;i++){
					vowel_count[i]=0;
				}

			}
		}
	}
}

shellcmd xsh_gen(int nargs, char *args[])
{
        /* Check for valid number of arguments */

        if (nargs != 3) {
                fprintf(stderr, "%s: invalid arguments\n", args[0]);
                return 1;
        }

	if (nargs == 3 && strncmp(args[1], "|", 2) == 0 && strncmp(args[2], "search", 7) == 0) {
		
		pipid32 pip;
		pip= pipcreate();
		if(pip == SYSERR) {
			fprintf(stderr, "%s: Fail to create the new pipe!\n", args[0]);
			return 1;
		}
		
		pid32 repid;	
		repid = create(readerg, 2048, 20, "readerg1", 1, pip);

		if (SYSERR == pipconnect(pip, currpid, repid)) {
			fprintf(stderr, "%s: Fail: fail to connect the pipe!\n", args[0]);
			return 1;
		} else {
			int32 i;
			gen_count=0;
			for(i=0;i<5;i++){
				vowel_count[i]=0;
			}
			resume(repid);

			//writing process
			srand(clktime);
			uint32 clkvar2=clktime;
			while(1){

				int32 i;
				i = rand();
				i %= 2048;
				if(SYSERR == pipwrite(pip, words[i], 5)){        //len ?
				//	kprintf("\nFail to write into pipe\n");
					return 1;
				}else{
					gen_count++;
				}
				if((clktime-clkvar2) >=5){
					clkvar2=clktime;
					kprintf("\n[gen]: %d\n", gen_count);
					gen_count=0;
				}
			}

		}
	

		return 0;

	}else{
		return 1;
	}

}

