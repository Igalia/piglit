#if TYPE == double
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#endif

typedef TYPE type_t;

kernel void store_global(global type_t *out, global type_t *in) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
	out[4] = in[4];
	out[5] = in[5];
	out[6] = in[6];
	out[7] = in[7];
}

kernel void store_global_wi(global type_t *out, global type_t *in) {
	size_t id = get_global_id(0);
	out[id] = in[id];
}
