SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS "-O -g -w -Wall -fPIC -fpermissive")


AUX_SOURCE_DIRECTORY(. SRC_LIST)
AUX_SOURCE_DIRECTORY(./inifile SRC_LIST)
AUX_SOURCE_DIRECTORY(./redis SRC_LIST)
AUX_SOURCE_DIRECTORY(./mongodb SRC_LIST)
AUX_SOURCE_DIRECTORY(./zookeeper SRC_LIST)
AUX_SOURCE_DIRECTORY(./protobuf SRC_LIST)


ADD_EXECUTABLE(dataservice_rdd ${SRC_LIST})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

LINK_DIRECTORIES(/usr/lib64 /usr/lib /usr/local/lib)

TARGET_LINK_LIBRARIES(dataservice_rdd log4cxx zookeeper_mt protobuf boost_thread boost_filesystem 
			boost_program_options boost_system boost_regex mongoclient)

INSTALL(TARGETS dataservice_rdd RUNTIME DESTINATION bin)


