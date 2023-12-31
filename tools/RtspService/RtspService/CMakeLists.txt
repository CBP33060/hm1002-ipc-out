get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_FOLDER})
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/  DIR_SRCS)

if("${TARGET_CPU}"  STREQUAL "x86")
    set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
    add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -D_INI_CONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/x86)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/x86)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/x86)
    link_directories(${70MAI_DIR}/lib/x86)
else()
    if("${CONFIG_UCLIBC_BUILD}" STREQUAL "y")
        set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations -muclibc ")
        add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -muclibc -D_INI_CONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin/uclibc/)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc/)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc/)
        link_directories(${70MAI_DIR}/zeratul-sdk/lib/uclibc)
        link_directories(${70MAI_DIR}/lib/uclibc)
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/uclibc)
    else()
        set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
        add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -D_INI_CONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin/glibc/)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc/)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc/)
        link_directories(${70MAI_DIR}/zeratul-sdk/lib/glibc)
        link_directories(${70MAI_DIR}/lib/glibc)
    endif()
endif()

add_definitions(-DSENSOR_${SENSOR}=1)

include_directories(${70MAI_DIR}/platform-osp)
include_directories(${70MAI_DIR}/platform-osp/libosp/include)
include_directories(${70MAI_DIR}/libcore/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${70MAI_DIR}/include)
include_directories(${70MAI_DIR}/zeratul-sdk/include)

add_executable(${CURRENT_FOLDER}  ${DIR_SRCS})

target_link_libraries(${CURRENT_FOLDER} pthread rt)
# RTSPComponent库
target_link_libraries(${CURRENT_FOLDER} RTSPComponent)
# 君正SDK需要使用的库
target_link_libraries(${CURRENT_FOLDER} alog imp)

install(TARGETS ${CURRENT_FOLDER} DESTINATION bin)
