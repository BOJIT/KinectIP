cmake_minimum_required( VERSION 3.0 )

project( KinectIP )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../bin/${CMAKE_SYSTEM_PROCESSOR}_${CMAKE_SYSTEM_NAME})

# include files
include_directories( ./include ./src ./libs/Logger/include ./libs/Randomize/include )

# target
add_executable( KinectIP.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} ./src/main.cpp ./src/game_engine.cpp ./src/game_interface.cpp )

# 3rd party libs
add_subdirectory( ./libs/Logger )
target_link_libraries( KinectIP.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} logger )

# 3rd party libs
add_subdirectory( ./libs/Randomize )
target_link_libraries( KinectIP.${CMAKE_SYSTEM_PROCESSOR}.${CMAKE_SYSTEM_NAME} randomize )