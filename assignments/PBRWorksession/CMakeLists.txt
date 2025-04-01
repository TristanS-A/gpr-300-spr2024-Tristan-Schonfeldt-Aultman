file(
 GLOB_RECURSE PBRWORKSESSION_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE PBRWORKSESSION_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this PBRWorksession's asset folder to bin when it is built
add_custom_target(PBRWorksessionAssets0 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${PBRWORKSESSION_INC} DESTINATION include/PBRWorksession)
add_executable(PBRWorksession ${PBRWORKSESSION_SRC} ${PBRWORKSESSION_INC})
target_link_libraries(PBRWorksession PUBLIC core IMGUI assimp)
target_include_directories(PBRWorksession PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when PBRWorksession is built
add_dependencies(PBRWorksession PBRWorksessionAssets0)