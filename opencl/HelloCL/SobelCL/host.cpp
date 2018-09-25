#include<stdlib.h>
#include <CL/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#pragma comment(lib, "opencv_core2413d.lib")
#pragma comment(lib, "opencv_highgui2413d.lib")
#pragma comment(lib, "opencv_imgproc2413d.lib")

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


int main()
{
	cl_context context;
	cl_device_id device;
	cl_command_queue command_queue;
	cl_program program;
	cl_kernel kernel;
	
	cl_int err_code;
	// 1. create an opencl context on an available platform
	context = CreateContext();

	// 2. create a command-queue on a device and context
	command_queue = CreateCommandQueue(context, &device);

	//lq_display_device_info(&device);

	// 3. create opencl program
	program = CreateProgram(context, device, "device.cl");

	// 4. create opencl kernel
	kernel = clCreateKernel(program, "sobel_rgb", NULL);
	if (kernel == NULL) {
		std::cerr << "Failed to create kernel" << std::endl;
		return false;
	}

	// 5. create memory objects
	cv::Mat src = cv::imread("C:/Users/qiao.li/Documents/code/sources/cv_data/fruits.jpg",
		CV_LOAD_IMAGE_COLOR);
	cv::cvtColor(src, src, CV_RGB2RGBA);
	int width = src.cols;
	int height = src.rows;

	cl_image_format img_fmt;
	img_fmt.image_channel_order = CL_RGBA;
	img_fmt.image_channel_data_type = CL_UNORM_INT8;
	
	cl_mem mem_img_src;
	mem_img_src = clCreateImage2D(context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&img_fmt,
		width,
		height,
		0,
		src.data,
		&err_code);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Failed to create src CL image." << std::endl;
		return false;
	}
	
	cl_mem mem_img_dst;
	mem_img_dst = clCreateImage2D(context,
		CL_MEM_READ_WRITE,
		&img_fmt,
		width,
		height,
		0,
		NULL,
		&err_code);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Failed to create dst CL image." << std::endl;
		return false;
	}

	// 6. queue the kernel and execute
	err_code = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem_img_src);
	err_code |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &mem_img_dst);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in setting arguments." << std::endl;
		return false;
	}

	
	size_t local_work_size[2] = { 32, 32 };
	size_t global_work_size[2] = {512, 480};
	clock_t start_time = clock();


	err_code = clEnqueueNDRangeKernel(command_queue,
		kernel,
		2,
		NULL,
		global_work_size,
		local_work_size,
		0, NULL, NULL);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in queuing kernel for execution." << std::endl;
		return false;
	}
	

	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { width, height, 1 };
	size_t row_pitch = 0;
	// 7. read back result
	unsigned char *dst_buff = (unsigned char *)clEnqueueMapImage(command_queue,
		mem_img_dst,
		CL_TRUE, CL_MAP_READ, 
		origin, region, &row_pitch,
		NULL, 0, NULL, NULL, &err_code);
	if (err_code != CL_SUCCESS) {
		std::cerr << "Error in reading back result." << std::endl;
		return false;
	}
	std::cout << dst_buff[0] << dst_buff[1] << dst_buff[2] << std::endl;
	clFinish(command_queue);
	cv::Mat dst = cv::Mat(cv::Size(width, height), CV_8UC4, dst_buff);
	clock_t end_time = clock();
	std::cout << "cost time: " << end_time - start_time << "ms." << std::endl;
	
	cv::imshow("origin", src);
	cv::imshow("result", dst);
	cv::waitKey(0);
	system("pause");
	return true;
}