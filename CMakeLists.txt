cmake_minimum_required (VERSION 2.8)
project ("Raspberry Pi Zero Screen Lag Tester")

SET(COMPILE_DEFINITIONS -Werror)

include_directories(
  /opt/vc/include
  /opt/vc/include/interface/vcos/pthreads
  /opt/vc/include/interface/vmcs_host/linux
)

link_directories(
  /opt/vc/lib
)

set(RPi_LIBS
  libbcm_host.so
  libvcos.so
  wiringPi
  rt
  LoadShaders
)

set(GL_LIBS
  GLESv2
  EGL
)

add_executable(lagpi src/lagpi.cpp)

add_library(LoadShaders src/LoadShaders.cpp)

target_link_libraries(lagpi
  ${RPi_LIBS}
  ${GL_LIBS}
)

file(
  COPY
  src/simplefragshader.glsl
  src/simplevertshader.glsl
  DESTINATION ${CMAKE_BINARY_DIR}
)
