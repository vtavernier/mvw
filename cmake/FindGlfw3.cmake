# Find package for libepoxy
include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_detect(Glfw3 epoxy
	FIND_PATH GLFW/glfw3.h
	FIND_LIBRARY glfw)

# Set include dir and libraries variables
set(Glfw3_PROCESS_INCLUDES Glfw3_INCLUDE_DIR)
set(Glfw3_PROCESS_LIBS Glfw3_LIBRARY)
libfind_process(Glfw3)

