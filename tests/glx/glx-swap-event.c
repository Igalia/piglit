/*
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    jian.j.zhao@intel.com
 */

/*
 *  Simple test case on the intel_swap_event.
 *  
 */

#include "piglit-util-gl.h"
#include "GL/glx.h"
#include <sys/time.h>


#if defined(GLX_MESA_swap_control) && defined(GLX_INTEL_swap_event)


/* return current time (in seconds) */
static double
current_time(void)
{
    struct timeval tv;
#ifdef __VMS
    (void) gettimeofday(&tv, NULL );
#else
    struct timezone tz;
    (void) gettimeofday(&tv, &tz);
#endif
    return (double) tv.tv_sec + tv.tv_usec / 1000000.0;
}

PFNGLXSWAPINTERVALMESAPROC pglXSwapIntervalMESA;
PFNGLXGETSWAPINTERVALMESAPROC pglXGetSwapIntervalMESA;

#define STACK_L 10

static GLboolean fullscreen = GL_FALSE;    /* Create a fullscreen window */
static GLboolean verbose = GL_FALSE;    /* Disable verbose.  */
static GLboolean Automatic = GL_FALSE;    /* Test automatically.  */
static GLboolean test_events = GL_FALSE;    /* Test event can be received.  */
static GLboolean interval_diff = GL_FALSE;    /* Test interval can be set.  */
static GLboolean async = GL_FALSE;    /* Test asynchronize.  */
int event_base, Glx_event, count=0, swap_count=0, event_count=0;
static int Intel_swap_event=0;
int  event_count_total=0, frames_total=0, message_count=0;
static double time_call=0.0, time_fin=0.0, time_val=0.0;
double swap_start[STACK_L],swap_returned[STACK_L];
int interval=0;
char * swap_event_type=NULL;
/**
 * Determine whether or not a GLX extension is supported.
 */
static int
is_glx_extension_supported(Display *dpy, const char *query)
{
    const int scrnum = DefaultScreen(dpy);
    const char *glx_extensions = NULL;
    const size_t len = strlen(query);
    const char *ptr;
    
    if (glx_extensions == NULL) {
        glx_extensions = glXQueryExtensionsString(dpy, scrnum);
    }
    
    ptr = strstr(glx_extensions, query);
    return ((ptr != NULL) && ((ptr[len] == ' ') || (ptr[len] == '\0')));
}

static void
query_swap_event(Display *dpy)
{

    if ( ! is_glx_extension_supported(dpy, "GLX_INTEL_swap_event")) {
        printf("The GLX_INTEL_swap_event is not supported in current version.\n");
        piglit_report_result(PIGLIT_SKIP);
    } else {
        printf("The GLX_INTEL_swap_event is supported in current version.\n");
    }
    
    if (interval_diff) {
        if ( ! is_glx_extension_supported(dpy, "GLX_MESA_swap_control")) {
            printf("GLX_MESA_swap_control was not supported by the driver.\n");
            piglit_report_result(PIGLIT_SKIP);
        } else {
            pglXGetSwapIntervalMESA = (PFNGLXGETSWAPINTERVALMESAPROC) 
            glXGetProcAddressARB((const GLubyte *) "glXGetSwapIntervalMESA");
            pglXSwapIntervalMESA = (PFNGLXSWAPINTERVALMESAPROC) 
            glXGetProcAddressARB((const GLubyte *) "glXSwapIntervalMESA");
        }
    }
}

