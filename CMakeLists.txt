CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(AnotherInfiniteNightattheOrbitalPhoneBank)

set(SD2_INCLUDE_DIR "/usr/local/Cellar/sdl2/2.0.8/include/SDL2/")

include_directories(${SD2_INCLUDE_DIR})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(main ${DIR_SRCS} PhoneMode.cpp PhoneMode.hpp)