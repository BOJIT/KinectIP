cmake_minimum_required( VERSION 3.0 )

project( KinectIP )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin/${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_SYSTEM_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)

# include files
include_directories( ./include ./src )

# link files
#link_directories( /usr/local/lib/NDI )
#link_directories( /mnt/rock64/usr/local/lib/NDI )
#link_directories( /mnt/rock64/usr/lib/aarch64-linux-gnu )
#link_directories( /mnt/rock64/lib/aarch64-linux-gnu )

# target
add_executable( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} ./src/main.cpp )

# NDI_Lib
find_library(NDI NAMES "ndi" PATHS ${CMAKE_FIND_ROOT_PATH}/usr/local/lib/NDI)
find_library(AVAHI_COMMON NAMES "libavahi-common.so.3" PATHS ${CMAKE_FIND_ROOT_PATH}/usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
find_library(AVAHI_CLIENT NAMES "libavahi-client.so.3" PATHS ${CMAKE_FIND_ROOT_PATH}/usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
find_library(DBUS NAMES "libdbus-1.so.3" PATHS ${CMAKE_FIND_ROOT_PATH}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)
find_library(LIBC NAMES "libc.so.6" PATHS ${CMAKE_FIND_ROOT_PATH}/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu)

#link_directories(${AVAHI_CLIENT}, ${DBUS})

target_link_libraries( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} ${NDI} ${AVAHI_COMMON} ${AVAHI_CLIENT} ${DBUS} ${LIBC})