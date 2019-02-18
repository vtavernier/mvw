# Find package for ZeroMQ
include(LibFindMacros)

# Use pkg-config to get hints
libfind_pkg_detect(zeromq libzmq
    FIND_PATH zmq.h
    FIND_LIBRARY zmq)

# Set include dir and libraries variables
set(ZeroMQ_PROCESS_INCLUDES zeromq_INCLUDE_DIR)
set(ZeroMQ_PROCESS_LIBS zeromq_LIBRARY)
libfind_process(ZeroMQ)
