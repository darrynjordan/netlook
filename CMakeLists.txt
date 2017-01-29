cmake_minimum_required(VERSION 2.8)
project( cpu_proc )

find_package( OpenCV REQUIRED )
find_library(FFTW_LIBRARY NAMES fftw3)

message(${CMAKE_CXX_FLAGS_RELEASE})

add_executable( 		cpu_proc 
						main.cpp 
						main.hpp 
						source/plotting.cpp
						source/plotting.hpp
						source/fft.cpp
						source/fft.hpp
						source/general.cpp
						source/general.hpp
						source/data.cpp
						source/data.hpp
						)


target_link_libraries( 	cpu_proc 
						${OpenCV_LIBS} 
						${FFTW_LIBRARY}
						-lfftw3_threads
						)
