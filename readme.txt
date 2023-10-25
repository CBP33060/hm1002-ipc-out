1、全编译app,编译glibc还是uclibc版本的程序，需要修改app目录下Makefile里面CONFIG_UCLIBC_BUILD这个变量，现在这个版本默认是编译uclibc
    make

2、单独编译一个模块
    make SUB=libcomproxy

3、编译在x86上面运行的程序
    make CPU=x86
    有些模块是没有x86的库的，所以在默认全编译的时候不要添加到APPS里面，除非把库补齐

4、添加模块需要在app目录下面的Makefile里面APPS 添加模块名字，模块编译使用的CMake，可执行程序的CMakeLists.txt 可以参考ai_manage_susbsystem编写，动态库的参考libcomproxy

5、编译third-party,除非添加新的模块，默认是不需要编译third-party的，如果要单独编译，需要进入对应模块的目录，命令同下
    cd third-party
    ./autobuild.sh
    # 安装
    ./autobuild.sh install

6、Mick 是小米sdk，zeratul_sdk是君正提供的sdk，升级时候覆盖即可

7、添加内核模块编译lkm目录
    全编译编译流程：
        cd lkm
        make
    单独编译：
        cd lkm
        make SUB=dir