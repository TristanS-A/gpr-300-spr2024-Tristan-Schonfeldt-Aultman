file(
 GLOB_RECURSE WORKSESSION1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this workSession1's asset folder to bin when it is built
add_custom_target(copyAssetsW1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION1_INC} DESTINATION include/workSession1)
add_executable(workSession1 ${WORKSESSION1_SRC} ${WORKSESSION1_INC})
target_link_libraries(workSession1 PUBLIC core IMGUI assimp)
target_include_directories(workSession1 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when workSession1 is built
add_dependencies(workSession1 copyAssetsW1)