# This is a stop-gap. Should really port the Makefile into a CMake file directly,
# but this is easier.
add_custom_target(PMTest ALL 
                  make BUILD=${CMAKE_CURRENT_BINARY_DIR} 
                       -C ${CMAKE_CURRENT_SOURCE_DIR} 
                       debug)

# Export information.
set_property(GLOBAL PROPERTY pmtest_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/libs)
set_property(GLOBAL PROPERTY pmtest_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)