/*
 ============================================================================
 Author        : E. Badawi
 Version       : 1.0
 Last modified : November 2017
 To build use  : mpic++ ./AsFar.cpp -o asfar
 To run use    :mpirun -np N --hostfile hosts ./asfar    ex: mpirun -hostfile hosts -np 3 ./asfar
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
#include <unistd.h> //for usleep
using namespace std;


// Check if Ill become passive or no based on the stage

int main(int argc, char *argv[])
{
    int size, rank, mcounter, stage,myID,localCounter,min,tag;
	double time1,time2;
	int inmsg;	//incoming msg first
	int msg; 	//outgoing msg
	MPI_Status Stat;
	MPI_Request req;

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
	bool terminated=false;
	//myID=size-rank+1; to try best case
	mcounter=0; // number of messages I get from other nodes (used for termination)
	msg=myID; //contains my ID
	min=myID;// the initial min is my ID
	tag=1;//if tag=1 then normal message if tag=2 then termination
	int incoming=(rank-1)%size;
	if (rank==0)
		incoming=size-1;
	MPI_Irecv(&inmsg,1, MPI_INT, incoming,tag,MPI_COMM_WORLD,&req);
	int sleepTime=rand()%3000+500;
	usleep(sleepTime);
	int flag=0;
	MPI_Request_get_status( req, &flag, &Stat);
	if(!flag) //spontanous
	{
		MPI_Send(&msg, 1, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
		mcounter++;//increase the counter of received messages
		MPI_Wait( &req, &Stat);//wait the message from another node
		if(inmsg<min)
		{
			min=inmsg;
			MPI_Send(&inmsg, 1, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
			mcounter++;//increase the counter of received messages
		}
		
	} //receved message
	else
	{
		MPI_Send(&msg, 1, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
		mcounter++;//increase the counter of received messages
		if(inmsg<min)
		{
			min=inmsg;
			MPI_Send(&inmsg, 1, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
			mcounter++;//increase the counter of received messages
		}
	}
	while(!terminated)
	{
		//receive ID from left
		int incoming=(rank-1)%size;
		if (rank==0)
			incoming=size-1;
		MPI_Recv(&inmsg, 1, MPI_INT,incoming, MPI_ANY_TAG, MPI_COMM_WORLD, &Stat);
		if(min==inmsg&&Stat.MPI_TAG==1)//I got my value back I am leader I should notify
		{
			//cout<<"I am the leader with value "<<min<<" from process "<<rank<<endl;
			MPI_Send(&inmsg, 1, MPI_INT, (rank+1)%size, 2, MPI_COMM_WORLD);
			mcounter++;//increase the counter of received messages
			terminated=true;
		}
		else if(inmsg<min)//forward incmoing message iff smaller than me
		{
			min=inmsg;
			MPI_Send(&inmsg, 1, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
			mcounter++;//increase the counter of received messages
		}
		else if (Stat.MPI_TAG==2)//notification message
		{
			MPI_Send(&inmsg, 1, MPI_INT, (rank+1)%size, 2, MPI_COMM_WORLD);
			min=inmsg;
			terminated=true;			
			mcounter++;//increase the counter of received messages
		}
		//cout<<"send"
	}


	//cout<<"*"<<endl;
	//if(myID==min)
		//cout<<"I am the leader with value "<<myID<<" and my process "<<rank<<endl;
	int total_messages=0;
	//cout<<"imad"<<endl;
	time2=MPI_Wtime()-time1;
	MPI_Reduce(&mcounter, &total_messages,1, MPI_INT,MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Finalize();
	if(rank==0)
	{	
		//cout<<"Total number of communicated messages equals to "<<total_messages<<endl;
		//cout<<"Max possible number of communicated messages equals to "<<(size)*(size)<<endl; 
		cout<<size<<" "<<total_messages<<" "<<(size)*(size)<<" "<<time2<<endl;
	}

}
