project(fastNoise)

#remove min and max macro declares from windows builds, causes issues with std::min/max
add_definitions(-DNOMINMAX)

set(fastNoise_sources
    FastNoiseSIMD.cpp
    FastNoiseSIMD_avx2.cpp
    FastNoiseSIMD_avx512.cpp
    FastNoiseSIMD_internal.cpp
    FastNoiseSIMD_neon.cpp
    FastNoiseSIMD_sse2.cpp
    FastNoiseSIMD_sse41.cpp
)

set(fastNoise_headers
    FastNoiseSIMD.h
    FastNoiseSIMD_internal.h
)

set(fastNoise_libraries
)

add_library(fastNoise STATIC
    ${fastNoise_sources}
    ${fastNoise_headers}
)

target_link_libraries(fastNoise ${fastNoise_libraries})

target_include_directories(fastNoise PUBLIC ${CMAKE_CURRENT_LIST_DIR})