#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda.h>
#include <omp.h>
#include <string.h>

#define cudaCheck(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

// CUDA kernel. Each thread takes care of one element of c
__global__ void first(float *c,float *d, int n)
{
    // Get our global thread ID
    int tid = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    __shared__ float temp[1024];
    if (tid < n)
      temp[threadIdx.x] = c[tid];
    else
      temp[threadIdx.x] = 0;
    for(int j=blockDim.x>>1;j>=1;j>>=1){
      __syncthreads();
      if (threadIdx.x < j) temp[threadIdx.x] += temp[threadIdx.x+j];
    }
    //float sum = 0;
    //for(int i=0; i<1024;i++) sum += tmp[i];
    if (threadIdx.x == 0) atomicAdd(d,temp[0]);
    //if (threadIdx.x == 0) d[blockIdx.x] = temp[0];
} 
 
int main( int argc, char* argv[] )
{
    // Size of vectors
    int n = 2000;
 
    //Host vector
    float *h_c;
 
    //Device output vector
    float *d_c;
    float *d_d;
        
    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);
 
    // Allocate memory on host
    h_c = (float*)malloc(bytes);

    for(int i=0;i<n;i++)
    h_c[i] = 1;
 
    // Allocate memory on GPU
    cudaCheck(cudaMalloc(&d_c, bytes));
    cudaCheck(cudaMalloc(&d_d, sizeof(float)));
    cudaCheck(cudaMemset(d_d,0.0, sizeof(float)));
    cudaCheck(cudaMemcpy(d_c,h_c,bytes, cudaMemcpyHostToDevice));
 
    // Copy host vectors to device
    int blockSize, gridSize;

    // Number of threads in each thread block
    blockSize = 1024;
 
    // Number of thread blocks in grid
    gridSize = (int)ceil((float)n/blockSize);
 
    // Execute the kernel
    double t1 = omp_get_wtime();
    first<<<gridSize, blockSize>>>(d_c, d_d, n);
 
    // Synchronize
    cudaCheck(cudaDeviceSynchronize());
    double elapsed = omp_get_wtime()-t1;
    printf("Time: %g\n", elapsed);

    // Copy array back to host
    cudaCheck(cudaMemcpy( h_c, d_d, sizeof(float), cudaMemcpyDeviceToHost));
    printf("Checksum: %f\n",h_c[0]);
    // Release device memory
    cudaFree(d_c);
 
    // Release host memory
    free(h_c);
 
    return 0;
}

