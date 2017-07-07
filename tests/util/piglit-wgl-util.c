/*
 * Copyright © 2017 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * WGL utility functions, based on GLX utility functions by Eric Anholt.
 *
 * Authors:
 *    Brian Paul
 *
 */

#include <stdio.h>
#include "piglit-util-gl.h"
#include "piglit-wgl-util.h"


int piglit_width = 100;
int piglit_height = 100;


static LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		//reshape(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


HWND
piglit_get_wgl_window(void)
{
	int pixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle, dwStyle;
	HDC hDC;
	RECT winrect;
	HINSTANCE hInst;
	HWND hWnd;
	char *name = "wgl";

	static const PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	winrect.left = 0;
	winrect.right = piglit_width;
	winrect.top = 0;
	winrect.bottom = piglit_height;

	hInst = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = name;
	if (!RegisterClass(&wc)) {
		/* This fails for second and subsequent calls */
		/*
		fprintf(stderr, "failed to register class\n");
		fflush(stderr);
		return 0;
		*/
	}

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPEDWINDOW;
	AdjustWindowRectEx(&winrect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(dwExStyle, name, name,
				    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
				    0, 0,
				    winrect.right - winrect.left,
				    winrect.bottom - winrect.top,
				    NULL, NULL, hInst, NULL))) {
		fprintf(stderr, "failed to create window\n");
		fflush(stderr);
		return 0;
	}

	if (!(hDC = GetDC(hWnd))) {
		fprintf(stderr, "GetDC failed\n");
		fflush(stderr);
		return 0;
	}

	if (!(pixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		fprintf(stderr, "ChoosePixelFormat failed\n");
		fflush(stderr);
		return 0;
	}

	if (!(SetPixelFormat(hDC, pixelFormat, &pfd))) {
		fprintf(stderr, "SetPixelFormat failed\n");
		fflush(stderr);
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	return hWnd;
}


HGLRC
piglit_get_wgl_context(HWND hWnd)
{
	HDC hDC;
	HGLRC hRC;

	if (!(hDC = GetDC(hWnd))) {
		fprintf(stderr, "GetDC failed\n");
		fflush(stderr);
		return 0;
	}

	if (!(hRC = wglCreateContext(hDC))) {
		fprintf(stderr, "wglCreateContext failed\n");
		fflush(stderr);
		return 0;
	}

	return hRC;
}


void
piglit_wgl_event_loop(enum piglit_result (*draw)(void))
{
	MSG msg;
	enum piglit_result result = PIGLIT_SKIP;

	while(1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		result = draw();

		if (piglit_automatic) {
			break;
		}
	}

	piglit_report_result(result);
}
