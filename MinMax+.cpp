/*
 ============================================================================
 Author        : E. Badawi
 Version       : 1.0
 Last modified : November 2017
 To build use  : mpic++ ./MinMax+.cpp -o minmax+
 To run use    :mpirun -np N --hostfile hosts ./minmax+    ex: mpirun -hostfile hosts -np 10 ./minmax+
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

int fib(int n)
{
  if(1 == n || 2 == n)
  {
      return 1;
  }
  else
  {
      return fib(n-1) + fib(n-2);
  }

}

int main(int argc, char *argv[])
{
    int size, rank, mcounter, stage,myID;
	double time1,time2;
	int inmsg[3];	//incoming msg first element is the id, 2nd element is the stage and 3rd is counter
	int msg[3]; 	//outgoing msg first element is the id, 2nd element is the stage and 3rd is counter
	MPI_Status Stat;

        ////////////////////////////////////
	MPI_Init (&argc, &argv);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size (MPI_COMM_WORLD, &size);
	int *nodeArray=new int[size];
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
	time1=MPI_Wtime();
//Start the real alg. from here
	//cout<<"From process "<<rank<<" my ID equals "<<myID<<endl;
	int old_stage=-1,old_val=-1;
	bool active =true;
	bool terminated=false;
	stage=1; // the stage I am in
	mcounter=0; // number of messages I will send
	msg[0]=myID; //contains my ID
	msg[1]=stage;// the current stage
	msg[2]=fib(stage+2);
	MPI_Send(&msg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
	mcounter++;
//cout<<"Check Point #1 "<<endl;
	while(!terminated)
	{
		//receive ID with stage from left
		int incoming=(rank-1)%size;
		if (rank==0)
			incoming=size-1;
		MPI_Recv(&inmsg, 3, MPI_INT,incoming, 1, MPI_COMM_WORLD, &Stat);
//cout<<"Check Point #2 from process "<<rank<<" and stage "<<inmsg[1]<<" and active equals "<<active<<" "<<terminated<<endl;
		if (active)//if the node is actice
		{
			if(inmsg[0]==myID)
			{
				//cout<<"I am the leader in stage "<<inmsg[1]<<" and in process "<<rank<<" my value equals "<<inmsg[0]<<endl;
				//send termination to the other nodes
				msg[0]=-1;
				msg[1]=rank;
				mcounter++;//ii
				terminated=true;
				MPI_Send(&msg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
//cout<<"Check Point #3 from process "<<rank<<" and stage "<<inmsg[1]<<" and active equals "<<active<<" "<<terminated<<endl;

			}			
			else//if I survive I will start the second stage Rule #3
			{
//cout<<"Check Point #4 from process "<<rank<<" and stage "<<inmsg[1]<<" and active equals "<<active<<endl;//<<" "<<terminated<<endl;
				if(inmsg[1]>stage)//I got a message from higher stage Ill be defeated Rule #3 and #5
				{
//cout<<"I got defeated "<<stage<<" "<<inmsg[1]<<endl;
					active=false;
					inmsg[1]--;
					MPI_Send(&inmsg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD); // just forward the messages as it is
					mcounter++;//ii
				}
				else if(stage==inmsg[1])//I got a message that I am waiting
				{
					active=minmax(myID,inmsg[0],inmsg[1]);
//cout<<active<<" ";
					//cout<<"*"<<active<<" "<<stage<<"* ";
					if(!active &&inmsg[1]%2==0)//Rule #4 save my data if I got defeated in even stage
					{
						old_stage=stage;
						old_val=myID;
					}
					if (active)
					{
						stage=inmsg[1]+1;
						myID=inmsg[0];
						msg[0]=myID;
						msg[1]=stage;
						msg[2]=fib(stage+2);
						if(stage%2==1)
							msg[2]=0;
						MPI_Send(&msg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
						mcounter++;
					}
				}
			}
		}
		else //if the node is passive
		{
			//send the incoming message to the next node since I am passive
			if(inmsg[0]!=-1)//still working
			{
				if(stage%2==0)//Rule #1  Even stages logic
				{
					inmsg[2]--;
					if(inmsg[2]!=0)// there is still nodes I can visit
					{
						MPI_Send(&inmsg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
						mcounter++;//ii
//cout<<"even stage "<<inmsg[2]<<endl;
					}
					else // I reached zero so Ill activate the current node and give it my value Rule #2
					{
						active=true;//Ill activate my self
						stage=inmsg[1]+1;
						myID=inmsg[0];
						msg[0]=myID;
						msg[1]=stage;
						msg[2]=0;//no need to set msg[2] since this will be an odd stage
						MPI_Send(&msg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
						mcounter++;//ii
					}
				}
				else //Rule #4 Odd stages logic 
				{
					if((old_val!=-1)&&(inmsg[0]<old_val))//Rule #4 Ill become active and start next stage with this value
					{
						active=true;//Ill activate my self
						stage=inmsg[1]+1;
						myID=inmsg[0];
						msg[0]=myID;
						msg[1]=stage;
						msg[2]=fib(stage+2); //since this will be an even stage
						old_val=-1;
						MPI_Send(&msg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
						mcounter++;//ii
//cout<<"odd stage "<<stage<<endl;
					}
					else //just forward the value
					{
						MPI_Send(&inmsg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
						mcounter++;//ii
					}
				}
			}
			else//termination 
			{
				{
					//cout<<""<<((rank+1)%size)<<endl;
					MPI_Send(&inmsg, 3, MPI_INT, (rank+1)%size, 1, MPI_COMM_WORLD);
					mcounter++;//ii
				}
				terminated=true;
			}
		}
/*
nodeArray[rank]=active;		
MPI_Gather(&nodeArray[rank], 1, MPI_INT,&nodeArray, 1, MPI_INT,0, MPI_COMM_WORLD);
if(rank==0)
{
	for(int i=0;i<size;i++)
		cout<<nodeArray[i]<<" ";
	cout<<endl;
}
*/
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
		//cout<<"Max possible number of communicated messages equals to "<<(1.27*size*log2(size)+size)<<endl;
		cout<<size<<" "<<total_messages<<" "<<(1.27*size*log2(size)+size)<<" "<<time2<<endl;
	}
}
