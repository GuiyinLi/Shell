###
 # @Author: Ken Kaneki
 # @Date: 2021-12-16 06:54:13
 # @LastEditTime: 2021-12-16 07:00:08
 # @Description: README
###

#! /bin/bash
#进入执行程序目录
cd bin
#编译工程
gcc -g ../source/*.c -I ../include -lreadline -o Shell
#执行可执行程序
./Shell
