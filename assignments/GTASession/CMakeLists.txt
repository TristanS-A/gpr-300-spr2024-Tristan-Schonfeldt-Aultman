file(
 GLOB_RECURSE GTASESSION_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE GTASESSION_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this GTASession's asset folder to bin when it is built
add_custom_target(GTASessionAssets0 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${GTASESSION_INC} DESTINATION include/GTASession)
add_executable(GTASession ${GTASESSION_SRC} ${GTASESSION_INC})
target_link_libraries(GTASession PUBLIC core IMGUI assimp)
target_include_directories(GTASession PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when GTASession is built
add_dependencies(GTASession GTASessionAssets0)