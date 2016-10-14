hunter_config(glm VERSION 0.9.7.6)
hunter_config(zlib VERSION 1.2.8-p3)
hunter_config(RapidJSON VERSION 1.0.2-p2)
hunter_config(glfw VERSION 3.2-p0)

hunter_config(Boost VERSION 1.61.0
CMAKE_ARGS 
	BUILD_SHARED_LIBS=ON 
	Boost_USE_STATIC_LIBS=OFF
	Boost_USE_MULTITHREADED=ON
	Boost_USE_STATIC_RUNTIME=OFF
	BOOST_ALL_DYN_LINK=ON
)

