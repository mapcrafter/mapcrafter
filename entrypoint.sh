#!/bin/sh

USER_ID=${LOCAL_USER_ID:-1000}
echo "Running as user $USER_ID"

useradd --shell /bin/sh -u $USER_ID -o -d /home/user -m user
mkdir -p /home/user && chown -R user:user /home/user

su -c "/usr/local/bin/mapcrafter $*" user
