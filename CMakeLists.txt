cmake_minimum_required(VERSION 2.8)
project(netlook)

find_package(OpenCV REQUIRED)
find_library(FFTW_LIBRARY NAMES fftw3)

#message(${CMAKE_CXX_FLAGS_RELEASE})

file(GLOB CPPFILES "source/*.cpp" "source/*.h")
add_executable(netlook ${CPPFILES})

target_link_libraries(netlook ${OpenCV_LIBS} ${FFTW_LIBRARY} -lfftw3_threads -pg -pthread)
