cmake_minimum_required(VERSION 2.6)
project(LoadLayoutArchive)

include_directories(
	/boot/home/config/develop/headers/alm
	/boot/home/config/develop/headers/linearspec
	/boot/home/config/develop/headers/ale
	/boot/system/develop/headers/alm
	/boot/system/develop/headers/linearspec
	/boot/system/develop/headers/ale
	/boot/system/develop/headers/private/shared
)

link_directories(
	/boot/home/config/lib
)

add_executable(LoadLayoutArchive LoadLayoutArchive.cpp)
target_link_libraries(LoadLayoutArchive ${CORELIBS} be alm tracker ale)
