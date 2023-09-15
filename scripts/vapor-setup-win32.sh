#!/bin/sh

if [ -z "${VAPOR3_HOME}" ]
then
	echo "VAPOR3_HOME enviroment variable not set"
	exit 1
fi

if [ -z "${PATH}" ]
then
    PATH="${VAPOR3_HOME}\\bin"; export PATH
else
    PATH="${VAPOR3_HOME}\\bin:$PATH"; export PATH
fi


