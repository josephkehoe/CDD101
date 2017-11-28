

/* Code: */

//
// saxpy.c
//
// A simple OpenCL program for computing saxpy operation
// y <- a x + y, x,y in R^n, a in R
// on single precision
// vectors. Inspired by the Apple OpenCL sample code available at
//
// http://developer.apple.com/mac/library/samplecode/OpenCL_Hello_World_Example/Introduction/Intro.html
//
// Tested on iMac core i7, ATI Radeon HD 4850, Mac OS X 10.6.3
//           PS3, IBM OpenCL Development Kit for Linux on Power, Fedora 12
// Tuomo Rossi, spring 2010
// users.jyu.fi/~tro/TIEA342/OpenCL/saxpy.c
////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef APPLE
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

// Use a static data size for simplicity

#define DATA_SIZE (1024)
#define SEPARATOR       ("----------------------------------------------------------------------\n")

// Simple kernel for computing the saxpy operation with real single precision
// vectors

const char *KernelSource = "\n" \
"__kernel void saxpy(                                                   \n" \
"   const unsigned int n,                                               \n" \
"   const float a,                                                      \n" \
"   __global float* x,                                                  \n" \
"   __global float* y)                                                  \n" \
"{                                                                      \n" \
"   int i = get_global_id(0);                                           \n" \
"   if(i < n)                                                           \n" \
"       y[i] = a * x[i] + y[i];                                         \n" \
"}                                                                      \n" \
"\n";

// Prototype for device reporting function

cl_int report_and_mark_devices(cl_device_id *, cl_uint, int *, int *, int *);

