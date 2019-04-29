1、开发环境

依赖库: 1、 cossdk 
        2、 cossdk依赖库，包括jsoncpp boost_system boost_thread Poco ssl crypto

推荐开发环境：
visual studio 

2 安装过程
 
2.1、安装cossdk 文档：https://github.com/toranger/cos-cpp-sdk-v5

2.2、安装cmake工具 http://www.cmake.org/download/

2.3、本地编译说明： 修改CMakeList.txt文件中，各依赖库的头文件路径，修改如下语句： INCLUDE_DIRECTORIES,修改各依赖库的lib路径，修改如下语句：link_directories

使用Cmake生成需要的vs工程，编译点播下载库