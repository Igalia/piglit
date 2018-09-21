#!/bin/bash
#
# This script searches artifact name in previous jobs and downloads it.
#
# Run:
#
# $ ./download-artifact.sh <private token> <artifact name> <output file>

export LC_ALL=C

if [ $# != 3 ] ; then
    echo "Usage: $0 <private token> <artifact name> <output file>"
    exit 0
fi

ARTIFACT_TOKEN=$1
ARTIFACT_NAME=$2
ARTIFACT_OUTPUT=$3

MAX_PAGES=11
PAGE=0
SERVER_URL=${CI_PROJECT_URL%%/${CI_PROJECT_PATH}}

echo -n "Searching job ..."

while [ -z "${JOB_ID}" ] && [ "$((PAGE++))" != "${MAX_PAGES}" ]; do
    JOB_ID=`curl -s --header "PRIVATE-TOKEN: ${ARTIFACT_TOKEN}" "${SERVER_URL}/api/v4/projects/${CI_PROJECT_ID}/jobs?page=${PAGE}&scope=success" | jq ".[] | select(.artifacts_file.filename==\"${ARTIFACT_NAME}\")" | jq .id | head -n 1`
done

if [ -z "${JOB_ID}" ]; then
    echo "Not found!"
    exit 1
else
    echo " found at job #${JOB_ID}"
fi

echo "Downloading \"${ARTIFACT_NAME}\" artifact ..."
curl -s --location --header "PRIVATE-TOKEN: ${ARTIFACT_TOKEN}" "${SERVER_URL}/api/v4/projects/${CI_PROJECT_ID}/jobs/${JOB_ID}/artifacts" -o ${ARTIFACT_OUTPUT}

