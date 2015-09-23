cmake_minimum_required (VERSION 2.6)

project(dataservice_rdd)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

SET(CMAKE_CXX_COMPILER "g++")

SET(CMAKE_CXX_FLAGS "-O -g -w -Wall -fPIC")

ADD_SUBDIRECTORY(src bin)

SET(CMAKE_INSTALL_PREFIX ./debug)

INSTALL(PROGRAMS dataservice.sh DESTINATION bin)

INSTALL(FILES COPYRIGHT README DESTINATION share/doc/dataservice)

INSTALL(DIRECTORY doc/ DESTINATION share/doc/dataservice)

