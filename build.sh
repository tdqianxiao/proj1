#!/bin/bash
if [ ! -d "./build" ];then
	mkdir ./build
	cd ./build 
	cmake ../
else
	echo "文件夹存在,已删除"
	rm -rf ./build 
	mkdir ./build
	cd ./build
	cmake ../
fi
