#!/bin/bash

# Define the filename
filename='reqserver.json'
USER=$1
PASS=$2
HOST=$3
DB=$4

content="{ \"version\" : 2, \"protocol\" : \"mongodb\", \"user\" : \"${USER}\", \"password\" : \"${PASS}\", \"host\" : \"${HOST}\", \"port\" : 27017, \"database\" : \"${DB}\" }"

echo ${content} >> $filename