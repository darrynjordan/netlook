cmake_minimum_required(VERSION 2.8)
project(netlook)

#set (CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
find_library(FFTW_LIBRARY NAMES fftw3)
find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)

#message(${CMAKE_CXX_FLAGS_RELEASE})

file(GLOB CPPFILES "source/*.cpp" "source/*.h")
add_executable(netlook ${CPPFILES})

target_link_libraries(netlook ${OpenCV_LIBS} ${FFTW_LIBRARY} ${Boost_LIBRARIES} -lfftw3_threads -pg)
