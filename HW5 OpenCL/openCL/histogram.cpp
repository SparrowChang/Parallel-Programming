#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <fstream>
#include <iostream>
#include <string>
#include <ios>
#include <stdint.h>
#include <cstdlib>
#include <string.h>

#include <stdio.h>                                                                                                                                               
#include <stdlib.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
using namespace std;

#define MAX_SOURCE_SIZE (0x100000)

typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char align;
} RGB;

typedef struct
{
    bool type;
    unsigned int size;
    unsigned int height;
    unsigned int weight;
    RGB *data;
} Image;

void OpenClHistogram(Image *img
					,unsigned int u32R[256]
					,unsigned int u32G[256]
					,unsigned int u32B[256])
{
    // Create the two input vectors
    unsigned int i;
    unsigned int RGB_SIZE = (unsigned int)256;
    unsigned int LIST_SIZE = (unsigned int)img->size;
    unsigned int THREAD_SIZE = 1024;
    unsigned int BUFFER_SIZE = RGB_SIZE*THREAD_SIZE;
    unsigned int LOOP_CNT = LIST_SIZE/THREAD_SIZE;
    unsigned int REM_LOOP_SIZE = LIST_SIZE%THREAD_SIZE;
    unsigned int REM_BUFFER_SIZE = REM_LOOP_SIZE*RGB_SIZE;
    unsigned int *A = (unsigned int*)malloc(sizeof(unsigned int)*BUFFER_SIZE);
    unsigned int *B = (unsigned int*)malloc(sizeof(unsigned int)*BUFFER_SIZE);
    unsigned int *C = (unsigned int*)malloc(sizeof(unsigned int)*BUFFER_SIZE);
 	
	// Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

	for( i=0 ; i<RGB_SIZE ; i++)
	{
		u32R[i] = 0x00;
		u32G[i] = 0x00;
		u32B[i] = 0x00;
	}
	
    fp = fopen("./histogram.cl", "r");
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL; 
    cl_int ret = clGetPlatformIDs(1, &platform_id, NULL);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 1, 
            &device_id, NULL);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	
    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, 
            BUFFER_SIZE * sizeof(unsigned int), NULL, &ret);
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE,
            BUFFER_SIZE * sizeof(unsigned int), NULL, &ret);
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, 
            BUFFER_SIZE * sizeof(unsigned int), NULL, &ret);
    cl_mem d_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            THREAD_SIZE * sizeof(unsigned int), NULL, &ret);	
	
    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);
	
    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_histogram", &ret);
	
    // Set the arguments of the kernel
	ret = 0;
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
    ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&d_mem_obj); 
 
	size_t global_item_size = THREAD_SIZE; // Process the entire lists
	
	for(unsigned int ii=0 ;ii<LOOP_CNT ; ii++)
	{
		for( i=0 ; i<BUFFER_SIZE ; i++)
		{
			A[i] = 0x00;
		}	
		// Copy the lists A and raw data to their respective memory buffers
		ret = 0;
		ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
				BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret |= clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
				BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret |= clEnqueueWriteBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
				BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret = clEnqueueWriteBuffer(command_queue, d_mem_obj, CL_TRUE, 0, 
				THREAD_SIZE * sizeof(unsigned int), &img->data[ii*THREAD_SIZE], 0, NULL, NULL);
		
		// Execute the OpenCL kernel on the list
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
				&global_item_size, 0, 0, NULL, NULL);
		
		// Read the memory buffer C on the device to the local variable C
		ret = 0;
		ret = clEnqueueReadBuffer(command_queue, a_mem_obj, CL_TRUE, 0, 
				BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);
		ret |= clEnqueueReadBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
				BUFFER_SIZE * sizeof(unsigned int), B, 0, NULL, NULL);
		ret |= clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
				BUFFER_SIZE * sizeof(unsigned int), C, 0, NULL, NULL);
			
		// Update the result to the screen
		for( i=0 ; i<THREAD_SIZE ; i++)
		{
			for( unsigned int j=0 ; j<RGB_SIZE ; j++)
			{
				if( A[j+(i*RGB_SIZE)]!=0 )
				{
					u32R[j]++;
				}
				if( B[j+(i*RGB_SIZE)]!=0 )
				{
					u32G[j]++;
				}
				if( C[j+(i*RGB_SIZE)]!=0 )
				{
					u32B[j]++;
				}
			}
		}
	}

	if( REM_LOOP_SIZE>0 )
	{
		for( i=0 ; i<BUFFER_SIZE ; i++)
		{
			A[i] = 0x00;
		}	
		// Copy the lists A and raw data to their respective memory buffers
		ret = 0;
		ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
				REM_BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret |= clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
				REM_BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret |= clEnqueueWriteBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
				REM_BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);			
		ret = clEnqueueWriteBuffer(command_queue, d_mem_obj, CL_TRUE, 0, 
				REM_LOOP_SIZE * sizeof(unsigned int), &img->data[LOOP_CNT*THREAD_SIZE], 0, NULL, NULL);
		
		global_item_size = REM_LOOP_SIZE;
		// Execute the OpenCL kernel on the list
		ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
				&global_item_size, 0, 0, NULL, NULL);
		
		// Read the memory buffer C on the device to the local variable C
		ret = 0;
		ret = clEnqueueReadBuffer(command_queue, a_mem_obj, CL_TRUE, 0, 
				REM_BUFFER_SIZE * sizeof(unsigned int), A, 0, NULL, NULL);
		ret |= clEnqueueReadBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
				REM_BUFFER_SIZE * sizeof(unsigned int), B, 0, NULL, NULL);
		ret |= clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
				REM_BUFFER_SIZE * sizeof(unsigned int), C, 0, NULL, NULL);
			
		// Update the result to the screen
		for( i=0 ; i<REM_LOOP_SIZE ; i++)
		{
			for( unsigned int j=0 ; j<RGB_SIZE ; j++)
			{
				if( A[j+(i*RGB_SIZE)]!=0 )
				{
					u32R[j]++;
				}
				if( B[j+(i*RGB_SIZE)]!=0 )
				{
					u32G[j]++;
				}
				if( C[j+(i*RGB_SIZE)]!=0 )
				{
					u32B[j]++;
				}
			}
		}		
	}
    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseMemObject(d_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(A);
    free(B);
    free(C);
}

