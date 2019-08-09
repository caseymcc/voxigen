cmake_minimum_required(VERSION 3.3)
project(imglib)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

option(IMGLIB_USE_COMPILETIME_TYPE "Turn on/off support for compile time type informat" ON)
option(IMGLIB_USE_PNG "Turn on/off support for PNG" ON)
option(IMGLIB_USE_EIGEN "Turn on/off support for eigen library" ON)
option(IMGLIB_USE_OPENCL "Turn on/off support for OpenCl" ON)
option(IMGLIB_USE_OPENCL_CPP "Turn on/off support for OpenCl CPP classes" ON)
option(IMGLIB_USE_CUDA "Turn on/off support for CUDA" OFF)
option(IMGLIB_BUILD_TEST "Turns on test application" ON)

set(IMGLIB_BOOST_FILESYSTEM 0)
set(IMGLIB_STD_FILESYSTEM 1)
set(IMGLIB_EXPERIMENTAL_FILESYSTEM 2)
set(IMGLIB_USE_FILESYSTEM ${IMGLIB_BOOST_FILESYSTEM})

#find what filesystem to use
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_definitions(-DNOMINMAX)

    if(MSVC_VERSION GREATER_EQUAL 1900)
        set(IMGLIB_USE_FILESYSTEM ${IMGLIB_EXPERIMENTAL_FILESYSTEM})
        if(MSVC_VERSION GREATER 1900)
            include(CheckCXXCompilerFlag)

            CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_17_flag_supported)
            if(_cpp_17_flag_supported)
                add_compile_options("/std:c++17")
                set(IMGLIB_USE_FILESYSTEM ${IMGLIB_STD_FILESYSTEM})
            endif()
        endif()
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
        set(IMGLIB_USE_FILESYSTEM ${IMGLIB_STD_FILESYSTEM})
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    set(IMGLIB_USE_FILESYSTEM ${IMGLIB_BOOST_FILESYSTEM})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(IMGLIB_USE_FILESYSTEM ${IMGLIB_EXPERIMENTAL_FILESYSTEM})
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
        set(IMGLIB_USE_FILESYSTEM ${IMGLIB_STD_FILESYSTEM})
    endif()
endif()

set(imglib_headers
    imglib/colorConversion.h
    imglib/copyTo.h
	imglib/draw.h
	imglib/gaussian.h
    imglib/image.h
    imglib/imageIo.h
    imglib/match.h
    imglib/ppm.h
    imglib/simpleImage.h
    imglib/transform.h
)

set(imglib_sources
    colorConversion.cpp
    image.cpp
    match.cpp
    ppm.cpp
    simpleImage.cpp
    transform.cpp
)

set(imglib_libraries )

if(IMGLIB_USE_FILESYSTEM EQUAL ${IMGLIB_BOOST_FILESYSTEM})
    hunter_add_package(Boost COMPONENTS system filesystem)
    find_package(Boost CONFIG REQUIRED system filesystem)

    set(imglib_libraries ${imglib_libraries} Boost::system Boost::filesystem)
endif()


if(IMGLIB_USE_COMPILETIME_TYPE)
    hunter_add_package(ctti)
    find_package(ctti REQUIRED CONFIG)

    set(imglib_libraries ${imglib_libraries} ctti)
endif()

if(IMGLIB_USE_EIGEN)
    hunter_add_package(Eigen)
    find_package(Eigen3 REQUIRED)
#    add_definitions(-DIMGLIB_EIGEN)

    set(imglib_libraries ${imglib_libraries} Eigen3::Eigen)
endif()

message(STATUS "IMGLIB_USE_PNG ${IMGLIB_USE_PNG}")
if(IMGLIB_USE_PNG)
    hunter_add_package(PNG)
    find_package(PNG CONFIG REQUIRED)
#    add_definitions(-DIMGLIB_PNG)

    set(imglib_headers ${imglib_headers}
        imglib/png.h
    )

    set(imglib_sources ${imglib_sources}
        png.cpp
    )

    set(imglib_libraries ${imglib_libraries} PNG::png)
endif()

if(IMGLIB_USE_OPENCL)
    hunter_add_package(OpenCL)
    find_package(OpenCL CONFIG REQUIRED)
