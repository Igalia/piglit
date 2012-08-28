/*
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "common.h"

static void
x11_process_next_event(void)
{
	struct glut_window *gwin = _glut->window;
	Display *xdpy = gwin->x11.display;

	bool redraw = false;
	XEvent event;

	if (!XPending(xdpy))
		return;

	XNextEvent(xdpy, &event);

	switch (event.type) {
		case Expose:
			redraw = true;
			break;
		case ConfigureNotify:
			if (gwin->reshape_cb)
				gwin->reshape_cb(event.xconfigure.width,
				                 event.xconfigure.height);
			break;
		case KeyPress: {
			char buffer[1];
			KeySym sym;
			int n;

			redraw = true;
			n = XLookupString(&event.xkey,
					  buffer,
					  sizeof(buffer), &sym, NULL);

			if (n > 0 && gwin->keyboard_cb)
				gwin->keyboard_cb(buffer[0],
				                  event.xkey.x, event.xkey.y);
			break;
		}
		default:
			break;
	}

	_glut->redisplay = redraw;
}

void
x11_event_loop(void)
{
	while (true) {
		x11_process_next_event();

		if (_glut->redisplay) {
			_glut->redisplay = 0;

			if (_glut->window->display_cb)
				_glut->window->display_cb();
		}
	}
}
