// Add you device OpenCL code
const sampler_t m_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void sobel_rgb(__read_only image2d_t src, __read_write image2d_t dst)
{
    int x = (int)get_global_id(0);
    int y = (int)get_global_id(1);
	float4 p00 = read_imagef(src, m_sampler, (int2)(x - 1, y - 1));
	float4 p10 = read_imagef(src, m_sampler, (int2)(x, y - 1));
	float4 p20 = read_imagef(src, m_sampler, (int2)(x + 1, y - 1));
	
	float4 p01 = read_imagef(src, m_sampler, (int2)(x - 1, y));
	float4 p21 = read_imagef(src, m_sampler, (int2)(x + 1, y));
	
	float4 p02 = read_imagef(src, m_sampler, (int2)(x - 1, y + 1));
	float4 p12 = read_imagef(src, m_sampler, (int2)(x, y + 1));
	float4 p22 = read_imagef(src, m_sampler, (int2)(x + 1, y + 1));

	float3 gx =-p00.xyz + p20.xyz + 2.0f * (p21.xyz - p01.xyz) - p02.xyz + p22.xyz;
	float3 gy =-p00.xyz - p20.xyz + 2.0f * (p12.xyz - p10.xyz) + p02.xyz + p22.xyz;

	float3 g = native_sqrt(gx * gx + gy * gy);
    write_imagef(dst, (int2)(x, y), (float4)(g.x, g.y, g.z, 1.0f));
}