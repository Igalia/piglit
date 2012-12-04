typedef OUT_TYPE out_t;
typedef SCALAR_TYPE scalar_t;

kernel void load_global(global out_t *out, global scalar_t *in) {
    out[0] = in[0];
}

kernel void load_param(global out_t *out, scalar_t in) {
	out[0] = in;
}

kernel void load_constant(global out_t *out, constant scalar_t *in) {
	out[0] = in[0];
}
