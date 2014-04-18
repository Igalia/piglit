#define glXGetSyncValuesOML(dpy, drawable, ust, msc, sbc) __piglit_glXGetSyncValuesOML(dpy, drawable, ust, msc, sbc)
#define glXGetMscRateOML(dpy, drawable, numerator, denominator) __piglit_glXGetMscRateOML(dpy, drawable, numerator, denominator)
#define glXSwapBuffersMscOML(dpy, drawable, target_msc, divisor, remainder) __piglit_glXSwapBuffersMscOML(dpy, drawable, target_msc, divisor, remainder)
#define glXWaitForMscOML(dpy, drawable, target_msc, divisor, remainder, ust, msc, sbc) __piglit_glXWaitForMscOML(dpy, drawable, target_msc, divisor, remainder, ust, msc, sbc)
#define glXWaitForSbcOML(dpy, drawable, target_sbc, ust, msc, sbc) __piglit_glXWaitForSbcOML(dpy, drawable, target_sbc, ust, msc, sbc)

extern PFNGLXGETSYNCVALUESOMLPROC __piglit_glXGetSyncValuesOML;
extern PFNGLXGETMSCRATEOMLPROC __piglit_glXGetMscRateOML;
extern PFNGLXSWAPBUFFERSMSCOMLPROC __piglit_glXSwapBuffersMscOML;
extern PFNGLXWAITFORMSCOMLPROC __piglit_glXWaitForMscOML;
extern PFNGLXWAITFORSBCOMLPROC __piglit_glXWaitForSbcOML;

extern Window win;
extern XVisualInfo *visinfo;

void piglit_oml_sync_control_test_run(enum piglit_result (*draw)(Display *dpy));
