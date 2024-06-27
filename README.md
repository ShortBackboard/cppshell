# cppshell
使用C++，结合C++标准库和 Linux 的系统调用，完成的一个linux下的shell程序

## 项目构建

1.进入项目
cd cppshell

2.创建build文件夹并进入
mkdir build && cd build

3.构建项目并编译
cmake .. && make

4.运行项目
./cppshell

## 运行简单的shell脚本
sh test.sh


## 完成功能
1. shell 程序能够提供命令的输入，执行并显示执行结果的功能。
2. shell 程序能够提供可 shell 编程的功能，能够执行简单的 shell 脚本。
3. shell 程序能够提供 I/O 重定向和管道的功能。