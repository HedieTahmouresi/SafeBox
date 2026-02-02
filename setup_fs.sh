#!/bin/bash
set -e

ROOTFS_DIR="./rootfs"

if [ -d "$ROOTFS_DIR" ]; then
    echo "[SETUP] RootFS already exists at $ROOTFS_DIR"
    exit 0
fi

echo "[SETUP] Downloading Alpine Linux RootFS..."
mkdir -p "$ROOTFS_DIR"

wget -qO alpine.tar.gz https://dl-cdn.alpinelinux.org/alpine/v3.18/releases/x86_64/alpine-minirootfs-3.18.4-x86_64.tar.gz

echo "[SETUP] Extracting..."
tar -xzf alpine.tar.gz -C "$ROOTFS_DIR"
rm alpine.tar.gz

echo "[SETUP] RootFS ready."