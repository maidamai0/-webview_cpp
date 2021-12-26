cmake_minimum_required(VERSION 3.20)

get_filename_component(ROOT ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${ROOT} CXX)

set(CMAKE_GENERATOR Ninja)

# update submodules
add_custom_target(update_dependencies
    COMMAND ${PROXY} git submodule foreach git pull
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# cpp compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  # add_compile_options(/fsanitize=address)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_definitions(_UNICODE UNICODE _WINDOWS WIN32 WIN32_LEAN_AND_MEAN _SILENCE_CLANG_COROUTINE_MESSAGE)
  add_compile_options(/Zc:__cplusplus) # see https://docs.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-160 for detail
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(nuget)

add_subdirectory(deps)
link_libraries(deps)

add_subdirectory(src)