Image *readbmp(const char *filename)
{
    ifstream bmp(filename, ios::binary);
    char header[54];
    bmp.read(header, 54);
    unsigned int size = *(int *)&header[2];
    unsigned int offset = *(int *)&header[10];
    unsigned int w = *(int *)&header[18];
    unsigned int h = *(int *)&header[22];
    unsigned int depth = *(ushort *)&header[28];
    if (depth != 24 && depth != 32)
    {
        printf("we don't suppot depth with %d\n", depth);
        exit(0);
    }
    bmp.seekg(offset, bmp.beg);

    Image *ret = new Image();
    ret->type = 1;
    ret->height = h;
    ret->weight = w;
    ret->size = w * h;
    ret->data = new RGB[w * h];
    for (int i = 0; i < ret->size; i++)
    {
        bmp.read((char *)&ret->data[i], depth / 8);
    }
    return ret;
}

int writebmp(const char *filename, Image *img)
{

    char header[54] = {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        32, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    // file size
    unsigned int file_size = img->size * 4 + 54;
    header[2] = (char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    unsigned int width = img->weight;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    unsigned int height = img->height;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    ofstream fout;
    fout.open(filename, ios::binary);
    fout.write((char *)header, 54);
    fout.write((char *)img->data, img->size * 4);
    fout.close();
}

void histogram(Image *img,unsigned int R[256],unsigned int G[256],unsigned int B[256]){
    fill(R, R+256, 0);
    fill(G, G+256, 0);
    fill(B, B+256, 0);

    for (int i = 0; i < img->size; i++){
        RGB &pixel = img->data[i];
        R[pixel.R]++;
        G[pixel.G]++;
        B[pixel.B]++;
    }
}

int main(int argc, char *argv[])
{
    char *filename;
    if (argc >= 2)
    {
        int many_img = argc - 1;
        for (int i = 0; i < many_img; i++)
        {
			unsigned int R[256];
            unsigned int G[256];
            unsigned int B[256];
            filename = argv[i + 1];
            readbmp(filename);
            Image *img = readbmp(filename);

            cout << img->weight << ":" << img->height << "\n";
			
			OpenClHistogram(img,R,G,B);
            
			int max = 0;
            for(int i=0;i<256;i++){
                max = R[i] > max ? R[i] : max;
                max = G[i] > max ? G[i] : max;
                max = B[i] > max ? B[i] : max;
            }

			if( max>0 )
			{
				Image *ret = new Image();
				ret->type = 1;
				ret->height = 256;
				ret->weight = 256;
				ret->size = 256 * 256;
				ret->data = new RGB[256 * 256]{};
	
				for(int i=0;i<ret->height;i++){
					for(int j=0;j<256;j++){
						if(R[j]*256/max > i)
							ret->data[256*i+j].R = 255;
						if(G[j]*256/max > i)
							ret->data[256*i+j].G = 255;
						if(B[j]*256/max > i)
							ret->data[256*i+j].B = 255;
					}
				}
	
				string newfile = "hist_" + string(filename); 
				writebmp(newfile.c_str(), ret);	
			}
			else
			{
				printf("max<=0....\n");				
			}
        }
    }else{
        printf("Usage: ./hist <img.bmp> [img2.bmp ...]\n");
    }
    return 0;
}