int main(int argc, char** argv)
{
  cl_int err;                         // error code returned from api calls
  
  float x[DATA_SIZE];                 // original data vector x given to device
  float y[DATA_SIZE];                 // original data vector y given to device
  float a;
  float results[DATA_SIZE];           // results returned from device
  unsigned int correct;               // number of correct results returned
  cl_device_id device_id;             // compute device id 
  int gpu = 1;
  
  if (argc == 1) {
    printf("%s -trying to use CL_DEVICE_GPU\n",argv[0]);
  } else if (argc == 2) {
    gpu = atoi(argv[1]);
    if (gpu != 0 && gpu != 1) {
      printf("Usage: %s 0 -use CL_DEVICE_CPU\n",argv[0]);
      printf("Usage: %s 1 -use CL_DEVICE_GPU or CL_DEVICE_ACCELERATOR\n",argv[0]); 
      return EXIT_FAILURE;
    }
    printf("%s -trying to use %s\n",argv[0], gpu ? "CL_DEVICE_GPU or CL_DEVICE_ACCELERATOR" :
	   "CL_DEVICE_CPU");
  }
  
  // Fill our data set with random float values
  
  int i = 0;
  a = 2.0;
  unsigned int count = DATA_SIZE;
  for(i = 0; i < count; i++) {
    x[i] = rand() / (float)RAND_MAX;
    y[i] = rand() / (float)RAND_MAX;
  }
  
  // Trying to get a handle to at least one OpenCL platform using function
  //
  // cl_int clGetPlatformIDs (cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
  //
  // num_entries is the number of cl_platform_id entries that can be added to platforms. If platforms
  // is not NULL, the num_entries must be greater than zero.
  // platforms returns a list of OpenCL platforms found. The cl_platform_id values returned in platforms
  // can be used to identify a specific OpenCL platform. If platforms argument is NULL, this argument is ignored.
  // The number of OpenCL platforms returned is the minimum of the value specified by num_entries or the number of
  // OpenCL platforms available.
  // num_platforms returns the number of OpenCL platforms available. If num_platforms is NULL, this argument is ignored.
  //
  // Trying to identify one platform:
  
  cl_platform_id platform;
  cl_uint num_platforms;
  err = clGetPlatformIDs(1,&platform,&num_platforms);

  if (err != CL_SUCCESS) {
    printf("Error: Failed to get a platform id!\n");
    return EXIT_FAILURE;
  }
  
  // Found one platform. Query specific information about the found platform using the function 
  //
  // cl_int clGetPlatformInfo (cl_platform_id platform, cl_platform_info param_name,
  //                           size_t param_value_size, void *param_value, 
  //                           size_t *param_value_size_ret)
  //
  // platform refers to the platform ID returned by clGetPlatformIDs or can be NULL.
  // If platform is NULL, the behavior is implementation-defined.
  //
  // param_name is an enumeration constant that identifies the platform information being queried.
  // We'll query the following information (for complete documentation, see Specification, page 30):
  //
  // CL_PLATFORM_NAME       -platform name string
  // CL_PLATFORM_VERSION    -OpenCL version supported by the implementation
  // CL_PLATFORM_PROFILE    -FULL_PROFILE if the implementation supports the OpenCL specification
  //                        -EMBEDDED_PROFILE - if the implementation supports the OpenCL embedded profile (subset).
  // CL_PLATFORM_EXTENSIONS -extension names supported by the platform
  //
  // param_value is a pointer to memory location where appropriate values for a given param_name will be returned.
  // If param_value is NULL, it is ignored.
  //
  // param_value_size specifies the size in bytes of memory pointed to by param_value.
  // param_value_size_ret returns the actual size in bytes of data being queried by param_value.
  //
  // Trying to query platform specific information...
  
  size_t returned_size = 0;
  cl_char platform_name[1024] = {0}, platform_prof[1024] = {0}, platform_vers[1024] = {0}, platform_exts[1024] = {0};
  err  = clGetPlatformInfo(platform, CL_PLATFORM_NAME,       sizeof(platform_name), platform_name, &returned_size);
  err |= clGetPlatformInfo(platform, CL_PLATFORM_VERSION,    sizeof(platform_vers), platform_vers, &returned_size);
  err |= clGetPlatformInfo(platform, CL_PLATFORM_PROFILE,    sizeof(platform_prof), platform_prof, &returned_size);
  err |= clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, sizeof(platform_exts), platform_exts, &returned_size);
  
  if (err != CL_SUCCESS) {
    printf("Error: Failed to get platform infor!\n");
    return EXIT_FAILURE;
  }
  
  printf("\nPlatform information\n");
  printf(SEPARATOR);
  printf("Platform name:       %s\n", (char *)platform_name);
  printf("Platform version:    %s\n", (char *)platform_vers);
  printf("Platform profile:    %s\n", (char *)platform_prof);
  printf("Platform extensions: %s\n", ((char)platform_exts[0] != '\0') ? (char *)platform_exts : "NONE");
  
  // Getting a list of available compute devices on our platform by function
  //
  // cl_int clGetDeviceIDs (cl_platform_id platform, cl_device_type device_type,
  //                        cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices)
  //
  // platform refers to the platform ID returned by clGetPlatformIDs
  //
  // device_type is a bitfield that identifies the type of OpenCL device. The device_type can
  // be used to query specific OpenCL devices or all OpenCL devices available. The valid values are
  //
  // CL_DEVICE_TYPE_CPU          An OpenCL device that is the host processor. The host processor runs the OpenCL
  //                             implementations and is a single or multi-core CPU.
  //
  // CL_DEVICE_TYPE_GPU          An OpenCL device that is a GPU. By this we mean that the device can also be used
  //                             to accelerate a 3D API such as OpenGL or DirectX.
  //
  // CL_DEVICE_TYPE_ACCELERATOR  Dedicated OpenCL accelerators (for example the IBM CELL Blade).
  //                             These devices communicate with the host processor using a peripheral interconnect such as PCIe.
  //
  // CL_DEVICE_TYPE_DEFAULT      The default OpenCL device in the system.
  //
  // CL_DEVICE_TYPE_ALL          All OpenCL devices available in the system.
  //
  // num_entries is the number of cl_device entries that can be added to devices. If devices is not
  // NULL, the num_entries must be greater than zero.
  //
  // devices returns a list of OpenCL devices found. The cl_device_id values returned in devices can be used
  // to identify a specific OpenCL device. If devices argument is NULL, this argument is ignored. The number of
  // OpenCL devices returned is the mininum of the value specified by num_entries or the number of OpenCL devices
  // whose type matches device_type.
  //
  // num_devices returns the number of OpenCL devices available that match device_type. If num_devices is NULL,
  // this argument is ignored.
  //
  // Get all available devices (up to 4)
  
  cl_uint num_devices;
  cl_device_id devices[4];
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 4, devices, &num_devices);

  if (err != CL_SUCCESS) {
    printf("Failed to collect device list on this platform!\n");
    return EXIT_FAILURE;
  }
  
  printf(SEPARATOR);
  printf("\nFound %d compute devices!:\n",num_devices);
  
  // Collect and report device information, return indices of devices of type CL_DEVICE_TYPE_CPU,
  // CL_DEVICE_TYPE_GPU and CL_DEVICE_TYPE_ACCELERATOR in array devices
  
  int a_cpu=-1, a_gpu=-1, an_accelerator=-1;
  err = report_and_mark_devices(devices,num_devices,&a_cpu,&a_gpu,&an_accelerator);

  if (err != CL_SUCCESS) {
    printf("Failed to report information about the devices on this platform!\n");
    return EXIT_FAILURE;
  }
  
  // Checking for availability of the required device
  
  if (gpu == 0) {     // No accelerator or gpu, just cpu
    if (a_cpu == -1) {
      printf("No cpus available, weird...\n");
      return EXIT_FAILURE;
    }
    device_id = devices[a_cpu];
    printf("There is a cpu, using it\n");
  }
  else {              // Trying to find a gpu, or if that fails, an accelerator 
    if (a_gpu != -1) { // There is a gpu in our platform
      device_id = devices[a_gpu];
      printf("Found a gpu, using it\n");
    } else if (an_accelerator != -1) {
      device_id = devices[an_accelerator];
      printf("No gpu but found an accelerator, using it\n");
    } else {
      printf("No cpu, no gpu, nor an accelerator... where am I running???\n");
      return EXIT_FAILURE;
    }
  }
  
  // We have a compute device of required type! Next, create a compute context on it.
  // The function 
  // 
  // cl_context clCreateContext (const cl_context_properties *properties, cl_uint num_devices,
  //                             const cl_device_id *devices, void (*pfn_notify)(const char *errinfo,
  //                                                                             const void *private_info, size_t cb,
  //                                                                             void *user_data),
  //                             void *user_data, cl_int *errcode_ret)
  //
  // creates an OpenCL context. An OpenCL context is created with one or more devices. Contexts are used
  // by the OpenCL runtime for managing objects such as command-queues, memory, program and kernel objects and for
  // executing kernels on one or more devices specified in the context.
  //
  // properties specifies a list of context property names and their corresponding values. properties can be NULL in which
  // case the platform that is selected is implementation-defined.
  //
  // num_devices is the number of devices specified in the devices argument.
  //
  // devices is a pointer to a list of unique devices5 returned by clGetDeviceIDs for a platform.
  // 
  // pfn_notify is a callback function that can be registered by the application. This callback function will be used
  // by the OpenCL implementation to report information on errors that occur in this context. If pfn_notify is NULL,
  // no callback function is registered.
  //
  // user_data will be passed as the user_data argument when pfn_notify is called. user_data can be NULL.
  //
  // errcode_ret will return an appropriate error code. If errcode_ret is NULL, no error code is returned.
  //
  // clCreateContext returns a valid non-zero context and errcode_ret is set to CL_SUCCESS if the context is created successfully.
  // Otherwise, it returns a NULL value with an error valuee returned in errcode_ret.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nCreating a compute context for the required device\n");
  
  cl_context context;                 // compute context
  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);

  if (!context) {
    printf("Error: Failed to create a compute context!\n");
    return EXIT_FAILURE;
  }
  
  // OpenCL objects such as memory, program and kernel objects are created using a context. Operations on these
  // objects are performed using a command-queue. The command-queue can be used to queue a set of operations (referred
  // to as commands) in order. Having multiple command-queues allows applications to queue multiple independent commands
  // without requiring synchronization. Note that this should work as long as these objects are not being shared.
  // Sharing of objects across multiple command-queues will require the application to perform appropriate synchronization.
  // 
  // The function
  //
  // cl_command_queue clCreateCommandQueue (cl_context context, cl_device_id device,
  //                                        cl_command_queue_properties properties, cl_int *errcode_ret)
  //
  // creates a command-queue on a specific device. context must be a valid OpenCL context.
  //
  // device must be a device associated with context. It can either be in the list of devices specified when context
  // is created using clCreateContext or have the same device type as the device type specified when the context is
  // created using clCreateContextFromType.
  //
  // properties is a bit-field which specifies a list of properties for the command-queue, we use the default one (0)
  //
  // errcode_ret will return an appropriate error code. If errcode_ret is NULL, no error code is returned.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nCreating a command queue\n");
  
  cl_command_queue commands;
  commands = clCreateCommandQueue(context, device_id, 0, &err);

  if (!commands) {
    printf("Error: Failed to create a command queue!\n");
    return EXIT_FAILURE;
  }
  
  // Create the compute program object for our context and load the source code from the source buffer
  //
  // The function
  //
  // cl_program clCreateProgramWithSource (cl_context context, cl_uint count,
  //                                       const char **strings, const size_t *lengths, cl_int *errcode_ret)
  //
  // creates a program object for a context, and loads the source code specified by the text strings in the
  // strings array into the program object. The devices associated with the program object are the devices associated with context.
  //
  // context must be a valid OpenCL context.
  //
  // strings is an array of count pointers to optionally null-terminated character strings that make up
  // the source code.
  //
  // The lengths argument is an array with the number of chars in each string (the string length). If an element
  // in lengths is zero, its accompanying string is null-terminated. If lengths is NULL, all strings in the strings
  // argument are considered null-terminated. Any length value passed in that is greater than zero excludes the
  // null terminator in its count.
  // 
  // errcode_ret will return an appropriate error code. If errcode_ret is NULL, no error code is returned.

  printf("\n");
  printf(SEPARATOR);
  printf("\nCreating the compute program from source\n");
  printf("%s\n", KernelSource);

  cl_program program;                 // compute program
  program = clCreateProgramWithSource(context, 1, (const char **) &KernelSource, NULL, &err);

  if (!program) {
    printf("Error: Failed to create compute program!\n");
    return EXIT_FAILURE;
  }
  
  // Build the program executable
  //
  // The function
  //
  // cl_int clBuildProgram (cl_program program, cl_uint num_devices,
  //                        const cl_device_id *device_list, const char *options,
  //                        void (*pfn_notify)(cl_program, void *user_data), void *user_data)
  //
  // builds (compiles & links) a program executable from the program source or binary for all the devices or
  // a specific device(s) in the OpenCL context associated with program. OpenCL allows program executables to be
  // built using the source or the binary. clBuildProgram must be called for program created using either
  // clCreateProgramWithSource or clCreateProgramWithBinary to build the program executable for one or more
  // devices associated with program.
  //
  // program is the program object we just created.
  //
  // device_list is a pointer to a list of devices associated with program. If device_list is a NULL value,
  // the program executable is built for all devices associated with program for which a source or binary has been loaded.
  // If device_list is a non-NULL value, the program executable is built for devices specified in this list for which
  // a source or binary has been loaded.
  //
  // num_devices is the number of devices listed in device_list.
  // 
  // options is a pointer to a string that describes the build options to be used for building the program executable
  // (see section 5.4.3)
  //
  // pfn_notify is a function pointer to a notification routine. The notification routine is a callback function that
  // an application can register and which will be called when the program executable has been built (successfully or unsuccessfully).
  //
  // user_data will be passed as an argument when pfn_notify is called. user_data can be NULL.

  printf(SEPARATOR);
  printf("\nCompiling the program executable\n");
  
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];
    printf("Error: Failed to build program executable!\n");
    
    // See page 98...
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    printf("%s\n", buffer);
    exit(1);
  }
  
  // Create the compute kernel object in the program we wish to run
  //
  // To create a kernel object, use the function
  //
  // cl_kernel clCreateKernel (cl_program program, const char *kernel_name,
  //                           cl_int *errcode_ret)
  //
  // program is a program object with a successfully built executable.
  //
  // kernel_name is a function name in the program declared with the __kernel qualifier (here "saxpy").
  // 
  // errcode_ret will return an appropriate error code. If errcode_ret is NULL, no error code is returned.
  //
  // clCreateKernel returns a valid non-zero kernel object and errcode_ret is set to CL_SUCCESS if the
  // kernel object is created successfully.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nCreating the compute kernel from program executable\n");
  
  cl_kernel kernel;                   // compute kernel
  kernel = clCreateKernel(program, "saxpy", &err);
  
  if (!kernel || err != CL_SUCCESS) {
    printf("Error: Failed to create compute kernel!\n");
    exit(1);
  }
  
  // Create buffer objects for the input and input/output arrays in device memory for our calculation
  //
  // A buffer object is created using the following function
  //
  // cl_mem clCreateBuffer (cl_context context, cl_mem_flags flags,
  //                        size_t size, void *host_ptr, cl_int *errcode_ret)
  //
  // context is a valid OpenCL context used to create the buffer object.
  //
  // flags is a bit-field that is used to specify allocation and usage information such as the memory
  // arena that should be used to allocate the buffer object and how it will be used.
  //
  // Usually, these are used:
  //
  // CL_MEM_READ_WRITE   -This flag specifies that the memory object will be read and written by a kernel. This is the default.
  //
  // CL_MEM_WRITE_ONLY   -This flags specifies that the memory object will be written but not read by a kernel.
  //                      Reading from a buffer or image object created with CL_MEM_WRITE_ONLY inside a kernel is undefined.
  //
  // CL_MEM_READ_ONLY    -This flag specifies that the memory object is a read-only memory object when used inside a kernel.
  //                      Writing to a buffer or image object created with CL_MEM_READ_ONLY inside a kernel is undefined.
  //
  // CL_MEM_USE_HOST_PTR -This flag is valid only if host_ptr is not NULL. If specified, it indicates that the application
  //                      wants the OpenCL implementation to use memory referenced by host_ptr as the storage bits for
  //                      the memory object.
  //
  // size is the size in bytes of the buffer memory object to be allocated.
  //
  // host_ptr is a pointer to the buffer data that may already be allocated by the application. The size of the buffer
  // that host_ptr points to must be >= size bytes.
  //
  // errcode_ret will return an appropriate error code. If errcode_ret is NULL, no error code is returned.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nCreating the input and input/output arrays in device memory\n");
  
  cl_mem dx;                       // device memory used for the input array x
  cl_mem dy;                       // device memory used for the input/output array y
  dx = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * count, NULL, NULL);
  dy = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * count, NULL, NULL);
  
  if (!dx || !dy) {
    printf("Error: Failed to allocate device memory!\n");
    exit(1);
  }    
    
  // Write our data set into the input buffer object in device memory 
  //
  // cl_int clEnqueueWriteBuffer (cl_command_queue command_queue, cl_mem buffer,
  //                              cl_bool blocking_write, size_t offset, size_t cb, const void *ptr,
  //                              cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event)
  //
  // command_queue refers to the command-queue in which the read / write command will be queued.
  // command_queue and buffer must be created with the same OpenCL context.
  //
  // buffer refers to a valid buffer object.
  //
  // blocking_write indicates if the write operations are blocking or non-blocking.
  // If blocking_write is CL_TRUE, the OpenCL implementation copies the data referred to by ptr and enqueues the write
  // operation in the command-queue. The memory pointed to by ptr can be reused by the application after the
  // clEnqueueWriteBuffer call returns.
  //
  // If blocking_write is CL_FALSE, the OpenCL implementation will use ptr to perform a non- blocking write.
  // As the write is non-blocking the implementation can return immediately. The memory pointed to by ptr cannot be
  // reused by the application after the call returns. The event argument returns an event object which can be used
  // to query the execution status of the write command. When the write command has completed, the memory pointed
  // to by ptr can then be reused by the application.
  //
  // offset is the offset in bytes in the buffer object to read from or write to.
  //
  // cb is the size in bytes of data being written.
  //
  // ptr is the pointer to buffer in host memory where data is to be written from.
  //
  // event_wait_list and num_events_in_wait_list specify events that need to complete before this particular command
  // can be executed. If event_wait_list is NULL, then this particular command does not wait on any event to complete.
  //
  // event returns an event object that identifies this particular write command and can be used to query or queue
  // a wait for this particular command to complete. event can be NULL in which case it will not be possible for the
  // application to query the status of this command or queue a wait for this command to complete.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nWriting the data into the input and input/output arrays in device memory\n");
  
  err  = clEnqueueWriteBuffer(commands, dx, CL_TRUE, 0, sizeof(float) * count, x, 0, NULL, NULL);
  err |= clEnqueueWriteBuffer(commands, dy, CL_TRUE, 0, sizeof(float) * count, y, 0, NULL, NULL);

  if (err != CL_SUCCESS) {
    printf("Error: Failed to write to source array!\n");
    exit(1);
  }
  
  // Setting the arguments to our compute kernel in order to execute it.
  //
  // The function
  //
  // cl_int clSetKernelArg (cl_kernel kernel, cl_uint arg_index,
  //                        size_t arg_size, const void *arg_value)
  //
  // is used to set the argument value for a specific argument of a kernel.
  //
  // kernel is a valid kernel object.
  //
  // arg_index is the argument index. Arguments to the kernel are referred by indices that go from 0 for the leftmost
  // argument to n - 1, where n is the total number of arguments declared by a kernel.
  // 
  // For example, our kernel has the arguments
  // __kernel void saxpy(                                                        
  // 	const unsigned int n,                                               
  //	const float a,                                                      
  //	__global float* x,                                                  
  //	__global float* y)                                                  
  // {
  //  ...
  // }
  //
  // Argument index values for saxpy will be 0 for n, 1 for a, 2 for x and 3 for y.
  //
  // arg_value is a pointer to data that should be used as the argument value for argument specified by arg_index.
  // The argument data pointed to by arg_value is copied and the arg_value pointer can therefore be reused by the
  // application after clSetKernelArg returns.
  //
  // arg_size specifies the size of the argument value. Here it is sizeof(cl_mem) for memory buffer objects and
  // sizeof(unsigned int) for count argument and sizeof(float) for the real constant.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nSetting the kernel arguments\n");
  
  err  = 0;
  err  = clSetKernelArg(kernel, 0, sizeof(unsigned int), &count);
  err |= clSetKernelArg(kernel, 1, sizeof(float),        &a);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem),       &dx);
  err |= clSetKernelArg(kernel, 3, sizeof(cl_mem),       &dy);
  
  if (err != CL_SUCCESS) {
    printf("Error: Failed to set kernel arguments! %d\n", err);
    exit(1);
  }
  
  // Get the maximum kernel work group size for executing the kernel on the device
  //
  // The function
  //
  // cl_int clGetKernelWorkGroupInfo (cl_kernel kernel, cl_device_id device,
  //                                  cl_kernel_work_group_info param_name, size_t param_value_size,
  //                                  void *param_value, size_t *param_value_size_ret)
  //
  // returns information about the kernel object that may be specific to a device.
  //
  // kernel specifies the kernel object being queried.
  //
  // device identifies a specific device in the list of devices associated with kernel. The list of devices is
  // the list of devices in the OpenCL context that is associated with kernel. If the list of devices associated
  // with kernel is a single device, device can be a NULL value.
  //
  // param_name specifies the information to query. We use (for others, see page 107):
  //
  // CL_KERNEL_WORK_GROUP_SIZE  -This provides a mechanism for the application to query the maximum work-group size
  //                             that can be used to execute a kernel on a specific device given by device. The OpenCL
  //                             implementation uses the resource requirements of the kernel (register usage etc.)
  //                             to determine what this work-group size should be.
  //
  // param_value is a pointer to memory where the appropriate result being queried is returned.
  // If param_value is NULL, it is ignored.
  //
  // param_value_size is used to specify the size in bytes of memory pointed to by param_value.
  // This size must be >= size of return type
  // 
  // param_value_size_ret returns the actual size in bytes of data copied to param_value.
  // If param_value_size_ret is NULL, it is ignored.
  
  size_t local;                       // local domain size for our calculation
  err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
  
  if (err != CL_SUCCESS) {
    printf("Error: Failed to retrieve kernel work group info! %d\n", err);
    exit(1);
  }

  // Execute the kernel over the entire range of our logically 1d configuration
  // using the maximum kernel work group size
  //
  // The function
  //
  // cl_int clEnqueueNDRangeKernel (cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim,
  //                                const size_t *global_work_offset, const size_t *global_work_size,
  //                                const size_t *local_work_size, cl_uint num_events_in_wait_list,
  //                                const cl_event *event_wait_list, cl_event *event)
  //
  // enqueues a command to execute a kernel on a device.
  //
  // command_queue is a valid command-queue. The kernel will be queued for execution on the
  // device associated with command_queue.
  //
  // kernel is a valid kernel object. The OpenCL context associated with kernel and command-queue
  // must be the same.
  //
  // work_dim is the number of dimensions used to specify the global work-items and work-items in
  // the work-group. work_dim must be greater than zero and less than or equal to three.
  //
  // global_work_offset must currently be a NULL value.
  //
  // global_work_size points to an array of work_dim unsigned values that describe the number of global work-items
  // in work_dim dimensions that will execute the kernel function. The total number of global work-items is computed as
  // global_work_size[0]*...*global_work_size[work_dim-1].
  //
  // local_work_size points to an array of work_dim unsigned values that describe the number of work-items that make
  // up a work-group (also referred to as the size of the work-group) that will execute the kernel specified by kernel.
  // The total number of work-items in the work-group must be less than or equal to the CL_DEVICE_MAX_WORK_GROUP_SIZE
  //
  // event_wait_list and num_events_in_wait_list specify events that need to complete before this particular
  // command can be executed.
  //
  // event returns an event object that identifies this particular kernel execution instance.
  
  size_t global;                      // global domain size for our calculation
  global = count;
  printf("\n");
  printf(SEPARATOR);
  printf("\nExecuting the kernel over 1d range %d.\n", (int)global);
  printf("Maximum work group size for this kernel is %d on this device\n", (int)local);
  
  err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  
  if (err) {
    printf("Error: Failed to execute kernel!\n");
    return EXIT_FAILURE;
  }
  
  // Wait for the command commands to get serviced before reading back results
  
  clFinish(commands);
  
  // Read back the results from the device to verify the output. In order to fully mimic
  // the saxpy operation, the result should be read into the array y, we read it to array results
  // in order to verify the computations.
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nReading back the results from the device\n");
  
  err = clEnqueueReadBuffer( commands, dy, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL );  

  if (err != CL_SUCCESS) {
    printf("Error: Failed to read output array! %d\n", err);
    exit(1);
  }
  
  // Validate our results
  
  correct = 0;
  for(i = 0; i < count; i++) {
    if(results[i] == a*x[i] + y[i])
      correct++;
  }
    
  // Print a brief summary detailing the results
  
  printf("\n");
  printf(SEPARATOR);
  printf("\nComputed '%d/%d' correct values!\n", correct, count);
  
  // Shutdown and cleanup
  
  clReleaseMemObject(dx);
  clReleaseMemObject(dy);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);
  
  return 0;
}

