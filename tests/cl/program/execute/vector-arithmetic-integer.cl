typedef VECTOR_TYPE vector_t;

kernel void add(global vector_t* out, vector_t a, vector_t b) {
	out[0] = a + b;
}

kernel void sub(global vector_t* out, vector_t a, vector_t b) {
	out[0] = a - b;
}

kernel void mul(global vector_t* out, vector_t a, vector_t b) {
	out[0] = a * b;
}

kernel void div(global vector_t* out, vector_t a, vector_t b) {
	out[0] = a / b;
}

kernel void mod(global vector_t* out, vector_t a, vector_t b) {
	out[0] = a % b;
}

kernel void plus(global vector_t* out, vector_t in) {
	out[0] = +in;
}

kernel void minus(global vector_t* out, vector_t in) {
	out[0] = -in;
}

kernel void postinc(global vector_t* out, vector_t in) {
	out[0] = in++;
}

kernel void preinc(global vector_t* out, vector_t in) {
	out[0] = ++in;
}

kernel void postdec(global vector_t* out, vector_t in) {
	out[0] = in--;
}

kernel void predec(global vector_t* out, vector_t in) {
	out[0] = --in;
}
