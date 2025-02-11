file(
 GLOB_RECURSE WORKSESSION1_5_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION1_5_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this workSession1_5's asset folder to bin when it is built
add_custom_target(copyAssetsW1_5 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION1_5_INC} DESTINATION include/workSession1_5)
add_executable(workSession1_5 ${WORKSESSION1_5_SRC} ${WORKSESSION1_5_INC})
target_link_libraries(workSession1_5 PUBLIC core IMGUI assimp)
target_include_directories(workSession1_5 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when workSession1_5 is built
add_dependencies(workSession1_5 copyAssetsW1_5)