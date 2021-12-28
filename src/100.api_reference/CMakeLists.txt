file(GLOB srcs *.cpp *.rc)

get_filename_component(app ${CMAKE_CURRENT_LIST_DIR} NAME)
add_executable(${app} WIN32 ${srcs})

target_link_libraries(${app} Urlmon CoreMessaging Shlwapi Pathcch D2d1)
target_precompile_headers(${app} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/stdafx.h)
target_compile_definitions(${app} PRIVATE USE_WEBVIEW2_WIN10)

add_custom_command(
        TARGET ${app} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory  ${CMAKE_CURRENT_LIST_DIR}/assets ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)

