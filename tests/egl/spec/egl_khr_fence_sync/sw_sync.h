/*
 * Copyright 2012 Google, Inc
 * Copyright © 2016 Collabora, Ltd.
 *
 * Based on the implementation from the Android Open Source Project
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
 * Authors:
 *    Robert Foss <robert.foss@collabora.com>
 */

#ifndef SW_SYNC_H
#define SW_SYNC_H

#include <stdbool.h>

#define SW_SYNC_FENCE_STATUS_ERROR		(-1)
#define SW_SYNC_FENCE_STATUS_ACTIVE		(0)
#define SW_SYNC_FENCE_STATUS_SIGNALED	(1)

bool sw_sync_is_supported(void);
int sw_sync_timeline_create(void);
void sw_sync_timeline_destroy(int timeline);
void sw_sync_fence_destroy(int fence);
int sw_sync_fence_create(int fd, int32_t seqno);
void sw_sync_timeline_inc(int fd, uint32_t count);

#endif

