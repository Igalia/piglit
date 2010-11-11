// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - variable hides type constructor */

uniform bool condition;
void main() {
   struct id {
      int n;
   };
   if (condition) {
      float id = 1.0;
      int x = id(5);
   }
}
