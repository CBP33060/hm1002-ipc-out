get_filename_component(CURRENT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_FOLDER})
set(CMAKE_C_COMPILER mips-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER mips-linux-gnu-g++)

aux_source_directory(src/  DIR_SRCS)

if("${CONFIG_UCLIBC_BUILD}" STREQUAL "y")
    set(CMAKE_CXX_FLAGS "  -muclibc -Wall -Werror")
    add_definitions(-g  -O2 -std=c++1y  -muclibc -D_INI_CONFIG -D_CRYPTO_ENABLE -Wall -Werror)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/uclibc)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/uclibc)
    link_directories(${70MAI_DIR}/lib/uclibc)
else()
    set(CMAKE_CXX_FLAGS "-Wno-error=deprecated-declarations -Wno-deprecated-declarations ")
    add_definitions(-g -Wl,--whole-archive   -Wl,--no-whole-archive -O2 -std=c++1y -Wdeprecated-declarations -D_INI_CONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${APP_DIR}/bin/glibc)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${70MAI_DIR}/lib/glibc)
    link_directories(${70MAI_DIR}/lib/glibc)
endif()

include_directories(${70MAI_DIR}/platform-osp)
include_directories(${70MAI_DIR}/platform-osp/libosp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcomproxy/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcore/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libcrypt/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${70MAI_DIR}/include)
# include_directories(${70MAI_DIR}/include/ImgRecognition)
# include_directories(${70MAI_DIR}/include/ImgRecognition/stb)

add_executable(${CURRENT_FOLDER}  ${DIR_SRCS})
target_link_libraries(${CURRENT_FOLDER} pthread json uuid dl rpc comproxy core crypt osp)
target_link_libraries(${CURRENT_FOLDER} )

install(TARGETS ${CURRENT_FOLDER} DESTINATION bin)
