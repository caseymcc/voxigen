project(opengl_util)

#hunter_add_package(glew)
#find_package(glew CONFIG REQUIRED)
hunter_add_package(glbinding)
find_package(glbinding CONFIG REQUIRED)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

#if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#    find_package(OpenGL)
#endif()

set(opengl_util_sources
    initOpenGL.cpp
    program.cpp
    shader.cpp
    uniform.cpp
    uniformBuffer.cpp
)

set(opengl_util_headers
    opengl_util/initOpenGL.h
    opengl_util/program.h
    opengl_util/shader.h
    opengl_util/uniform.h
    opengl_util/uniformBuffer.h
)

set(opengl_util_libraries
#    glew::glew
    glbinding::glbinding
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(opengl_util_libraries ${opengl_util_libraries}
        opengl32.lib
    )
#elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#    set(opengl_util_libraries ${opengl_util_libraries}
#        ${OPENGL_gl_LIBRARY}
#    )
endif()

if(VOXIGEN_OUTPUT_DIRECTORY_DEBUG)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${VOXIGEN_OUTPUT_DIRECTORY_DEBUG})
endif()
if(VOXIGEN_OUTPUT_DIRECTORY_RELEASE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${VOXIGEN_OUTPUT_DIRECTORY_RELEASE})
endif()

add_library(opengl_util STATIC
    ${opengl_util_sources}
    ${opengl_util_headers}
)

#include_directories(${GLM_INCLUDE_DIRS})

target_link_libraries(opengl_util ${opengl_util_libraries})

#set_target_properties(opengl_util PROPERTIES FOLDER "libraries")

#create_target_launcher(opengl_util
#    RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBS}
#    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
#)

include(GenerateExportHeader)
generate_export_header(opengl_util)

target_include_directories(opengl_util PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(opengl_util PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

