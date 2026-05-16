# 需求
@warlock/wkf/MercatorMapWidget.hpp @warlock/wkf/MercatorMapWidget.cpp 
这已经有一个墨卡托投影的栅格引擎了，可以根据缩放，显示不同层级的瓦片。
然后，我现在需要引入矢量数据，根据自定义的json文件，绘制地标、道路、区域，即点、线、面等。(我需要多边形的城市区域,不光是矩形)
这个矢量数据需要和原来的栅格引擎一样，不同层级显示不同的细节（LOD）。


# 已有条件
Qt、QOpengl是已经配置好的，你不需要引入什么别的依赖

<!-- # 运行并测试 -->
<!-- 不需要你运行并测试！！ -->
<!-- cmake构建目录在 "${workspaceFolder}/build",
cmake路径在 "D:\\CMake\\CMake-4.0\\bin\\cmake.exe",

在build目录下使用“D:\CMake\CMake-4.0\bin\cmake.exe .. ”进行configure
在build目录下使用“D:\CMake\CMake-4.0\bin\cmake.exe --build . --target install -j 16”进行编译

你可以在build/bin下面，运行"warlock.exe D:/WorkSpace/MiniSim/build/bin/projects/_helloworld.txt"，进行测试 -->

# 约束
1. 你只能更改@warlock/wkf下面的文件
<!-- 2. 每次编译完，需要把@build/bin/Debug的文件，都拷贝到@build/bin下面 -->

2. 还要生成 地图矢量json文件，包含我要求的LOD、地标、道路、区域，即点、线、面等(城市名为中文).(我需要多边形的城市区域,不光是矩形)
我需要真实的区域数据,不是随机的多边形.

3. 不需要你运行并测试！！

4. vector_test.json文件中,至少有1000条测试数据,并且显示城市的名称