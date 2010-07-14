/* FAIL - noise functions are not allowed in constant expressions */
#version 120
void main() {
   const vec2 noise = noise2(0.5);
}
