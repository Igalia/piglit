/* FAIL - user functions are not allowed in constant expressions */
#version 120
float id(x) {
   return x;
}

void main() {
   const float one = id(1.0);
   gl_FragColor = vec4(one);
}
