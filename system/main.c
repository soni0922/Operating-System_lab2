/* main.c - main */
 
#include <xinu.h>
#include <stdio.h>
#include <string.h>
/************************************************************************/
/* */
/* main - main program for testing Xinu */
/* */
/************************************************************************/

char wordsbs[7500] = "The Computational Research Infrastructure for Science (CRIS), developed at Purdue under the technical leadership of Peter Baker and the scientific supervision of Professor Elisa Bertino, is now available as open source.Bertino said, Today.s scientists must conduct relevant, verifiable research in a rapidly changing collaborative landscape, with an ever-increasing scale of data. It has come to a point where research activities cannot scale at the rate required without improved cyberinfrastructure (CI). Developed to address these challenges, CRIS provides an easy to use, scalable and collaborative scientific data management and workflow cyberinfrastructure. CRIS offers:semantic definition of scientific data using domain vocabularies embedded provenance for all levels of research activity (data, workflows, tools, etc.) easy integration of existing heterogeneous data and computational tools on local or remote computers automatic data quality monitoring for syntactic and domain standards shareable yet secure access to research data, computational tools and equipment CRIS has been used in several different areas of research at Purdue, including agronomy, biochemistry, bioinformatics and healthcare engineering. Bertino said, The CRIS architecture has been designed based on an extensive requirement analysis carried out in collaboration with colleagues from several colleges at Purdue. I feel confident that CRIS well meets the data management needs of scientific research.";

int sem;
pipid32 wrpid1, repid1;

void writer(pipid32 pip, int len, char c, int s) {
	int tlen = strnlen(wordsbs, 8000);
	if (SYSERR == pipwrite(pip, wordsbs, tlen)) {
		kprintf("Write %c: Fail to write into pip!\r\n", c);
	}
	
	sleep(3);
	wait(sem);
	if (SYSERR == pipdisconnect(pip)) {
		kprintf("Writer %c: Fail to disconnect the pip!\r\n", c);
	}else {
		kprintf("Writer %c: disconnect the pip successfully\r\n", c);
	}

	if (SYSERR == pipdelete(pip)) {
		kprintf("Writer %c: Fail to delete the pip!\r\n", c);
	}else {
		kprintf("Writer %c: delete the pip successfully\r\n", c);
	}
   
	if(c=='b') {
		if(SYSERR == pipconnect(pip, wrpid1, repid1)) {
			kprintf("Fail: fail to reconnect pip!\r\n");
			
		} else {
			kprintf("Pass: reconnect pip!\r\n");
		}
	}

	signal(sem);

	return;
}
void reader(pipid32 pip, int len, char c, int s) {
	int length = 0;
	char mybuf[7500];
	int mylen;

	int success = 1;
	int tlen = strnlen(wordsbs, 8000);
	while (length < tlen) {
		mylen = pipread(pip, &mybuf[length], tlen-length);
		if (SYSERR == mylen) {
			kprintf("Reader %c: Fail to read from the pip\r\n", c);
			success = 0;
			break;
		} else {
			kprintf("Reader %c: read %d bytes from pip\r\n", c, mylen);
		}
		length += mylen;
		sleepms(s);
	}

	
	if( success ) {
		kprintf("Reader %c finish characters reading!\r\n");
		int flag = 1;
		int i = 0;
		while( i < tlen) {
			if(mybuf[i] != wordsbs[i]) {
				flag = 0;
				break;
			}
			i++;
		}

		if(flag) {
			kprintf("Pass reader %c: read the right characters!\n\r", c);
		} else {
			kprintf("Fail reader %c: read wrong characters!\n\r", c);
		}
	} else {
		
		wait(sem);
		if (SYSERR == pipdisconnect(pip)) {
			kprintf("Reader %c: Fail to disconnect the pip!\r\n", c);
		} else {
			kprintf("Reader %c: disconnect the pip successfully!\r\n", c);
		}	
		signal(sem);
	}
	return;
}


 

int main(int argc, char **argv) {
	sem = semcreate(1);
	pipid32 pip[11];

	kprintf("---------------Test #1: pip create------------------\r\n");
	int i;
	for(i=0; i<11; i++) {
		pip[i] = pipcreate();
		if(pip[i] == SYSERR) {
			kprintf("Fail to create the %dth pip!\r\n",  i+1);
			if(i==0) {
				return 0;
			}
		}
	}


	kprintf("-------------Test #2: pip connect-----------------\r\n");
	pipid32 ivpip = 100;

	pid32 wrpid2, wrpid3;
	pid32 repid2, repid3;
	pid32 ivpid = 100;

	wrpid1 = create(writer, 2048, 20, "writer1", 4, pip[0], 1000, 'a', 0);
	repid1 = create(reader, 2048, 20, "reader1", 4, pip[0], 1000, 'a', 100);
	wrpid2 = create(writer, 2048, 20, "writer2", 4, pip[1], 1000, 'b', 100);
	repid2 = create(reader, 9000, 20, "reader2", 4, pip[1], 1000, 'b', 100);

	if (SYSERR == pipconnect(ivpip, wrpid1, repid1)) {
		kprintf("Pass: fail to connect invalid pip!\r\n");
	} else {
		kprintf("Fail: connect to invalid pip!\r\n");
	}

	if (SYSERR == pipconnect(pip[1], wrpid2, repid2)) {
		kprintf("Fail: fail to connect valid pip!\r\n");
		return 0;
	} else {
		kprintf("Pass: connect to valid pip!\r\n");
	}

	if (SYSERR == pipconnect(pip[1], wrpid1, repid1)) {
		kprintf("Pass: fail to connect a pip twice!\r\n");
	} else {
		kprintf("Fail: connect to a pip twice!\r\n");
	}

	kprintf("-----------------------Test #3: Pip read & write & delete & disconnect -----------------\r\n");

	
	wrpid3 = create(writer, 2048, 20, "writer3", 4, pip[1], 1000, 'c', 0);
	repid3 = create(reader, 2048, 20, "reader3", 4, pip[1], 1000, 'c', 100);


	resume(wrpid3);
	resume(repid3);

	sleep(2);
	resume(wrpid2);
	resume(repid2);
	
	while(1) {
		sleep(100);
	}
	return OK;
}

