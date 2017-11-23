# CSI5308-Project

In this project, the C++ and Message Passing Interface were exploits to implementing minmax, all the way and as far election algorithm in unidirectional rings, and capture the number of exchanged messages in each one of the algorithms, then compare the acquired messages number with the worst case complexity for each of the algorithms.


In all of the implemented algorithms, the number of lunched processes represents the number of nodes in the rings. The connections between the nodes is identified by the fact that MPI processes have distinct IDs (process rank), in which these IDs can be viewed as a ring. Figure 1 represents 8 lunched processes connected with each other in such a way to create ring. The processes IDs can be used as a distinct IDs for the corresponding nodes, but this will limit the simulation with one case, which is the ascending order or descending order. Another way to assign distinct IDs is to use a master process to generate random distinct IDs and distribute them between the other processes. The later way of assigning IDs was used.


To build the algorithm:
mpic++ ./code_file.cpp -o executable_file 
For Example, to build AS Far code: mpic++ ./AsFar.cpp -o asfar
To run the algorithm:
mpirun -np N --hostfile hosts ./ executable_file 
For Example, to run AS Far executable file: mpirun -hostfile hosts -np 30 ./asfar
Where:
N: represent the number of lunched processes (the number of nodes)
Hosts: a file that contains the IPs of the used cluster (1 IP per line)
