file(
 GLOB_RECURSE WORKSESSION0_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE WORKSESSION0_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this workSession0's asset folder to bin when it is built
add_custom_target(copyAssetsW0 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${WORKSESSION0_INC} DESTINATION include/workSession0)
add_executable(workSession0 ${WORKSESSION0_SRC} ${WORKSESSION0_INC})
target_link_libraries(workSession0 PUBLIC core IMGUI assimp)
target_include_directories(workSession0 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when workSession0 is built
add_dependencies(workSession0 copyAssetsW0)