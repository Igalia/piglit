#!/bin/sh
#
# Hacked-together script for building the results summaries on
#  http://people.freedesktop.org/~nh/piglit/results/
# The script is based on a database of results in the ${databasedir}.
# The ${classesdir} is supposed to contain one file classes which
# is simply a list of class names, and one file for each class which
# contains names of results for that class, in chronological order.
#
# So for example, ${classesdir} might contain the files:
#
# ${classesdir}/classes:
#   R300
#   R500
#
# ${classesdir}/R300:
#   R300ND-2008-06-12
#   R300ND-2008-06-13-2
#   R300ND-2008-06-30
#   R300ND-2008-07-04
#
# And similarly, ${classesdir}/R500.
# Corresponding results of test runs must exist in ${databasedir}


### Configuration
piglitdir=~/dev/xorg/piglit/repo
resultsdir=../html/piglit/results
classesdir=./classes
databasedir=./database


### Script starts here
SUMMARY=${piglitdir}/piglit-summary-html.py

mkdir -p ${resultsdir}
classes=$(cat ${classesdir}/classes)

echo "[" > tmpresults.all

latest=
for class in ${classes}; do
	echo "Building report for class ${class}"
	echo "['${databasedir}/$(tail -n 1 ${classesdir}/${class})'," >> tmpresults.all
	echo "{'name': '${class}', 'href': '../${class}/index.html'}]," >> tmpresults.all

	classlist=$(tail -n 4 ${classesdir}/${class} | sed s@^@${databasedir}/@)
	${SUMMARY} -o ${resultsdir}/${class} ${classlist}
done

echo "Building report across classes"
echo "]" >> tmpresults.all
${SUMMARY} -o -l tmpresults.all ${resultsdir}/all
rm tmpresults.all
