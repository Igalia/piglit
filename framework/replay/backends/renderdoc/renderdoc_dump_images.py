#!/usr/bin/env python3
# coding=utf-8
#
# Copyright (c) 2019 Collabora Ltd
# Copyright © 2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

import argparse
import atexit
import os
import shutil
import sys
import tempfile
from pathlib import Path

def cleanup(dirpath):
    shutil.rmtree(dirpath)

TMP_DIR = tempfile.mkdtemp()
atexit.register(cleanup, TMP_DIR)
RENDERDOC_DEBUG_FILE = TMP_DIR + "/renderdoc.log"

# Needs to be in the environment before importing the module
os.environ['RENDERDOC_DEBUG_LOG_FILE'] = RENDERDOC_DEBUG_FILE
import renderdoc as rd


def find_draw_with_event_id(controller, event_id):
    for d in controller.GetDrawcalls():
        if d.eventId == event_id:
            return d

    return None


def dump_image(controller, event_id, output_dir, tracefile):
    draw = find_draw_with_event_id(controller, event_id)
    if draw is None:
        raise RuntimeError("Couldn't find draw call with event ID " +
                           str(event_id))

    controller.SetFrameEvent(draw.eventId, True)

    texsave = rd.TextureSave()

    # Select the first color output
    texsave.resourceId = draw.outputs[0]

    if texsave.resourceId == rd.ResourceId.Null():
        return

    filepath = Path(output_dir)
    filepath.mkdir(parents = True, exist_ok = True)
    filepath = filepath / (tracefile + "-" + str(int(draw.eventId)) + ".png")

    print("Saving image at event ID %d: %s to %s" % (draw.eventId,
                                                     draw.name, filepath))

    # Most formats can only display a single image per file, so we select the
    # first mip and first slice
    texsave.mip = 0
    texsave.slice.sliceIndex = 0

    # For formats with an alpha channel, preserve it
    texsave.alpha = rd.AlphaMapping.Preserve
    texsave.destType = rd.FileType.PNG
    controller.SaveTexture(texsave, str(filepath))


def load_capture(filename):
    cap = rd.OpenCaptureFile()

    status = cap.OpenFile(filename, '', None)

    if status != rd.ReplayStatus.Succeeded:
        raise RuntimeError("Couldn't open file: " + str(status))
    if not cap.LocalReplaySupport():
        raise RuntimeError("Capture cannot be replayed")

    status, controller = cap.OpenCapture(rd.ReplayOptions(), None)

    if status != rd.ReplayStatus.Succeeded:
        if os.path.exists(RENDERDOC_DEBUG_FILE):
            print(open(RENDERDOC_DEBUG_FILE, "r").read())
        raise RuntimeError("Couldn't initialise replay: " + str(status))

    if os.path.exists(RENDERDOC_DEBUG_FILE):
        open(RENDERDOC_DEBUG_FILE, "w").write("")

    return (cap, controller)


def renderdoc_dump_images(filename, event_ids, output_dir):
    rd.InitialiseReplay(rd.GlobalEnvironment(), [])
    cap, controller = load_capture(filename);

    tracefile = Path(filename).name

    if not event_ids:
        event_ids.append(controller.GetDrawcalls()[-1].eventId)

    for event_id in event_ids:
        dump_image(controller, event_id, output_dir, tracefile)

    cap.Shutdown()

    rd.ShutdownReplay()


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('file_path',
                        help='the path to a trace file.')
    parser.add_argument('output_dir',
                        help='the path in which to place the results')
    parser.add_argument('draw_id',
                        type=int,
                        nargs=argparse.REMAINDER,
                        help=('a draw-id number from the trace to dump. '
                              'If none are provided, by default, '
                              'the last frame will be used.'))

    args = parser.parse_args()

    renderdoc_dump_images(args.file_path, args.draw_id, args.output_dir)


if __name__ == '__main__':
    main()
