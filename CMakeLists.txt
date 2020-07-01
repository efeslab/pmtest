# This is a stop-gap. Should really port the Makefile into a CMake file directly,
# but this is easier.
add_custom_target(PMTEST ALL 
                  make BUILD=${CMAKE_CURRENT_BINARY_DIR} 
                       -C ${CMAKE_CURRENT_SOURCE_DIR} 
                       debug)

# Export information.
set(PMTEST_LIBS ${CMAKE_CURRENT_BINARY_DIR}/libs CACHE INTERNAL "PMTest lib dir")
set(PMTEST_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "PMTest include dir")