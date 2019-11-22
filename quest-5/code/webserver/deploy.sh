#!/bin/bash

if [ -z "$1" ];
then
    echo "IP address required"
    exit 1
fi

IP_ADDRESS="$1"
PKG="security-central.zip"
CREATED=0

if [ ! -f "$PKG" ];
then
    echo "Creating archive."
    ./package.sh
    CREATED=1
fi

STAGE_DIR="/tmp/staging"

if [ -d "${STAGE_DIR}" ];
then
    rm -r "${STAGE_DIR}"
fi

mkdir -p "$STAGE_DIR"
unzip "$PKG" -d "$STAGE_DIR"
rsync -a "${STAGE_DIR}/security-central/" pi@${IP_ADDRESS}:~/security-central
ssh pi@${IP_ADDRESS} "sudo systemctl restart ec444-quest5.service"

rm -r "${STAGE_DIR}"

if [ "${CREATED}" == "1" ];
then
    rm "${PKG}"
fi
