# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify architecture (optional, only determines target suffix)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

#set(CMAKE_CXX_FLAGS "... --sysroot=/mnt/rock64/ ")
#set(CMAKE_CXX_LINK_FLAGS "... --sysroot=/mnt/rock64/ ")

#SET(CMAKE_LIBRARY_PATH /mnt/rock64/usr/local/lib)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH /mnt/rock64 )
SET(CMAKE_SYSROOT /mnt/rock64 )
#SET(CMAKE_PREFIX_PATH /mnt/rock64/ )
# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_definitions(-Wall)
# end of the file