file(GLOB srcs *.cpp *.rc)
file(GLOB res WebView2Sample* *.png *.ico)

get_filename_component(app ${CMAKE_CURRENT_LIST_DIR} NAME)
add_executable(${app} WIN32 ${srcs})

target_link_libraries(${app} Urlmon CoreMessaging)
target_precompile_headers(${app} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/pch.h)

add_custom_command(
        TARGET ${app} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${res} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

