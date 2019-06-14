# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify architecture (optional, only determines target suffix)
SET(CMAKE_SYSTEM_PROCESSOR aarch64)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

#set(CMAKE_CXX_FLAGS "... --sysroot=/opt/rock64_sysroot/ ")
#set(CMAKE_CXX_LINK_FLAGS "... --sysroot=/opt/rock64_sysroot/ ")

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH /opt/rock64_sysroot)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# end of the file