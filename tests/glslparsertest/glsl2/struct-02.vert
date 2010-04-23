/* FAIL - structure name conflicts with another structure */

struct foo {
  float x;
  int y;
  bool z;
};

struct foo {
  float f;
  int i;
  bool b;
};
