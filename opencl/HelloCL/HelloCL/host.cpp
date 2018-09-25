#include<stdlib.h>
#include "lq_cl.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>

#define ARRAY_SIZE 10000000

cl_context CreateContext() 
{
	cl_int err_code;
	cl_uint plat_num;
	cl_platform_id first_plat;
	cl_context context = NULL;

	err_code = clGetPlatformIDs(1, &first_plat, &plat_num);
	if (err_code != CL_SUCCESS || plat_num <= 0) {
		std::cerr << "Failed to find any avalid OpenCL platforms." << std::endl;
		return NULL;
	}

	cl_context_properties con_properties[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)first_plat,
		0
	};
	context = clCreateContextFromType(con_properties,
		CL_DEVICE_TYPE_CPU,
		NULL,
		NULL,
		&err_code);
	if (err_code != CL_SUCCESS) {
		context = clCreateContextFromType(con_properties,
			CL_DEVICE_TYPE_GPU,
			NULL,
			NULL,
			&err_code);
	}
	if (err_code != CL_SUCCESS) {
		std::cerr << "Failed to Create an OpenCL GPU or CPU context." << std::endl;
		return NULL;
	}
	return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id *device) 
{
	cl_command_queue command_queue = NULL;
	size_t device_buff_size = 0;
	cl_int err_code;
	cl_device_id *devices;
	
	err_code = clGetContextInfo(context, CL_CONTEXT_DEVICES, 
		0, NULL, &device_buff_size);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Failed to call clGetContextInfo." << std::endl;
		return NULL;
	}
	if (device_buff_size <= 0) {
		std::cerr << "No devices availabe." << std::endl;
		return NULL;
	}

	devices = new cl_device_id[device_buff_size / sizeof(cl_device_id)];

	err_code = clGetContextInfo(context, CL_CONTEXT_DEVICES,
		device_buff_size, devices, NULL);
	if (err_code != CL_SUCCESS) {
		std::cerr << "No devices availabe." << std::endl;
		delete[] devices;
		return NULL;
	}

	command_queue = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (command_queue == NULL) {
		std::cerr << "Failed to create commandqueue for device 0." << std::endl;
		delete[] devices;
		return NULL;
	}

	*device = devices[0];
	delete[] devices;
	return command_queue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char* file_name)
{
	cl_program program = NULL;
	cl_int err_code;

	std::ifstream kernel_file(file_name, std::ios::in);
	if (!kernel_file.is_open()) {
		std::cerr << "Failed to open file " << file_name << std::endl;
		return NULL;
	}
	std::ostringstream oss;
	oss << kernel_file.rdbuf();
	std::string str_source = oss.str();
	const char *char_source = str_source.c_str();
	program = clCreateProgramWithSource(context,
		1, (const char **)&char_source,
		NULL, NULL);
	if (program == NULL) {
		std::cerr << "Failed to create CL program from source." << std::endl;
		return NULL;
	}
	err_code = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err_code != CL_SUCCESS) {
		char build_log[16383];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
			sizeof(build_log), build_log, NULL);
		std::cerr << "Error in kernel:" << std::endl;
		std::cerr << build_log << std::endl;
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}

bool CreaateMemoryObjs(cl_context context, cl_mem memObjects[3], float *a, float *b) 
{
	memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * ARRAY_SIZE, a, NULL);

	memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(float) * ARRAY_SIZE, b, NULL);

	memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
		sizeof(float) * ARRAY_SIZE, NULL, NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL) {
		std::cerr << "Error creating memory objects." << std::endl;
		return false;
	}

	return true;
}


int main()
{
	cl_context context; 
	cl_device_id device;
	cl_command_queue command_queue;
	cl_program program;
	cl_kernel kernel;
	cl_mem mem_objs[3];
	cl_int err_code;
	// 1. create an opencl context on an available platform
	context = CreateContext();

	// 2. create a command-queue on a device and context
	command_queue = CreateCommandQueue(context, &device);

	lq_display_device_info(&device);
	lq_enum_platform_list();
	// 3. create opencl program
	program = CreateProgram(context, device, "hello_world.cl");

	// 4. create opencl kernel
	kernel = clCreateKernel(program, "hello_kernel", NULL);
	if (kernel == NULL) {
		std::cerr << "Failed to create kernel" << std::endl;
		return false;
	}

	// 5. create memory objects
	float *result = new float[ARRAY_SIZE];
	float *a = new float[ARRAY_SIZE];
	float *b = new float[ARRAY_SIZE];

	
	for (int i = 0; i < ARRAY_SIZE; i++) {
		a[i] = i;
		b[i] = 3 * i;
	}

	int bet = CreaateMemoryObjs(context, mem_objs, a, b);
	if (!bet) {
		std::cerr << "Failed to create kernel" << std::endl;
		return false;
	}

	// 6. queue the kernel and execute
	err_code = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_objs[0]);
	err_code |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_objs[1]);
	err_code |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem_objs[2]);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in setting arguments." << std::endl;
		return false;
	}

	size_t global_work_size[1] = {ARRAY_SIZE};
	size_t local_work_size[1] = { 1 };
	clock_t start_time = clock();
	err_code = clEnqueueNDRangeKernel(command_queue,
		kernel,
		1,
		NULL,
		global_work_size,
		local_work_size,
		0, NULL, NULL);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in queuing kernel for execution." << std::endl;
		return false;
	}

	// 7. read back result
	err_code = clEnqueueReadBuffer(command_queue, mem_objs[2],
		CL_TRUE, 0,
		ARRAY_SIZE * sizeof(float),
		result,
		0, NULL, NULL);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in reading back result." << std::endl;
		return false;
	}
	clock_t end_time = clock();
	std::cout << "cost time: " << end_time - start_time << "ms." << std::endl;
	

	for (int i = 0; i < 10; i++) {
		std::cout << result[i] << std::endl;
	}

	system("pause");
	return true;
}