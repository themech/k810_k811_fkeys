#!/bin/bash
if [ -z "$1" ];
then
    /opt/k81x/k81x-fkeys -s on
else
    /opt/k81x/k81x-fkeys -s -u $1 on
fi