/** Draw single frame, do SwapBuffers, compute FPS */
static void
draw_frame(Display *dpy, Window win)
{
    static int frames = 0, async_swap;
    static double tRot0 = -1.0, tRate0 = -1.0;
    static double swap_freq[2];
    double t = current_time();
    int tem, ret;
    
    if (tRot0 < 0.0)
        tRot0 = t;
    tRot0 = t;
    
    if (tRate0 < 0.0)
        tRate0 = t;
    if (t - tRate0 >= 3.0) {
        GLfloat seconds;
        if (interval_diff) {
            if (message_count & 0x1) {
                ret = (*pglXSwapIntervalMESA)(1);
                if ( ret ) {
		    printf("Failed to set swap interval to 1 (%d).\n", ret);
                    piglit_report_result(PIGLIT_FAIL);
                }
            } else{
		ret = (*pglXSwapIntervalMESA)(0);
                if ( ret ) {
                    printf("Failed to set swap interval to 0.\n");
                    piglit_report_result(PIGLIT_FAIL);
                }
            }
        }
        message_count++;
        seconds = t - tRate0;
        if ( (time_val / frames) < 0.0016 ) {  
            // 0.0016 <=> 60Hz * 10 or 100Hz * 6 
            async_swap=1;
        } else{
            async_swap=0;
        }
        interval=1-interval;
        swap_freq[interval] = frames / seconds;
        if (Automatic) {
            if (message_count==2) {
                if (test_events) {
                    if ( ! Intel_swap_event == 0 ) {
                        if (verbose) {
                            printf("glXSwapBuffers is called %d times and there\
 is %d Intel_swap_event received in past %3.1f seconds.\n", swap_count,
event_count, seconds);
                            printf("There is swap event received, and the swap \
type is %s.\n", swap_event_type);
                        }
                        piglit_report_result(PIGLIT_PASS);
                    } else{
                        if (verbose) {
                            printf("glXSwapBuffers is called %d times and there\
 is %d Intel_swap_event received in past %3.1f seconds.\n", swap_count,
event_count, seconds);
                            printf("There is no swap event received, and the \
swap type is %s.\n", swap_event_type);
                        }
                        piglit_report_result(PIGLIT_FAIL);
                    }
                    
                }
                if (interval_diff) {
                    tem = 1.5 * swap_freq[1];
                    if ( swap_freq[0] >= tem ) {
                        if (verbose) {
                            printf("The swap frequency of no swap interval is \
much larger than swap interval being 1.\n");
                        }
                        piglit_report_result(PIGLIT_PASS);
                    } else{
			if(fullscreen)
			{
			   if(verbose)
			        printf("In fullscreen mode, the swap frequency of \
no swap interval is limited under fresh rate.\n");
			   piglit_report_result(PIGLIT_PASS);
			}
                        if (verbose) {
                            printf("The swap frequency of no swap interval is \
not much larger than swap interval being 1. They are %lf and %lf.\n",
swap_freq[0], swap_freq[1]);
                        }
                        piglit_report_result(PIGLIT_FAIL);
                    }
                }
                if (async) {
                    if (verbose) {
                        printf("It takes about %lf seconds returning back from\
 the glXSwapBuffers call on average.\n", (time_val / frames));
                    }
                    if (async_swap ==1 ) {
                        piglit_report_result(PIGLIT_PASS);
                    } else{
                        piglit_report_result(PIGLIT_FAIL);
                    }
                }
            }    
        }
        tRate0 = t;
        frames = 0;
        time_val = 0;
        swap_count= 0;
        event_count= 0;
    }
         
    if (frames_total & 1) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    count=frames_total%STACK_L;
    time_call=current_time();
    swap_start[count]=time_call;
    glXSwapBuffers(dpy, win);
    time_fin=current_time();
    swap_returned[count]=time_fin;
    time_val+=(time_fin-time_call);
    
    frames++;
    frames_total++;
    swap_count++;
}

/**
 * Remove window border/decorations.
 */
static void
no_border( Display *dpy, Window w)
{
    static const unsigned MWM_HINTS_DECORATIONS = (1 << 1);
    static const int PROP_MOTIF_WM_HINTS_ELEMENTS = 5;
    
    typedef struct
    {
        unsigned long       flags;
        unsigned long       functions;
        unsigned long       decorations;
        long                inputMode;
        unsigned long       status;
    } PropMotifWmHints;
    
    PropMotifWmHints motif_hints;
    Atom prop, proptype;
    unsigned long flags = 0;
    
    /* setup the property */
    motif_hints.flags = MWM_HINTS_DECORATIONS;
    motif_hints.decorations = flags;
    
    /* get the atom for the property */
    prop = XInternAtom( dpy, "_MOTIF_WM_HINTS", True );
    if (!prop) {
        /* something went wrong! */
        return;
    }
    
    /* not sure this is correct, seems to work, XA_WM_HINTS didn't work */
    proptype = prop;
    
    XChangeProperty( dpy, w,                         /* display, window */
                     prop, proptype,                 /* property, type */
                     32,                             /* format: 32-bit datums */
                     PropModeReplace,                /* mode */
                     (unsigned char *) &motif_hints, /* data */
                     PROP_MOTIF_WM_HINTS_ELEMENTS    /* nelements */
                   );
}