cl_int report_and_mark_devices(cl_device_id *devices, cl_uint num_devices, int *a_cpu, int *a_gpu, int *an_accelerator)
{
  int i, type_name_index = 0;
  cl_int err = 0;
  size_t returned_size;
  size_t max_workgroup_size = 0;
  cl_uint max_compute_units = 0, vec_width_char = 0, vec_width_short = 0;
  cl_uint vec_width_int = 0, vec_width_long = 0, vec_width_float = 0, vec_width_double = 0;
  char vendor_name[1024] = {0}, device_name[1024] = {0}, device_version[1024] = {0};
  cl_ulong global_mem_size;
  cl_device_type device_type;
  char type_names[3][27]={"CL_DEVICE_TYPE_CPU        " , "CL_DEVICE_TYPE_GPU        " , "CL_DEVICE_TYPE_ACCELERATOR"};
  
  for (i=0;i<num_devices;i++) {
    err = clGetDeviceInfo(devices[i], CL_DEVICE_TYPE,                          sizeof(device_type),    &device_type,        &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR,                        sizeof(vendor_name),    vendor_name,         &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_NAME,                          sizeof(device_name),    device_name,         &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_VERSION,                       sizeof(device_version), device_version,      &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_MAX_WORK_GROUP_SIZE,           sizeof(size_t),         &max_workgroup_size, &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS,             sizeof(cl_uint),        &max_compute_units,  &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_GLOBAL_MEM_SIZE,               sizeof(cl_ulong),       &global_mem_size,    &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,   sizeof(cl_uint),        &vec_width_char,     &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,  sizeof(cl_uint),        &vec_width_short,    &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,    sizeof(cl_uint),        &vec_width_int,      &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,   sizeof(cl_uint),        &vec_width_long,     &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,  sizeof(cl_uint),        &vec_width_float,    &returned_size);
    err|= clGetDeviceInfo(devices[i], CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint),        &vec_width_double,   &returned_size);
    
    if (err != CL_SUCCESS) {
      printf("Error: Failed to retrieve device info!\n");
      return EXIT_FAILURE;
    }

    if (device_type == CL_DEVICE_TYPE_CPU) {
      *a_cpu = i;
      type_name_index = 0;
    }

    if (device_type == CL_DEVICE_TYPE_GPU) {
      *a_gpu = i;
      type_name_index = 1;
    }

    if (device_type == CL_DEVICE_TYPE_ACCELERATOR) {
      *an_accelerator = i;
      type_name_index = 2;
    }

    printf("\nDevice information:\n");
    printf(SEPARATOR);
    printf("Type:               %s\n", type_names[type_name_index]);
    printf("Vendor:             %s\n", vendor_name);
    printf("Device:             %s\n", device_name);
    printf("Version:            %s\n", device_version);
    printf("Max workgroup size: %d\n", (int)max_workgroup_size);
    printf("Max compute units:  %d\n", (int)max_compute_units);
    printf("Global mem size:    %ld\n", (long)global_mem_size);
    printf(SEPARATOR);
    
    printf("\nPreferred vector widths by type:\n");
   
    printf(SEPARATOR);
    printf("Vector char:  %d\n",   (int)vec_width_char);
    printf("Vector short: %d\n",   (int)vec_width_short);
    printf("Vector int:   %d\n",   (int)vec_width_int);
    printf("Vector long:  %d\n",   (int)vec_width_long);
    printf("Vector float: %d\n",   (int)vec_width_float);
    printf("Vector dble:  %d\n",   (int)vec_width_double);
    printf(SEPARATOR);
    printf("\n");
  }
  return err;
}

/* saxpyOpenCL.c ends here */
