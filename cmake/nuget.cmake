function(nuget_install install_dir package_name)
  message(STATUS "Installing ${package_name}")
  execute_process(
    COMMAND nuget install ${package_name}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
    OUTPUT_VARIABLE NONE
  )
  file(GLOB NUGET_PACKAGE_FILES ${CMAKE_BINARY_DIR}/${package_name}*)
  list(GET NUGET_PACKAGE_FILES 0 package_full_path)
  message(STATUS "Installed ${package_name} to ${package_full_path}")
  set("${install_dir}" ${package_full_path} PARENT_SCOPE)
endfunction()