/*
 * Create an RGB, double-buffered window.
 * Return the window and context handles.
 */
static void
make_window( Display *dpy, const char *name,
             int x, int y, int width, int height,
             Window *winRet, GLXContext *ctxRet, GLXWindow *glxWinRet)
{
    int attribs[] = {
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,   GLX_RGBA_BIT,
            GLX_DOUBLEBUFFER,  True,  /* Request a double-buffered color buffer with */
            GLX_RED_SIZE,      1,     /* the maximum number of bits per component    */
            GLX_GREEN_SIZE,    1,
            GLX_BLUE_SIZE,     1,
            None };

    int scrnum, nelements;
    XSetWindowAttributes attr;
    unsigned long mask;
    Window root;
    Window win;
    GLXContext ctx;
    XVisualInfo *visinfo;
    GLXFBConfig *fbc;
    GLXWindow gwin;
    Bool ret;
    
    scrnum = DefaultScreen( dpy );
    root = RootWindow( dpy, scrnum );
    
    if (fullscreen) {
        x = 0; y = 0;
        width = DisplayWidth( dpy, scrnum );
        height = DisplayHeight( dpy, scrnum );
    }
    
    fbc = glXChooseFBConfig(dpy, scrnum, attribs, &nelements);
    if (!fbc) {
	    printf("Error: couldn't get framebuffer config\n");
	    piglit_report_result(PIGLIT_FAIL);
    }
    visinfo = glXGetVisualFromFBConfig(dpy, fbc[0]);
    if (!visinfo) {
        printf("Error: couldn't get an RGB, Double-buffered visual\n");
        piglit_report_result(PIGLIT_SKIP);
    }
    ctx = glXCreateNewContext(dpy, fbc[0], GLX_RGBA_TYPE, 0, GL_TRUE);

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    /* XXX this is a bad way to get a borderless window! */
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    win = XCreateWindow( dpy, root, x, y, width, height,
                     0, visinfo->depth, InputOutput,
                     visinfo->visual, mask, &attr );
    XMapWindow(dpy, win);
    gwin = glXCreateWindow(dpy, fbc[0], win, attribs);
    ret = glXMakeContextCurrent(dpy, gwin, gwin, ctx);
    if (!ret)
	    printf("make current failed: %d\n", glGetError());
    glXSelectEvent(dpy, gwin, GLX_BUFFER_SWAP_COMPLETE_INTEL_MASK);

    if (fullscreen)
        no_border(dpy, win);

    
    XFree(visinfo);
    
    *winRet = win;
    *ctxRet = ctx;
    *glxWinRet = gwin;
}


static int64_t last_sbc;

/**
 * Only handle one glx event.
 */
