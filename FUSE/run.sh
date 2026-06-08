#!/bin/bash
echo if not compiling please run - sudo apt install libfuse3-dev g++ pkg-config
g++ -Wall my_fuse_fs.cpp `pkg-config fuse3 --cflags --libs` -o my_fs
g++ -Wall manager.cpp -o manager
mkdir -p ./mount_point
fusermount -u ./mount_point 2>/dev/null
./my_fs ./mount_point &
sleep 2
./manager
fusermount -u ./mount_point
