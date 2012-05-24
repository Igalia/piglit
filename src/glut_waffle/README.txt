glut_waffle is transitionary only and not intended to be a permanent
addition to Piglit. Its purpose is to make Piglit's transition from GLUT
to Waffle go smoothly. Once the transition is complete, piglit-framework.c
will be updated to use Waffle directly, and libglut_waffle will be
removed.
