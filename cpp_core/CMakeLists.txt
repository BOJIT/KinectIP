cmake_minimum_required( VERSION 3.0 )

project( KinectIP )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin/${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_SYSTEM_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)

# include files
include_directories( ./include ./src ./libs/Logger/include ./libs/Randomize/include )

# link files
link_directories( /usr/local/lib/NDI )
link_directories( /opt/rock64_sysroot/usr/local/lib/NDI )

# target
add_executable( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} ./src/main.cpp ./src/game_engine.cpp ./src/game_interface.cpp )

# 3rd party libs
add_subdirectory( ./libs/Logger )
target_link_libraries( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} logger )

# 3rd party libs
add_subdirectory( ./libs/Randomize )
target_link_libraries( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} randomize )

# NDI_Lib
#find_path(NDI_Path libndi.so)
#message(${NDI_Path})
#find_library( NDI ndi )
#add_library(ndi SHARED IMPORTED)
#set_property(TARGET ndi PROPERTY IMPORTED_LOCATION "/usr/local/lib/NDI/libndi.so")
target_link_libraries( ${CMAKE_PROJECT_NAME}.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} ndi )