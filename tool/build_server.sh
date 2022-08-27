#!/bin/bash

# Define the filename
filename='reqserver.json'
USER=$ENV_USER
PASS=$ENV_PASS
HOST=$ENV_HOST
DB=$ENV_DB

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

content="{ \"version\" : 2, \"protocol\" : \"mongodb\", \"user\" : \"${USER}\", \"password\" : \"${PASS}\", \"host\" : \"${HOST}\", \"port\" : 27017, \"database\" : \"${DB}\" }"

> reqserver.json
printf "${YELLOW}Creating ${GREEN}reqsever.json\n"
echo ${content} >> $filename
printf "${YELLOW}Creating build directory\n"
cd /delay-tool/reqserver && mkdir -p build
printf "${YELLOW}Building ${CYAN}reqserver...\n ${NC}"
cd /delay-tool/reqserver/build && cmake .. && make
printf "${GREEN}Success!\n"
printf "${YELLOW}Copying config file to bin dir...\n ${NC}"
cp -f /delay-tool/reqserver.json /delay-tool/reqserver/build/bin