#!/bin/sh
# Shell script for Sphinx documentation
#

set -e

# You can set these variables from the command line.
SPHINXOPTS=
SPHINXBUILD=sphinx-build
PAPER=
BUILDDIR=_build

# Internal variables.
case "$PAPER" in
    a4)
        PAPEROPT="-D latex_paper_size=a4"
        ;;
    letter)
        PAPEROPT="-D latex_paper_size=letter"
        ;;
    *)
        PAPEROPT=
        ;;
esac
PAPEROPT_a4="-D latex_paper_size=a4"
PAPEROPT_letter="-D latex_paper_size=letter"
ALLSPHINXOPTS="-d ${BUILDDIR}/doctrees ${PAPEROPT} ${SPHINXOPTS} ."

${SPHINXBUILD} -b html ${ALLSPHINXOPTS} ${BUILDDIR}/html
echo
echo "Build finished. The HTML pages are in ${BUILDDIR}/html."
