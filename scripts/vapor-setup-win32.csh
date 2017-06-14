#!/bin/csh -f

if !($?VAPOR3_HOME) then
	echo "VAPOR3_HOME enviroment variable not set"
	exit 1
endif

if !($?PATH) then
    setenv PATH "${VAPOR3_HOME}\bin"
else
    setenv PATH "${VAPOR3_HOME}\bin:$PATH"
endif

