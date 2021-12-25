file(GLOB srcs *.cpp)

foreach(src ${srcs})
  get_filename_component(app ${src} NAME_WLE)
  add_executable(${app} WIN32 ${src})
endforeach()