void
handle_event(Display *dpy, Window win, XEvent *event)
{
    (void) dpy;

    if ( Glx_event == event->type) {
        XEvent * event_p=event;
        GLXBufferSwapComplete * glx_event=(GLXBufferSwapComplete *) event_p;
        static double t_last=-1.0;
        time_fin=current_time();
        if (t_last < 0) {
            t_last=time_fin;
        }
        if ( time_fin - t_last >= 3.0) {
            if ( verbose ) {
                count=event_count_total%STACK_L;
                printf("It receives the recent event at %lf seconds, and that\
 glXSwapBuffers was called at %lf seconds, its swap returned at %lf seconds, so\
 the total time of glXSwapBuffers takes is %lf seconds.\n", time_fin,
swap_start[count], swap_returned[count], (time_fin-swap_start[count]));
            }
            t_last=time_fin;
        }

        if (glx_event->drawable != win) {
            printf("Error: swap event was not on X11 Drawable\n");
            piglit_report_result(PIGLIT_FAIL);
        }

	if (glx_event->sbc == 0) {
		printf("Error: swap event returned 0 swap count\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (glx_event->sbc == last_sbc) {
		printf("Error: swap event count did not change\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	last_sbc = glx_event->sbc;

	if (verbose)
		printf("swap event: ust %lld\tmsc %lld\tsbc %lld\n",
		       (unsigned long long)glx_event->ust,
		       (unsigned long long)glx_event->msc,
		       (unsigned long long)glx_event->sbc);
        switch (glx_event->event_type) {
        case GLX_EXCHANGE_COMPLETE_INTEL:
            Intel_swap_event=GLX_EXCHANGE_COMPLETE_INTEL;
            swap_event_type="GLX_EXCHANGE_COMPLETE_INTEL";
            event_count++;
            event_count_total++;
            break;
        case GLX_COPY_COMPLETE_INTEL:
            Intel_swap_event=GLX_COPY_COMPLETE_INTEL;
            swap_event_type="GLX_COPY_COMPLETE_INTEL";
            event_count++;
            event_count_total++;
            break;
        case GLX_FLIP_COMPLETE_INTEL:
            Intel_swap_event=GLX_FLIP_COMPLETE_INTEL;
            swap_event_type="GLX_FLIP_COMPLETE_INTEL";
            event_count++;
            event_count_total++;
            break;
        }
    }
}


static void
event_loop(Display *dpy, GLXWindow glxWin, Window win)
{
    while (1) {
        while (XPending(dpy) > 0) {
            XEvent event;
            XNextEvent(dpy, &event);
            Glx_event=event_base + GLX_BufferSwapComplete;
            handle_event(dpy, win, &event);
        }
        
        draw_frame(dpy, glxWin);
    }
}


static void
usage(void)
{
    printf("Usage:\n");
    printf("  -fullscreen             run in fullscreen mode\n");
    printf("  -v       verbose mode, have more log\n");
    printf("  -auto       test automatically \n");
    printf(" --event         test whether we can get swap events\n");
    printf(" --interval      we expect that swap interval set to 0 should \
have higher swap frequency than interval to 1\n");
    printf(" --async   test whether glXSwapBuffers is done asynchronously\n");
}
 

int
main(int argc, char *argv[])
{
    unsigned int winWidth = 30, winHeight = 30;
    int x = 0, y = 0;
    Display *dpy;
    Window win;
    GLXWindow glxWin;
    GLXContext ctx;
    char *dpyName = NULL;
    int i, error_base, ret;
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-auto") == 0) {
            Automatic = GL_TRUE;
        }
        else if (strcmp(argv[i], "-v") == 0) {
            verbose = GL_TRUE;
        }
        else if (strcmp(argv[i], "-fullscreen") == 0) {
            fullscreen = GL_TRUE;
        }
        else if (strcmp(argv[i], "--event") == 0) {
            test_events=GL_TRUE;
        }
        else if (strcmp(argv[i], "--async") == 0) {
            async = GL_TRUE;
        }
        else if (strcmp(argv[i], "--interval") == 0) {
            interval_diff = GL_TRUE;
        }
        else {
            usage();
            piglit_report_result(PIGLIT_SKIP);
        }
    }
    if (!( interval_diff || async || test_events )) {
       printf("Which do you want to test, events? asynchronous? or swap interval?\n");
       usage();
       piglit_report_result(PIGLIT_SKIP);
    }
    
    dpy = XOpenDisplay(dpyName);
    if (!dpy) {
        printf("Error: couldn't open display %s\n",
            dpyName ? dpyName : getenv("DISPLAY"));
        piglit_report_result(PIGLIT_FAIL);
    }
    
    make_window(dpy, "Swap event test", x, y, winWidth, winHeight, &win, &ctx, &glxWin);
    
    query_swap_event(dpy);
    
    glXQueryExtension(dpy, &error_base, &event_base);
    
    if (interval_diff) {
        ret = (*pglXSwapIntervalMESA)(1);
        if ( ret ) {
	    printf("Failed to set swap interval to 1 (%d).\n", ret);
            piglit_report_result(PIGLIT_FAIL);
        }
    }
    piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);
    event_loop(dpy, glxWin, win);
    
    glXDestroyContext(dpy, ctx);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    
    return 0;
}

#else

int
main(int argc, char *argv[])
{
   piglit_report_result(PIGLIT_SKIP);
   return 0;
}

#endif /* GLX_MESA_swap_control && GLX_INTEL_swap_event */
