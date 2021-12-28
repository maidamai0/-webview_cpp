if(${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
  set(arch x64)
else()
  set(arch x86)
endif()

add_library(deps INTERFACE)

nuget_install(webview2_path Microsoft.Web.WebView2 1.0.1083-prerelease)
target_include_directories(deps INTERFACE ${webview2_path}/build/native/include)
target_link_libraries(deps INTERFACE ${webview2_path}/build/native/${arch}/WebView2LoaderStatic.lib)

nuget_install(wil_path Microsoft.Windows.ImplementationLibrary "")
target_include_directories(deps INTERFACE ${wil_path}/include)