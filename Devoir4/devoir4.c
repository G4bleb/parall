// This program implements a vector addition using OpenCL
// System includes
#include <stdio.h>
#include <stdlib.h>
// OpenCL includes
#include <CL/cl.h>
#define PLATFORM_INDEX 0
//2800 54 secondes GPU
//4500 58 secondes CPU
// OpenCL kernel to perform an element-wise
// add of two arrays
const char *programSource_bak =
"__kernel																    \n"
"void floyd(__global int* mat,											    \n"
"		__global int* k)									        \n"
"{																		    \n"
"																		    \n"
"	int i = get_global_id(0);											    \n"
"	int n = get_global_size(0);											    \n"
"	int j;												        		    \n"
"	for (j = 0; j < n; j++) {											    \n"
"		mat[i * n + j] = (mat[i * n +j]) < (mat[i * n + *k] + mat[*k * n + j])  \n"
"			? (mat[i * n +j])												    \n"
"			: (mat[i * n + *k] + mat[*k * n + j]);						    \n"
"	}																	    \n"
"}																		    \n"
;

const char *programSource =
"__kernel																    \n"
"void floyd(__global int* mat,											    \n"
"		__global int* k)									        \n"
"{																		    \n"
"																		    \n"
"	int i = get_global_id(0);											    \n"
"	int n = get_global_size(0);											    \n"
"	int j = get_global_id(1);												        		    \n"
"   int item = mat[i*n+j];\n"
"   int compr = mat[i * n + *k] + mat[*k * n + j];\n"
"		mat[i * n + j] = item < compr ? item : compr;						    \n"
"}																		    \n"
;

int *initialize(int n) {
    int *mat = (int *)malloc(n * n * sizeof(int));
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i == j) {
                mat[i * n + j] = 0;
            }else if(i - j == 1 || j - i == 1){
                mat[i * n + j] = 1;
            }else if((i == 0 && j == n-1) || (j == 0 && i == n-1)){
                mat[i * n + j] = 1;
            }else{
                mat[i * n + j] = n+1;
            }
        }
    }
    return mat;
}

void display(int *mat, int n){
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", mat[i * n + j]);
        }
        printf("\n");
    }
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Usage : %s n\n", argv[0]);
        return 1;
    }
    // This code executes on the OpenCL host
    // Elements in each array
    int n = atoi(argv[1]);
    const int elements = n*n;
    // Compute the size of the data
    size_t datasize = sizeof(int) * elements;
    // Initialize the input data
    int *mat = initialize(n); // IN/OUT
    // display(mat, n);
    // Use this to check the output of each API call
    cl_int status;
    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    // Use clGetPlatformIDs() to retrieve the number of
    // platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    // Allocate enough space for each platform
    platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
    // Fill in platforms with clGetPlatformIDs()
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
    char Name[1000];
    clGetPlatformInfo(platforms[PLATFORM_INDEX], CL_PLATFORM_NAME, sizeof(Name), Name, NULL);
    printf("Name of platform : %s\n", Name);
    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //-----------------------------------------------------
    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;
    // Use clGetDeviceIDs() to retrieve the number of
    // devices present
    status =
        clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
    printf("Number of devices = %d\n", (int)numDevices);
    // Allocate enough space for each device
    devices = (cl_device_id *)malloc(numDevices * sizeof(cl_device_id));
    // Fill in devices with clGetDeviceIDs()
    status = clGetDeviceIDs(platforms[PLATFORM_INDEX], CL_DEVICE_TYPE_ALL, numDevices,
                            devices, NULL);
    for (int i = 0; i < numDevices; i++) {
        clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(Name), Name, NULL);
        printf("Name of device %d: %s\n", i, Name);
    }
    //-----------------------------------------------------
    // STEP 3: Create a context
    //-----------------------------------------------------
    cl_context context = NULL;
    // Create a context using clCreateContext() and
    // associate it with the devices
    context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //-----------------------------------------------------
    cl_command_queue cmdQueue;
    // Create a command queue using clCreateCommandQueue(),
    // and associate it with the device you want to execute
    // on
    // cmdQueue = clCreateCommandQueue(context, devices[0], 0, &status);
    cmdQueue = clCreateCommandQueueWithProperties(context, devices[0], 0, &status);

    //Create events
    cl_event inEvent, floydEvent, outEvent;

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //-----------------------------------------------------
    cl_mem bufferMat;
    cl_mem bufferK;
    bufferMat =
        clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
    bufferK =
        clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int), NULL, &status);
    
    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //-----------------------------------------------------
    // Create a program using clCreateProgramWithSource()
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&programSource, NULL, &status);
    // Build (compile) the program for the devices with
    // clBuildProgram()

    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);

    size_t len = 0;
    cl_int ret = CL_SUCCESS;
    ret = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0,
                                NULL, &len);
    char *buffer = calloc(len, sizeof(char));
    ret = clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, len,
                                buffer, NULL);
    printf("%s\n", buffer);

    if (status){
        printf("ERREUR A LA COMPILATION: %d\n", status);
        return 1;
    }
    //-----------------------------------------------------
    // STEP 8: Create the kernel
    //-----------------------------------------------------
    // Use clCreateKernel() to create a kernel
    cl_kernel kernelfloyd = clCreateKernel(program, "floyd", &status);

    //-----------------------------------------------------
    // STEP 9: Set the kernel arguments
    //-----------------------------------------------------
    // Associate the input and output buffers with the
    // kernel
    // using clSetKernelArg()
    status = clSetKernelArg(kernelfloyd, 0, sizeof(cl_mem), &bufferMat);
    status |= clSetKernelArg(kernelfloyd, 1, sizeof(cl_mem), &bufferK);
    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //-----------------------------------------------------
    // Define an index space (global work size) of work
    // items for
    // execution. A workgroup size (local work size) is not
    // required,
    // but can be used.
    size_t globalWorkSize[2];
    // There are 'elements' work-items
    globalWorkSize[0] = n;
    globalWorkSize[1] = n;
    //-----------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //-----------------------------------------------------
    // Execute the kernel by using
    // clEnqueueNDRangeKernel().
    // 'globalWorkSize' is the 1D dimension of the
    // work-items
    status = clEnqueueWriteBuffer(cmdQueue, bufferMat, CL_FALSE, 0, datasize,
                                  mat, 0, NULL, &inEvent);
    int k;
    for (k = 0; k < n; k++)
    {
        // printf("k = %d\n", k);
        status = clEnqueueWriteBuffer(cmdQueue, bufferK, CL_FALSE, 0,
                                      sizeof(int), &k, 0, NULL, &inEvent);
        status = clEnqueueNDRangeKernel(cmdQueue, kernelfloyd, 2, NULL,
                                        globalWorkSize, NULL, 1, &inEvent,
                                        &floydEvent);
        clWaitForEvents(1, &floydEvent);
    }

    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //-----------------------------------------------------
    // Use clEnqueueReadBuffer() to read the OpenCL output
    // buffer (bufferC)
    // to the host output array (C)
    clEnqueueReadBuffer(cmdQueue, bufferMat, CL_TRUE, 0, datasize, mat, 0,
                        NULL, &outEvent);
    clWaitForEvents(1, &outEvent);
    // display(mat, n);
    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //-----------------------------------------------------
    // Free OpenCL resources
    clReleaseEvent(inEvent);
    clReleaseEvent(floydEvent);
    clReleaseEvent(outEvent);
    clReleaseKernel(kernelfloyd);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferMat);
    clReleaseMemObject(bufferK);
    clReleaseContext(context);
    // Free host resources
    free(mat);
    free(platforms);
    free(devices);
}
