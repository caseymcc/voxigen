project(generic)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

option(GENERIC_DISABLE_ATTRIBUTE "Disable support for attributes" OFF)

if(NOT GENERIC_DISABLE_ATTRIBUTE)
    hunter_add_package(Boost)
    find_package(Boost CONFIG)
endif()

set(GENERIC_BOOST_FILESYSTEM 0)
set(GENERIC_STD_FILESYSTEM 1)
set(GENERIC_EXPERIMENTAL_FILESYSTEM 2)
set(GENERIC_USE_FILESYSTEM ${GENERIC_BOOST_FILESYSTEM})

#find what filesystem to use
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-DNOMINMAX)

    if(MSVC_VERSION GREATER_EQUAL 1900)
        set(GENERIC_USE_FILESYSTEM ${GENERIC_EXPERIMENTAL_FILESYSTEM})
        if(MSVC_VERSION GREATER 1900)
            include(CheckCXXCompilerFlag)

            CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
            if(_cpp_17_flag_supported)
                add_compile_options("/std:c++17")
                set(GENERIC_USE_FILESYSTEM ${GENERIC_STD_FILESYSTEM})
            endif()
        endif()
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
        set(GENERIC_USE_FILESYSTEM ${GENERIC_STD_FILESYSTEM})
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(GENERIC_USE_FILESYSTEM ${GENERIC_BOOST_FILESYSTEM})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(GENERIC_USE_FILESYSTEM ${GENERIC_EXPERIMENTAL_FILESYSTEM})
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
        set(GENERIC_USE_FILESYSTEM ${GENERIC_STD_FILESYSTEM})
    endif()
endif()

hunter_add_package(RapidJSON)
find_package(RapidJSON CONFIG REQUIRED)

set(generic_sources
    attribute.cpp
    attributeContainer.cpp
    attributes.cpp
    errorMsg.cpp
    fileIO.cpp
    jsonSerializer.cpp
    log.cpp
)

set(generic_headers
    generic/generic_export.h
    generic/attribute.h
    generic/attributeContainer.h
    generic/attributes.h
    generic/callIf.h
    generic/errorMsg.h
    generic/fileIO.h
    generic/filesystem.h
    generic/jsonSerializer.h
    generic/log.h
    generic/serializer.h
    generic/stdFileIO.h
    generic/types.h
)

set(generic_libraries RapidJSON::rapidjson)
if(NOT GENERIC_DISABLE_ATTRIBUTE)
    set(generic_libraries ${generic_libraries}
        Boost::boost
    )
endif()
if(GENERIC_USE_FILESYSTEM EQUAL ${GENERIC_BOOST_FILESYSTEM})
    hunter_add_package(Boost COMPONENTS system filesystem)
    find_package(Boost CONFIG REQUIRED system filesystem)

    set(generic_libraries ${generic_libraries} Boost::system Boost::filesystem)
endif()

add_library(generic STATIC
        ${generic_sources}
        ${generic_headers}
    )

target_link_libraries(generic ${generic_libraries})
target_include_directories(generic PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if(GENERIC_DISABLE_ATTRIBUTE)
    target_compile_definitions(generic PUBLIC "-DGENERIC_DISABLE_ATTRIBUTE")
endif()

target_compile_definitions(generic PUBLIC "-DGENERIC_BOOST_FILESYSTEM=${GENERIC_BOOST_FILESYSTEM}")
target_compile_definitions(generic PUBLIC "-DGENERIC_STD_FILESYSTEM=${GENERIC_STD_FILESYSTEM}")
target_compile_definitions(generic PUBLIC "-DGENERIC_EXPERIMENTAL_FILESYSTEM=${GENERIC_EXPERIMENTAL_FILESYSTEM}")
target_compile_definitions(generic PUBLIC "-DGENERIC_USE_FILESYSTEM=${GENERIC_USE_FILESYSTEM}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if(GENERIC_USE_FILESYSTEM GREATER ${GENERIC_BOOST_FILESYSTEM})
        target_compile_options(generic PUBLIC "-lstdc++fs")
        target_link_libraries(generic stdc++fs)
    endif()
endif()