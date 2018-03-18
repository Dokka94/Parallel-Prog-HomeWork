#include <mpi.h>
#include <stdio.h>
#include <sstream>
/*
  I loaded up my matrices with integers 1-16, to represent a result. It was not necessary, but I thougth it will be more representative.
*/

int main (int argc, char *argv[]){
  int N = 4;
  int A[N*N] = {1,2,3,4,9,10,11,12,13,14,15,16,5,6,7,8};
  int B[N*N] ={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  int C[N*N] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  int p,rank, rc;
  rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS) {
    printf ("Error starting MPI porgram. Terminating. \n");
    MPI_Abort(MPI_COMM_WORLD,rc);
  }


  MPI_Comm_size(MPI_COMM_WORLD,&p);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Status status;
  printf("Number of tasks: %d, my rank is: %d \n",p,rank);
  float a[N/2*N/2],b[N/2*N/2];  
  int c,d,e,f,g,h; //for indexing
  //A[(i+c*N/2)*N+j+d*N/2]
  //B[(i+e*N/2)*N+j+f*N/2]
  //C[(i+g*N/2)*N+j+h*N/2]
  if (rank == 0){
    c=0;
    d=0;
    e=0;
    f=0;
    g=0;
    h=0;

    //print A & B  
    std::ostringstream ssA,ssB;  
    for(int i=0;i<N;i++){
      for(int j=0;j<N;j++){
        ssA << A[i*N+j] << " ";
        ssB << B[i*N+j] << " ";
      }
      ssA << "\n";
      ssB << "\n";
    }
    std::cout << "A: " << ssA.str() << '\n';
    std::cout << "B: " << ssB.str() << '\n';
  }
  else if (rank == 1){
    c=0;
    d=1;
    e=1;
    f=1;
    g=0;
    h=1;    
  }
  else if (rank == 2){
    c=1;
    d=1;
    e=1;
    f=0;
    g=1;
    h=0;    
  }
  else if (rank == 3){
    c=1;
    d=0;
    e=0;
    f=1;
    g=1;
    h=1;
  }
  
  //counting the mini-matrices a,b
  for(int i=0;i<N/2;i++){
    for(int j=0;j<N/2;j++){
      a[i*N/2+j] = A[(i+c*N/2)*N+j+d*N/2];
      b[i*N/2+j] = B[(i+e*N/2)*N+j+f*N/2];
    }
  }
  //counting C with the first a,b
  for(int i=0;i<N/2;i++){
    for(int j=0;j<N/2;j++){
      for(int k=0;k<N/2;k++){
        C[(i+g*N/2)*N+j+h*N/2] += a[i*N/2+k] * b[k*N/2+j];
      }
    }
  }
  
  if (rank == 0){
    //sending a to thread 1
    MPI_Send(&a,N/2*N/2,MPI_INT,1,22,MPI_COMM_WORLD);
    //receiving a from thread 1
    MPI_Recv(&a,N/2*N/2,MPI_INT,1,22,MPI_COMM_WORLD,&status);
    //sending b to thread 2
    MPI_Send(&b,N/2*N/2,MPI_INT,2,22,MPI_COMM_WORLD);
    //receiving b from thread 2
    MPI_Recv(&b,N/2*N/2,MPI_INT,2,22,MPI_COMM_WORLD,&status);
  }
  else if (rank == 1){
    //sending a to thread 0
    MPI_Send(&a,N/2*N/2,MPI_INT,0,22,MPI_COMM_WORLD);
    //receiving a from thread 0
    MPI_Recv(&a,N/2*N/2,MPI_INT,0,22,MPI_COMM_WORLD,&status);
    //sending b to thread 3
    MPI_Send(&b,N/2*N/2,MPI_INT,3,22,MPI_COMM_WORLD);
    //receiving b from thread 3
    MPI_Recv(&b,N/2*N/2,MPI_INT,3,22,MPI_COMM_WORLD,&status);
  }
  else if (rank == 2){
    //sending a to thread 3
    MPI_Send(&a,N/2*N/2,MPI_INT,3,22,MPI_COMM_WORLD);
    //receiving a from thread 3
    MPI_Recv(&a,N/2*N/2,MPI_INT,3,22,MPI_COMM_WORLD,&status);
    //sending b to thread 0
    MPI_Send(&b,N/2*N/2,MPI_INT,0,22,MPI_COMM_WORLD);
    //receiving b from thread 0
    MPI_Recv(&b,N/2*N/2,MPI_INT,0,22,MPI_COMM_WORLD,&status);
  }
  else if (rank == 3){
    //sending a to thread 2
    MPI_Send(&a,N/2*N/2,MPI_INT,2,22,MPI_COMM_WORLD);
    //receiving a from thread 2
    MPI_Recv(&a,N/2*N/2,MPI_INT,2,22,MPI_COMM_WORLD,&status);
    //sending b to thread 1
    MPI_Send(&b,N/2*N/2,MPI_INT,1,22,MPI_COMM_WORLD);
    //receiving b from thread 1
    MPI_Recv(&b,N/2*N/2,MPI_INT,1,22,MPI_COMM_WORLD,&status);
  }
  
  //counting C with the received a,b
  for(int i=0;i<N/2;i++){
    for(int j=0;j<N/2;j++){
      for(int k=0;k<N/2;k++){
        C[(i+g*N/2)*N+j+h*N/2] += a[i*N/2+k] * b[k*N/2+j];
      }
    }
  }
  //summary C
  if (rank == 0){
    for(int i=0;i<p-1;i++){
      int C_piece[N*N];
      MPI_Recv(&C_piece,N*N,MPI_INT,MPI_ANY_SOURCE,22,MPI_COMM_WORLD,&status);
      for(int j=0;j<N*N;j++){
        C[j] += C_piece[j];
      }
    }
    //print C
    std::ostringstream ss;
    for(int i=0;i<N;i++){
      for(int j=0;j<N;j++){
         ss << C[i*N+j] << " ";
      }
      ss << '\n';
    }
    std::cout << "C: " << ss.str() << '\n';
  }
  else{
    MPI_Send(&C,N*N,MPI_INT,0,22,MPI_COMM_WORLD);
  }
  MPI_Finalize();
  return 0;
}

