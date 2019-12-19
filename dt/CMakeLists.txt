cmake_minimum_required(VERSION 3.3)

project(dt)

set(dt_headers
    dt/delaunay.h
    dt/edge.h
    dt/numeric.h
    dt/triangle.h
    dt/vector2.h
)

set(dt_sources
    delaunay.cpp
    edge.cpp
    numeric.cpp
    triangle.cpp
    vector2.cpp
)

add_library(dt STATIC
    ${dt_sources}
    ${dt_headers}
)

target_link_libraries(dt glm)

target_include_directories(dt PUBLIC ${CMAKE_CURRENT_LIST_DIR})
