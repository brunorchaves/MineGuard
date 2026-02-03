cmake_minimum_required(VERSION 3.16)
project(mineguard_sim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(mineguard_sim
    src/main.cpp
    src/vehicle.cpp
    src/collision.cpp
    src/fleet.cpp
    src/tcp_client.cpp
)

if(UNIX)
    target_link_libraries(mineguard_sim PRIVATE pthread)
endif()
