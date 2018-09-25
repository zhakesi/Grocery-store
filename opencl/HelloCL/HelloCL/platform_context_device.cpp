#include <cassert>
#include <malloc.h>
#include <iostream>
#include "lq_cl.h"

void lq_enum_platform_list() 
{
	cl_uint numPlatforms;
	cl_platform_id *platform_ids;
	cl_int errCode;

	errCode = clGetPlatformIDs(0, NULL, &numPlatforms);
	assert(errCode == CL_SUCCESS);
	std::cout << numPlatforms << " valid platforms total" << std::endl;

	platform_ids = (cl_platform_id *)alloca(numPlatforms * sizeof(cl_platform_id));
	errCode = clGetPlatformIDs(numPlatforms, platform_ids, NULL);
	assert(errCode == CL_SUCCESS);
	size_t size;
	char *str_buf;
	for (cl_uint i = 0; i < numPlatforms; i++) {
		std::cout<<"*********************************"<<std::endl;
		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, 0, NULL, &size);
		str_buf = (char *)alloca(size);
		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_NAME, size, str_buf, NULL);
		std::cout << str_buf << std::endl;

		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, 0, NULL, &size);
		str_buf = (char *)alloca(size);
		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VERSION, size, str_buf, NULL);
		std::cout << str_buf << std::endl;

		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR, 0, NULL, &size);
		str_buf = (char *)alloca(size);
		clGetPlatformInfo(platform_ids[i], CL_PLATFORM_VENDOR, size, str_buf, NULL);
		std::cout << str_buf << std::endl;
	}
	std::cout << "*********************************" << std::endl;
}

int lq_get_defualt_platform(cl_platform_id *plat_id)
{
	cl_uint num_platforms;
	cl_int err_code;
	size_t size;
	char *str_buf;

	err_code = clGetPlatformIDs(0, NULL, &num_platforms);
	assert(err_code == CL_SUCCESS);
	if (num_platforms <= 0)
		return false;

	err_code = clGetPlatformIDs(1, plat_id, NULL);
	assert(err_code == CL_SUCCESS);

	std::cout << "*********************************" << std::endl;
	clGetPlatformInfo(*plat_id, CL_PLATFORM_NAME, 0, NULL, &size);
	str_buf = (char *)alloca(size);
	clGetPlatformInfo(*plat_id, CL_PLATFORM_NAME, size, str_buf, NULL);
	std::cout << str_buf << std::endl;

	clGetPlatformInfo(*plat_id, CL_PLATFORM_VERSION, 0, NULL, &size);
	str_buf = (char *)alloca(size);
	clGetPlatformInfo(*plat_id, CL_PLATFORM_VERSION, size, str_buf, NULL);
	std::cout << str_buf << std::endl;

	clGetPlatformInfo(*plat_id, CL_PLATFORM_VENDOR, 0, NULL, &size);
	str_buf = (char *)alloca(size);
	clGetPlatformInfo(*plat_id, CL_PLATFORM_VENDOR, size, str_buf, NULL);
	std::cout << str_buf << std::endl;
	std::cout << "*********************************" << std::endl;
	
	return true;
}

int lq_get_defualt_device(cl_device_id *device_id, cl_platform_id *plat_id)
{
	cl_int err_code;
	cl_device_type type;
	std::string str_type;
	
	err_code = clGetDeviceIDs(*plat_id, CL_DEVICE_TYPE_DEFAULT, 1, device_id, NULL);
	assert(err_code == CL_SUCCESS);

	clGetDeviceInfo(*device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
	switch (type)
	{
	case CL_DEVICE_TYPE_DEFAULT:
		str_type = "CL_DEVICE_TYPE_DEFAULT";
		break;
	case CL_DEVICE_TYPE_CPU:
		str_type = "CL_DEVICE_TYPE_CPU";
		break;
	case CL_DEVICE_TYPE_GPU:
		str_type = "CL_DEVICE_TYPE_GPU";
		break;
	case CL_DEVICE_TYPE_ACCELERATOR:
		str_type = "CL_DEVICE_TYPE_ACCELERATOR";
		break;
	case CL_DEVICE_TYPE_CUSTOM:
		str_type = "CL_DEVICE_TYPE_CUSTOM";
		break;
	default:
		str_type = "UNKNOWN";
		break;
	}
	std::cout << str_type.c_str() << std::endl;
	cl_uint compute_units;
	clGetDeviceInfo(*device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	std::cout <<"CL_DEVICE_MAX_COMPUTE_UNITS: "<< compute_units << std::endl;
	std::cout << "*********************************"<< std::endl;
	return true;
}

int lq_display_device_info(cl_device_id *device_id) 
{
	cl_int err_code;
	cl_device_type type;
	std::string str_type;

	clGetDeviceInfo(*device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
	switch (type)
	{
	case CL_DEVICE_TYPE_DEFAULT:
		str_type = "CL_DEVICE_TYPE_DEFAULT";
		break;
	case CL_DEVICE_TYPE_CPU:
		str_type = "CL_DEVICE_TYPE_CPU";
		break;
	case CL_DEVICE_TYPE_GPU:
		str_type = "CL_DEVICE_TYPE_GPU";
		break;
	case CL_DEVICE_TYPE_ACCELERATOR:
		str_type = "CL_DEVICE_TYPE_ACCELERATOR";
		break;
	case CL_DEVICE_TYPE_CUSTOM:
		str_type = "CL_DEVICE_TYPE_CUSTOM";
		break;
	default:
		str_type = "UNKNOWN";
		break;
	}
	std::cout << str_type.c_str() << std::endl;
	cl_uint compute_units;
	clGetDeviceInfo(*device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	std::cout << "CL_DEVICE_MAX_COMPUTE_UNITS: " << compute_units << std::endl;
	std::cout << "*********************************" << std::endl;
	return true;
}
