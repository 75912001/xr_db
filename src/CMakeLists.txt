CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
SET(CMAKE_CXX_COMPILER g++ )

ADD_DEFINITIONS("-Wall -std=c++0x -g -MMD -fPIC -D__STDC_FORMAT_MACROS -O0 -Wl,-rpath,${XR_PATH},${XR_SERVER_PATH}")

INCLUDE_DIRECTORIES(
		/usr/include/libxml2/
		/usr/local/protobuf/include
		$ENV{XR_PATH}/include
		$ENV{XR_SERVER_PATH}/include
		$ENV{XR_SERVER_PATH}/include_mysql
		../../xr_share
		../include
)

LINK_DIRECTORIES(
		/usr/local/lib
		/usr/lib64/mysql
		$ENV{XR_PATH}
)
LINK_LIBRARIES(
		dl
		rt
		xr
		mysqlclient
		protobuf
)

AUX_SOURCE_DIRECTORY(. src_list)
AUX_SOURCE_DIRECTORY(../share/protocol-src src_list)
AUX_SOURCE_DIRECTORY(../share/ src_list)
AUX_SOURCE_DIRECTORY($ENV{XR_SERVER_PATH}/src_mysql src_list)

SET(LIBRARY_OUTPUT_PATH $ENV{XR_SERVER_PATH})
ADD_LIBRARY(db SHARED ${src_list})

