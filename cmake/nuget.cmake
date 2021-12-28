function(nuget_install install_dir package_name package_version)
  message(STATUS "Installing ${package_name} ${package_version}")
  if(package_version)
    set(install_cmd nuget install ${package_name} -Version ${package_version})
  else()
    set(install_cmd nuget install ${package_name})
  endif()
  
  execute_process(
    COMMAND ${install_cmd}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
    OUTPUT_VARIABLE NONE
    COMMAND_ECHO STDOUT
  )
  file(GLOB NUGET_PACKAGE_FILES ${CMAKE_BINARY_DIR}/${package_name}*)
  list(GET NUGET_PACKAGE_FILES 0 package_full_path)
  message(STATUS "Installed ${package_name} to ${package_full_path}")
  set("${install_dir}" ${package_full_path} PARENT_SCOPE)
endfunction()
