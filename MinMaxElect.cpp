/*
 ============================================================================
 Author        : E. Badawi
 Version       : 1.0
 Last modified : November 2017
 To build use  : mpic++ ./MinMaxElect.cpp -o minmax
 To run use    :mpirun -np N --hostfile hosts ./minmax    ex: mpirun -hostfile hosts -np 3 ./minmax
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
bool minmax(int myval, int newval, int stage)
{
	if(stage%2==0)
	{
		if (myval<newval)
			return true;
		return false;	
	}
	else
	{
		if (myval<newval)
			return false;
		return true;	
	}
	
}

int main(int argc, char *argv[])
{
    int size, rank, mcounter, stage,myID;
	double time1,time2;
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
	if (rank==0)
		time1=MPI_Wtime(); 
//Start the real alg. from here
	//cout<<"From process "<<rank<<" my ID equals "<<myID<<endl;
	bool active =true;
	bool terminated=false;
	stage=1; // the stage I am in
	mcounter=0; // number of messages I will send
	msg[0]=myID; //contains my ID
	msg[1]=stage;// the current stage
	MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
	mcounter++;
	while(!terminated)
	{
		//receive ID with stage from left
		int incoming=(rank-1)%size;
		if (rank==0)
			incoming=size-1;
		MPI_Recv(&inmsg, 2, MPI_INT,incoming, 1, MPI_COMM_WORLD, &Stat);
		if (active)
		{
			if(inmsg[0]==myID)
			{
				//cout<<"I am the leader in stage "<<inmsg[1]<<" and in process "<<rank<<" my value equals "<<inmsg[0]<<endl;
				//send termination to the other nodes
				msg[0]=-1;
				msg[1]=rank;
				MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
				mcounter++;//ii
				terminated=true;
			}
			active=minmax(myID,inmsg[0],inmsg[1]);
			//cout<<"*"<<active<<" "<<stage<<"* ";
			
			if(active&&!terminated)//if I survive I will start the second stage
			{
				//increase the stage
				stage++;
				myID=inmsg[0];
				msg[0]=myID;
				msg[1]=stage;
				MPI_Send(&msg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
				mcounter++;
			}
		}
		else
		{
			//send the incoming message to the next node since I am passive
			if(inmsg[0]!=-1)
			{
				MPI_Send(&inmsg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
				mcounter++;//ii
			}
			else
			{
				{
					//cout<<""<<((rank+1)%size)<<endl;
					MPI_Send(&inmsg, 2, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
					mcounter++;//ii
				}
				terminated=true;
			}
		}	
	}
	MPI_Barrier( MPI_COMM_WORLD );
	//cout<<"*"<<endl;
	int total_messages=0;
	//cout<<"imad"<<endl;
	time2=MPI_Wtime()-time1;
	MPI_Reduce(&mcounter, &total_messages,1, MPI_INT,MPI_SUM, 0, MPI_COMM_WORLD);
	
	MPI_Finalize();
	if(rank==0)
	{	
		//cout<<"Total number of communicated messages equals to "<<total_messages<<endl;
		//cout<<"Max possible number of communicated messages equals to "<<(1.44*size*log2(size)+size)<<endl;
		cout<<size<<" "<<total_messages<<" "<<(1.44*size*log2(size)+size)<<" "<<time2<<endl;
	}
}
