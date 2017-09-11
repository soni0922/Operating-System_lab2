# Operating-System_lab2
Implemented pipe primitives for interprocess communication in Xinu. 
This mechanism will allow processes to open a maximum of NPIPE (set to 10 by default, it is the limit for whole system, not one process) 
pipes. A process can create a pipe and pass the ID of pipe to another process; the two processes can then use the pipe to send and 
receive stream of data -- one process sends data and another receives it. 