#    add_definitions(-DIMGLIB_OPENCL)
#    if(IMGLIB_USE_OPENCL_CPP)
#        add_definitions(-DIMGLIB_OPENCL_CPP)
#    endif()

    set(imglib_headers ${imglib_headers}
        imglib/colorConversion_cl.h
        imglib/kernels_cl.h
        imglib/openCLImage.h
        imglib/openCLImageBuffer.h
        imglib/operators_cl.h
        imglib/transform_cl.h
        imglib/utils_cl.h
    )

    set(imglib_sources ${imglib_sources}
        colorConversion_cl.cpp
        kernels_cl.cpp
        openCLImage.cpp
        openCLImageBuffer.cpp
        operators_cl.cpp
        transform_cl.cpp
        utils_cl.cpp
    )

    set(imglib_libraries ${imglib_libraries} OpenCL::OpenCL)

    include(EmbedFile)
    set(embedFiles
        kernels/colorConversion.cl
        kernels/operators.cl
        kernels/transform.cl
        kernels/utils.cl
    )
    embed_files(embededFiles "${embedFiles}" "imglib::cl")

endif()


add_library(imglib STATIC
    ${imglib_sources}
    ${imglib_headers}
    ${embededFiles}
)

include(GenerateExportHeader)
generate_export_header(imglib)

target_link_libraries(imglib ${imglib_libraries})

target_include_directories(imglib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(imglib PUBLIC ${PROJECT_BINARY_DIR}) #export files will be here

target_compile_definitions(imglib PUBLIC "-DIMGLIB_USE_FILESYSTEM=${IMGLIB_USE_FILESYSTEM}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if(IMGLIB_USE_FILESYSTEM GREATER ${IMGLIB_BOOST_FILESYSTEM})
        target_compile_options(imglib PUBLIC "-lstdc++fs")
        target_link_libraries(imglib stdc++fs)
    endif()
endif()

if(IMGLIB_USE_OPENCL)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_OPENCL)
    if(IMGLIB_USE_OPENCL_CPP)
        target_compile_definitions(imglib PUBLIC -DIMGLIB_OPENCL_CPP)
    endif()

    get_target_property(OPENCL_BIN_DIR OpenCL::OpenCL IMPORTED_LOCATION_DEBUG)
    get_filename_component(OPENCL_BIN_DIR ${OPENCL_BIN_DIR} DIRECTORY)
endif()

if(IMGLIB_USE_EIGEN)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_EIGEN)
endif()

if(IMGLIB_USE_PNG)
    target_compile_definitions(imglib PUBLIC -DIMGLIB_PNG)
endif()

#build test application
if(IMGLIB_BUILD_TEST)
    set(imglib_benchmark_headers
    )
    
    set(imglib_benchmark_sources
        test/benchmark.cpp
    )

    if(IMGLIB_USE_OPENCL)
        set(imglib_benchmark_headers ${imglib_benchmark_headers}
            test/benchmarkOpenCL.h
        )

        set(imglib_benchmark_sources ${imglib_benchmark_sources}
            test/benchmarkOpenCL.cpp
        )
    endif()

    add_executable(imglibBenchmark WIN32
        ${imglib_benchmark_headers}
        ${imglib_benchmark_sources}
    )

    target_link_libraries(imglibBenchmark imglib gflags-static opencl_util)

    set_target_properties(imglibBenchmark PROPERTIES FOLDER "benchmarks")

    if(WIN32)
        foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${CONFIG_TYPE} CONFIG_TYPE_UPPER )
            set_target_properties(imglibBenchmark PROPERTIES LINK_FLAGS_${CONFIG_TYPE_UPPER} "/SUBSYSTEM:CONSOLE")
            set_target_properties(imglibBenchmark PROPERTIES COMPILE_DEFINITIONS_${CONFIG_TYPE_UPPER} "_CONSOLE")
        endforeach()
    endif()

    set(RUNTIME_LIBS "")

    if(IMGLIB_USE_OPENCL)
        set(RUNTIME_LIBS ${RUNTIME_LIBS} ${OPENCL_BIN_DIR})
    endif()

    #message(FATAL_ERROR "RUNTIME_LIBS: ${RUNTIME_LIBS}")
    create_target_launcher(imglibBenchmark
        RUNTIME_LIBRARY_DIRS ${RUNTIME_LIBS}
    )
endif()

