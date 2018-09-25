#ifndef _LQ_CL_H
#define _LQ_CL_H
#include <CL/cl.h>
void lq_enum_platform_list();
int lq_get_defualt_platform(cl_platform_id *plat_id);
int lq_get_defualt_device(cl_device_id *device_id, cl_platform_id *plat_id);
int lq_display_device_info(cl_device_id *device_id);
#endif
