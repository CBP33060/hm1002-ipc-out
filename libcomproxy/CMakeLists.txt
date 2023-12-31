get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(comproxy)
if("${TARGET_CPU}"  STREQUAL "x86")
    set(CMAKE_C_COMPILER gcc)
    set(CMAKE_CXX_COMPILER g++)
else()
    set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)
endif()


aux_source_directory(src/  DIR_SRCS)

if("${TARGET_CPU}"  STREQUAL "x86")
    set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
    add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -D_INI_CONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/x86)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/x86)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/x86)
    link_directories(${70MAI_DIR}/lib/x86)
else()
    if("${CONFIG_UCLIBC_BUILD}" STREQUAL "y")
        set(CMAKE_CXX_FLAGS "-Wall -Werror -muclibc")
        add_definitions(-g -O1 -std=c++1y -Wall -Werror -muclibc -D_INI_CONFIG -D_CRYPTO_ENABLE)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/uclibc)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
        link_directories(${70MAI_DIR}/lib/uclibc)
    else()
        set(CMAKE_CXX_FLAGS " ")
        add_definitions(-g  -D_INI_CONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/glibc)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
        link_directories(${70MAI_DIR}/lib/glibc)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcore/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${70MAI_DIR}/platform-osp)
include_directories(${70MAI_DIR}/platform-osp/libosp/include)
include_directories(${70MAI_DIR}/include)

add_library(comproxy SHARED  ${DIR_SRCS})
target_link_libraries(comproxy rpc core rtp)

install(TARGETS comproxy DESTINATION lib)
