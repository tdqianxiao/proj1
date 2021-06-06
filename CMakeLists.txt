cmake_minimum_required(VERSION 2.8)
project(tadpole)

#cmake方法
include (utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

#包含目录
include_directories(.)
include_directories(./3rd/include/lua54)
include_directories(./3rd/include/yaml)
include_directories(./3rd/include/co)


#源文件,用于生成动态库
set(LIB_SRC
    src/utils.cc
	src/luaState.cc
	src/luaRegister.cc
	src/log.cc
    src/config.cc
    src/byteArray.cc
    src/mutex.cc
    src/thread.cc
    )

#添加动态库
add_library(tadpole SHARED ${LIB_SRC})

#链接动态库目录
link_directories(${PROJECT_SOURCE_DIR}/3rd/lib)
#链接动态库
link_libraries(lua)
link_libraries(yaml-cpp)
link_libraries(colib)
#重定义文件宏
force_redefine_file_macro_for_sources(tadpole)
#查找动态库
#find_library(PTHREAD pthread)

#需要库文件
set(LIBS
    tadpole
	lua
    yaml-cpp
    colib
    pthread
    )


tadpole_add_executable(xxx "tests/test.cc" tadpole "${LIBS}")
tadpole_add_executable(test_co "tests/test_co.cc" tadpole "${LIBS}")
tadpole_add_executable(test_yaml "tests/test_yaml.cc" tadpole "${LIBS}")
tadpole_add_executable(test_byte "tests/test_byte.cc" tadpole "${LIBS}")
tadpole_add_executable(test_thread "tests/test_thread.cc" tadpole "${LIBS}")

#可执行文件输出路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#库文件输出路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
