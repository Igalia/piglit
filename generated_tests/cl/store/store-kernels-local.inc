#if TYPE == double
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

typedef TYPE type_t;

kernel void store_local(global type_t *out, global type_t *in) {
	local type_t local_data[8];
	size_t id = get_local_id(0);
	size_t store_index = (id + 1) % 8;
	local_data[store_index] = in[store_index];
	barrier(CLK_LOCAL_MEM_FENCE);
	out[id] = local_data[id];
}
