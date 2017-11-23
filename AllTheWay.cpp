/*
 ============================================================================
 Author        : E. Badawi
 Version       : 1.0
 Last modified : November 2017
 To build use  : mpic++ ./AllTheWay.cpp -o alltheway
 To run use    :mpirun -np N --hostfile hosts ./alltheway    ex: mpirun -hostfile hosts -np 3 ./alltheway
 //Change the copy to referencing
 ============================================================================
 */
#include<iostream>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>
#include<algorithm>    // std::shuffle
#include<math.h>

using namespace std;


// Check if Ill become passive or no based on the stage

int main(int argc, char *argv[])
{
    int size, rank, mcounter, stage,myID,localCounter,min;
	int inmsg[2];	//incoming msg first element is the id and 2nd element is the stage
	int msg[2]; 	//outgoing msg first element is the id and 2nd element is the stage
	MPI_Status Stat;

        ////////////////////////////////////
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	if(rank==0)//limit 1 process to give distinct IDs
	{
		srand (time(NULL));
		int *values=new int[size];
		for(int i=1;i<=size;i++)
			values[i-1]=i;
		//*
		for (int i=0; i<size; i++) 
		{
            int r1 = rand() % size*2;  // generate a random position
			r1=r1/2;
			int r2 = rand() % size*3;  // generate a random position
			r2=r2/3;
            int temp = values[r2]; values[r2] = values[r1]; values[r1] = temp;
        }
		myID=values[0];
		for(int i=1;i<size;i++)
			MPI_Send(&values[i], 1, MPI_INT, i, 1, MPI_COMM_WORLD);
		//for(int i=0;i<size;i++)
			//cout<<values[i]<<" "<<endl;;
	}
	else
	{
		MPI_Recv(&myID,1, MPI_INT,0, 1, MPI_COMM_WORLD, &Stat);
	}
	MPI_Barrier( MPI_COMM_WORLD ); // wait until all nodes get their IDs
//Start the real alg. from here
	bool terminated=false;
	mcounter=0; // number of messages I get from other nodes (used for termination)
	msg[0]=myID; //contains my ID
	msg[1]=0;// counter for the number of visited nodes
	min=myID;// the initial min is my ID
	MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
	while(!terminated)
	{
		//receive ID from left
		int incoming=(rank-1)%size;
		if (rank==0)
			incoming=size-1;
		MPI_Recv(&inmsg, 2, MPI_INT,incoming, 1, MPI_COMM_WORLD, &Stat);
		mcounter++;//increase the counter of received messages
		if(min>inmsg[0])//find if the incmoing message from node smaller than me
			min=inmsg[0];
		inmsg[1]=inmsg[1]+1;//increase the incoming message counter
		if(inmsg[0]==myID&&inmsg[1]==mcounter)
		{
			terminated=true;				
		}
		MPI_Send(&inmsg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
		//cout<<"send"
	}


	//cout<<"*"<<endl;
	if(myID==min)
		cout<<"I am the leader with value "<<myID<<" and my process "<<rank<<endl;
	int total_messages=0;
	//cout<<"imad"<<endl;
	MPI_Reduce(&mcounter, &total_messages,1, MPI_INT,MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Finalize();
	if(rank==0)
	{	
		cout<<"Total number of communicated messages equals to "<<total_messages<<endl;
		cout<<"Max possible number of communicated messages equals to "<<(size)*(size)<<endl;
	}

}
