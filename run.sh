#! /bin/sh

cmake -S . -B out/build
cd out/build; make FuseFileSystemBoC -j 4; cd ../..;pwd;
cd out/build; ./FuseFileSystemBoC
