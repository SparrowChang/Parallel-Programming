typedef struct _RGB_t
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char align;
} RGB_t, *pRGB_t;

__kernel void vector_histogram(__global unsigned int *A, __global unsigned int *B, __global unsigned int *C, __global unsigned int *D)
{
    // Get the index of the current element
	__private unsigned int i = get_global_id(0);

    // Do the operation
	__private  pRGB_t pixel;
	pixel = &D[i];
	
	A[(i*256)+pixel->R]++;
    B[(i*256)+pixel->G]++;
    C[(i*256)+pixel->B]++;
}
