#!/bin/bash


uid=1000
gid=1000
me="$(pwd)"
wd="${me}/bin"
remount=""

if [ ! -z "$(mount | grep tmpfs | grep ${me})" ]; then
  remount=",remount"
fi
if [ -d ${wd} ]; then rm -rf ${wd}/*; fi
mkdir -p ${wd}
mount -t tmpfs -o size=1G,uid=${uid},gid=${gid}${remount} tmpfs ${wd}
cmake .
chown -R ${uid}:${gid} ${wd}
