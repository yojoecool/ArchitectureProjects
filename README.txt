README
Group 18
COMP 4300
Bridges Penn: bjp0008
James Daniel: jhd0008

Compilation Instructions:
1. Navigate to the folder that contains “GPRMem.cpp”, “lab3a.s”, “lab3b.s”, “lab3c.s”, “GPRMem.h”, “pipeSim.cpp”, and “Makefile” within your command line
2. While in the folder, type “make” to run the Makefile. This will create a .o file called “pipeSimulator”
3. Type “./pipeSimulator” to run the file

Usage Instructions:
1. After starting the program, type either “lab3a.s”, “lab3b.s”, or “lab3c.s” when prompted to enter the file you wish to read from.
2a. If lab3a.s or lab3c.s were chosen, the program will then output the number of instructions, number of cycles, and the number of NOPs.
2b. If lab3b.s was chosen, the program will then output whether or not the string is a palindrome, along with the number of instructions, number of cycles, and the number of NOPs.

Design Issues:
1. In the beginning of development, later steps in the pipeline could influence earlier steps.
2. We originally didn’t implement branches to wait a cycle before branching. Using the pull method, the fetch stage immediately received the new PC instead of waiting until the next cycle.
3. Until we focused on forwarding, there was confusion about the importance and implementation of opA and opB. 

Results:
lab3a.s:
Instruction Count:107	Cycles:284	NOP Count:37

lab3b.s: input:”palindrome”
Instruction Count:124	Cycles:284	NOP Count:61

lab3c.s:
Instruction Count:68	Cycles:156	NOP Count:17 