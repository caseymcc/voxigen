if(NOT DEFINED HUNTER_PACKAGE_INIT)
	if(NOT DEFINED HUNTER_PACKAGE_DIR) #not set assume part of source tree
		set(HUNTER_PACKAGE_DIR "${CMAKE_SOURCE_DIR}/packages/" CACHE STRING)
	endif()
	
	set(ENV{HUNTER_ROOT} "${HUNTER_PACKAGE_DIR}")
	set(HUNTER_DIR "${CMAKE_SOURCE_DIR}/cmake/Hunter")

	find_package(Git)

	if(NOT GIT_FOUND)
		message(FATAL_ERROR "HunterPackage requires Git.")
	endif()

	

	if (NOT EXISTS ${HUNTER_DIR}/HunterGate.cmake)
#		message(STATUS "Getting HunterGate (https://github.com/hunter-packages/gate/blob/master/cmake)")
#		execute_process(
#			COMMAND "${GIT_EXECUTABLE}" clone https://github.com/hunter-packages/gate/blob/master/cmake ${HUNTER_DIR}
#			RESULT_VARIABLE error_code
#			OUTPUT_QUIET ERROR_QUIET
#		)
#		if(error_code)
#			message(FATAL_ERROR "Hunter failed to get the hash for HEAD")
#		endif()
		set(fileName "HunterGate.cmake")
		set(url "https://raw.githubusercontent.com/hunter-packages/gate/master/cmake/${fileName}")
		file(DOWNLOAD ${url}
			${HUNTER_DIR}/${fileName}
			STATUS status
			LOG log)

		list(GET status 0 statusCode)
		list(GET status 1 statusString)
		
		if(NOT statusCode EQUAL 0)
			message(FATAL_ERROR "error: downloading '${url}' failed
			error: ${statusCode} - ${statusString}
			log: ${log}")
		endif()
	endif()
	set(HUNTER_PACKAGE_INIT TRUE)
endif()

message(STATUS "Including HunterGate: ${HUNTER_DIR}/HunterGate.cmake")
include(${HUNTER_DIR}/HunterGate.cmake)