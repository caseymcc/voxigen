project(opengl_util)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

set(opengl_util_sources
    initGlew.cpp
    program.cpp
    shader.cpp
    uniform.cpp
    uniformBuffer.cpp
)

set(opengl_util_headers
    opengl_util/initGlew.h
    opengl_util/program.h
    opengl_util/shader.h
    opengl_util/uniform.h
    opengl_util/uniformBuffer.h
)

set(opengl_util_libraries
    opengl32.lib
    glew::glew
)

add_library(opengl_util SHARED
    ${opengl_util_sources}
    ${opengl_util_headers}
)

include_directories(${GLM_INCLUDE_DIRS})

target_link_libraries(opengl_util ${opengl_util_libraries})

set_target_properties(opengl_util PROPERTIES FOLDER "libraries")

create_target_launcher(opengl_util
    RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBS}
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

include(GenerateExportHeader)
generate_export_header(opengl_util)

target_include_directories(opengl_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(opengl_util PUBLIC ${CMAKE_CURRENT_BINARY_DIR})