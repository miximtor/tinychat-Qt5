cmake_minimum_required(VERSION 3.14.2)
project(tinychat-Qt5)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${PROJECT_SOURCE_DIR}/ui )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake third_party/tinyrpc/cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

find_package(Boost 1.69 COMPONENTS system coroutine thread REQUIRED)

add_subdirectory(third_party/tinyrpc)

add_definitions(-DBOOST_COROUTINES_NO_DEPRECATION_WARNING)
if (WIN32)
    add_definitions(-DTINYCHAT_WIN32)
    if (MSVC)
        add_definitions(-D_WIN32_WINNT=0x0601)
        set(CompilerFlags
                CMAKE_CXX_FLAGS_DEBUG
                CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL
                CMAKE_CXX_FLAGS_RELWITHDEBINFO
                CMAKE_C_FLAGS_DEBUG
                CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL
                CMAKE_C_FLAGS_RELWITHDEBINFO
                )

        foreach (CompilerFlag ${CompilerFlags})
            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
        endforeach ()

    endif ()
elseif (UNIX)
    add_definitions(-DTINYCHAT_UNIX)
endif ()

#build
include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/third_party/tinyrpc/baseproto)
message("${Boost_INCLUDE_DIRS}")
include_directories(third_party/tinyrpc/include)
PROTOBUF_GENERATE_CPP(TCQT5_RPC_PROTO_SRC TCQT5_RPC_PROTO_HDR ${PROJECT_SOURCE_DIR}/protocol/chat.proto)
set_property(SOURCE  ${TCQT5_RPC_PROTO_SRC} ${TCQT5_RPC_PROTO_HDR} PROPERTY SKIP_AUTOGEN ON)

file(GLOB UI_FILES ${PROJECT_SOURCE_DIR}/ui/*.ui)
file(GLOB UI_CLASS_HDR ${PROJECT_SOURCE_DIR}/include/tc-qt5/ui/*.hpp)
file(GLOB NET_CLASS_HDR ${PROJECT_SOURCE_DIR}/include/tc-qt5/net/*.hpp)
file(GLOB UI_CLASS_SRC ${PROJECT_SOURCE_DIR}/src/ui/*.cpp)
file(GLOB NET_CLASS_SRC ${PROJECT_SOURCE_DIR}/src/net/*.cpp)

add_executable(tinychat-qt5
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${UI_FILES}
        ${UI_CLASS_HDR}
        ${UI_CLASS_SRC}
        ${NET_CLASS_HDR}
        ${NET_CLASS_SRC}
        ${TCQT5_RPC_PROTO_SRC}
        ${TCQT5_RPC_PROTO_HDR}
        ${PROTO_RPC_SERVICE_PTL_SRC}
        ${PROTO_RPC_SERVICE_PTL_HDRS}
        )
#Qt libraries
target_link_libraries(tinychat-qt5 Qt5::Core Qt5::Widgets)

#Boost libraries
target_link_libraries(tinychat-qt5 Boost::system Boost::coroutine Boost::thread)

#Protobuf libraris
target_link_libraries(tinychat-qt5 protobuf::libprotobuf)

#System libraries
if (WIN32)
    target_link_libraries(tinychat-qt5 ws2_32 wsock32)
endif ()
target_link_libraries(tinychat-qt5 Threads::Threads)


