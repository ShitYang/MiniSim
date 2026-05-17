# MiniSim
## 概述
MiniSim是一个缩小版本的Afsim（Afsim是军事仿真引擎，可以根据脚本文件运行仿真场景）
## 系统环境
1. Windows11
2. x86-64
## 前置条件
1. Qt-5.12.5-x64
2. CMake-4.0
3. Ninja
4. MSVC-14.29.30133
只需要根据我配置的CMakePresets.json，选择Ninja-MSVC进行配置。
## 运行须知
编译完成后，直接cmd运行build/bin下面的warlock.exe，后面跟想定参数，例如“warlock.exe projects/helloworld.txt”，其中helloworld.txt是想定脚本。
## 注意
源码版本并没有瓦片，只有网格地图，因为瓦片地图太大了。但是我放了一个Release版本，可以直接运行。

实现见 https://shityang.github.io/ShowSkills/%E6%A0%85%E6%A0%BC%E5%8C%96%E4%B8%8E%E7%9F%A2%E9%87%8F%E6%B7%B7%E5%90%88%E6%96%B9%E6%A1%88%E7%9A%84%E5%9C%B0%E5%9B%BE%E5%BC%95%E6%93%8E.html 