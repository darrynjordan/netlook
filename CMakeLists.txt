cmake_minimum_required(VERSION 2.8)
project( cpu_proc )

find_package( OpenCV REQUIRED )
find_library(FFTW_LIBRARY NAMES fftw3)


add_executable( 		cpu_proc 
						main.cpp 
						main.hpp 
						utilities/plotting.cpp
						utilities/plotting.hpp
						utilities/fft.cpp
						utilities/fft.hpp
						utilities/general.cpp
						utilities/general.hpp
						utilities/data.cpp
						utilities/data.hpp
						)


target_link_libraries( 	cpu_proc 
						${OpenCV_LIBS} 
						${FFTW_LIBRARY}
						)
