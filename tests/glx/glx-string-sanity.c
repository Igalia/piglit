/*
 * Copyright © 2011 Intel Corporation
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
 */


/** @file glx-string-sanity.c
 * Sanity check the various GLX extension strings that application can query.
 *
 * This test reproduces Mesa bug #56057.
 */

#include <stdbool.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

static const char *
eat_whitespace(const char *s)
{
	/* Page 17 of the GLX 1.4 spec says:
	 *
	 *     "The string is zero-terminated and contains a space-seperated
	 *     list of extension names."
	 *
	 * It doesn't say whitespace.  It just says space.
	 */
	while (*s == ' ' && *s != '\0')
		s++;

	return s;
}

static const char *
eat_characters(const char *s)
{
	while (*s != ' ' && *s != '\0')
		s++;

	return s;
}

static const char *
find_extension(const char *haystack, const char *needle, size_t needle_length)
{
	while ((haystack = strstr(haystack, needle)) != NULL) {
		if (haystack[needle_length] == '\0'
		    || haystack[needle_length] == ' ')
			break;
	}

	return haystack;
}

static bool
validate_string(const char *string, const char *name)
{
	bool pass = true;

	while (*string != '\0') {
		string = eat_whitespace(string);
		if (*string == '\0')
			break;

		if (strncmp("GLX_", string, 4) != 0) {
			char junk[15];

			/* Since the extension string may be very long, just
			 * log a few characters.
			 */
			strncpy(junk, string, sizeof(junk));
			junk[sizeof(junk) - 1] = '\0';
			fprintf(stderr, "%s contains junk: %s\n", name, junk);
			pass = false;
		}

		string = eat_characters(string);
	}

	return pass;
}

int
main(int argc, char **argv)
{
	bool pass = true;
	const char *client_string;
	const char *server_string;
	const char *unified_string;
	char *buf;

	Display *dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* First, make sure that all the strings have the correct format.
	 */
	server_string = glXQueryServerString(dpy, 0, GLX_EXTENSIONS);
	pass = validate_string(server_string, "server extensions string")
		&& pass;

	client_string = glXGetClientString(dpy, GLX_EXTENSIONS);
	pass = validate_string(client_string, "client extensions string")
		&& pass;

	unified_string = glXQueryExtensionsString(dpy, 0);
	pass = validate_string(unified_string, "unified extensions string")
		&& pass;

	/* Now make sure that any string listed in both the server string and
	 * the client string is listed in the unified string.
	 *
	 * Also make sure that any string *not* listed in the client string is
	 * *not* listed in the unified string.  Since there are several
	 * "client only" extensions (e.g., GLX_ARB_get_proc_address), it is
	 * valid for a string to not be in the server string when it is listed
	 * in the unified string.
	 */
	buf = malloc(strlen(server_string) + 1);
	while (*server_string != '\0') {
		const char *client;
		const char *end;
		size_t bytes;


		server_string = eat_whitespace(server_string);
		if (*server_string == '\0')
			break;

		end = eat_characters(server_string);

		bytes = (size_t)(end - server_string);
		memcpy(buf, server_string, bytes);
		buf[bytes] = '\0';

		/* Try to find the extension in the client extension list.
		 */
		client = find_extension(client_string, buf, bytes);
		if (client != NULL) {
			if (find_extension(unified_string, buf, bytes) == NULL) {
				fprintf(stderr,
					"%s found in both client and server "
					"extension strings, but missing from "
					"unified string.\n",
					buf);
				pass = false;
			}
		} else {
			if (find_extension(unified_string, buf, bytes) != NULL) {
				fprintf(stderr,
					"%s not found in client extension "
					"string, but found in unified "
					"string.\n",
					buf);
				pass = false;
			}
		}

		server_string = end;
	}

	free(buf);

	XCloseDisplay(dpy);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
