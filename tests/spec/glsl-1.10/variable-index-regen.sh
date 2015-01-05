#!/bin/sh
#
# Run this script with ./variable-index-regen.sh to regenerate all the
# variable-indexing scripts.

(cd execution/variable-indexing &&
	../../../glsl-1.10/variable-index-read.sh 1.10 &&
	../../../glsl-1.10/variable-index-write.sh 1.10)

(cd ../glsl-1.20/execution/variable-indexing &&
	../../../glsl-1.10/variable-index-read.sh 1.20 &&
	../../../glsl-1.10/variable-index-write.sh 1.20)
