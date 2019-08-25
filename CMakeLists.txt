cmake_minimum_required(VERSION 3.14)
project(Navigator)

set(CMAKE_CXX_STANDARD 17)

add_executable(Navigator
        main.cpp
        descriptions.cpp
        json.cpp
        requests.cpp
        sphere.cpp
        transport_catalog.cpp
        transport_router.cpp
        utils.cpp
        svg.cpp
        map_renderer.cpp
        sphere_projection.cpp)