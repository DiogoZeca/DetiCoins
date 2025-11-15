#ifndef AAD_SHA1_OPENCL
#define AAD_SHA1_OPENCL

#define CL_TARGET_OPENCL_VERSION 120

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "aad_data_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_program program;
  cl_kernel kernel;
  char device_name[256];
  char platform_name[256];
  size_t max_work_group_size;
  cl_ulong global_mem_size;
  cl_ulong local_mem_size;
  cl_uint compute_units;
} opencl_context_t;

__attribute__((unused))
static void list_opencl_devices(void)
{
  cl_uint num_platforms;
  cl_int err = clGetPlatformIDs(0, NULL, &num_platforms);
  
  if(err != CL_SUCCESS || num_platforms == 0)
  {
    fprintf(stderr, "No OpenCL platforms found\n");
    return;
  }
  
  cl_platform_id *platforms = (cl_platform_id *)malloc(num_platforms * sizeof(cl_platform_id));
  clGetPlatformIDs(num_platforms, platforms, NULL);
  
  printf("Available OpenCL platforms and devices:\n");
  
  for(cl_uint p = 0; p < num_platforms; p++)
  {
    char platform_name[256];
    clGetPlatformInfo(platforms[p], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
    printf("Platform %u: %s\n", p, platform_name);
    
    cl_uint num_devices;
    err = clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    
    if(err == CL_SUCCESS && num_devices > 0)
    {
      cl_device_id *devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
      clGetDeviceIDs(platforms[p], CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
      
      for(cl_uint d = 0; d < num_devices; d++)
      {
        char device_name[256];
        cl_ulong mem_size;
        cl_uint compute_units;
        
        clGetDeviceInfo(devices[d], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
        clGetDeviceInfo(devices[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
        clGetDeviceInfo(devices[d], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
        
        printf("  Device %u: %s (%.2f GB, %u CUs)\n", d, device_name, mem_size / (1024.0 * 1024.0 * 1024.0), compute_units);
      }
      
      free(devices);
    }
    else
    {
      printf("  No GPU devices\n");
    }
  }
  
  free(platforms);
}

__attribute__((unused))
static int initialize_opencl(opencl_context_t *ctx, int platform_id, int device_id)
{
  cl_int err;
  cl_uint num_platforms;
  
  err = clGetPlatformIDs(0, NULL, &num_platforms);
  if(err != CL_SUCCESS || num_platforms == 0)
  {
    fprintf(stderr, "No OpenCL platforms found\n");
    return -1;
  }
  
  cl_platform_id *platforms = (cl_platform_id *)malloc(num_platforms * sizeof(cl_platform_id));
  clGetPlatformIDs(num_platforms, platforms, NULL);
  
  if(platform_id < 0 || platform_id >= (int)num_platforms)
  {
    fprintf(stderr, "Invalid platform_id %d\n", platform_id);
    free(platforms);
    return -1;
  }
  
  ctx->platform = platforms[platform_id];
  clGetPlatformInfo(ctx->platform, CL_PLATFORM_NAME, sizeof(ctx->platform_name), ctx->platform_name, NULL);
  free(platforms);
  
  cl_uint num_devices;
  err = clGetDeviceIDs(ctx->platform, CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
  if(err != CL_SUCCESS || num_devices == 0)
  {
    fprintf(stderr, "No GPU devices found\n");
    return -1;
  }
  
  cl_device_id *devices = (cl_device_id *)malloc(num_devices * sizeof(cl_device_id));
  clGetDeviceIDs(ctx->platform, CL_DEVICE_TYPE_GPU, num_devices, devices, NULL);
  
  if(device_id < 0 || device_id >= (int)num_devices)
  {
    fprintf(stderr, "Invalid device_id %d\n", device_id);
    free(devices);
    return -1;
  }
  
  ctx->device = devices[device_id];
  free(devices);
  
  clGetDeviceInfo(ctx->device, CL_DEVICE_NAME, sizeof(ctx->device_name), ctx->device_name, NULL);
  clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(ctx->max_work_group_size), &ctx->max_work_group_size, NULL);
  clGetDeviceInfo(ctx->device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ctx->global_mem_size), &ctx->global_mem_size, NULL);
  clGetDeviceInfo(ctx->device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(ctx->local_mem_size), &ctx->local_mem_size, NULL);
  clGetDeviceInfo(ctx->device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(ctx->compute_units), &ctx->compute_units, NULL);
  
  printf("OpenCL: %s / %s\n", ctx->platform_name, ctx->device_name);
  
  ctx->context = clCreateContext(NULL, 1, &ctx->device, NULL, NULL, &err);
  if(err != CL_SUCCESS)
  {
    fprintf(stderr, "Error creating context\n");
    return -1;
  }
  
  cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
  ctx->queue = clCreateCommandQueue(ctx->context, ctx->device, props, &err);
  if(err != CL_SUCCESS)
  {
    fprintf(stderr, "Error creating queue\n");
    clReleaseContext(ctx->context);
    return -1;
  }
  
  return 0;
}

__attribute__((unused))
static int load_opencl_kernel(opencl_context_t *ctx, const char *source, const char *kernel_name, const char *build_options)
{
  cl_int err;
  size_t source_len = strlen(source);
  
  ctx->program = clCreateProgramWithSource(ctx->context, 1, &source, &source_len, &err);
  if(err != CL_SUCCESS)
  {
    fprintf(stderr, "Error creating program\n");
    return -1;
  }
  
  err = clBuildProgram(ctx->program, 1, &ctx->device, build_options, NULL, NULL);
  if(err != CL_SUCCESS)
  {
    size_t log_size;
    clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    char *log = (char *)malloc(log_size);
    clGetProgramBuildInfo(ctx->program, ctx->device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
    fprintf(stderr, "Build error:\n%s\n", log);
    free(log);
    clReleaseProgram(ctx->program);
    return -1;
  }
  
  ctx->kernel = clCreateKernel(ctx->program, kernel_name, &err);
  if(err != CL_SUCCESS)
  {
    fprintf(stderr, "Error creating kernel\n");
    clReleaseProgram(ctx->program);
    return -1;
  }
  
  return 0;
}

__attribute__((unused))
static void cleanup_opencl(opencl_context_t *ctx)
{
  if(ctx->kernel) clReleaseKernel(ctx->kernel);
  if(ctx->program) clReleaseProgram(ctx->program);
  if(ctx->queue) clReleaseCommandQueue(ctx->queue);
  if(ctx->context) clReleaseContext(ctx->context);
}

#endif
