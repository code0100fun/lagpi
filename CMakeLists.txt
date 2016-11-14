cmake_minimum_required (VERSION 2.8)
project ("Raspberry Pi Zero Screen Lag Tester")

SET(COMPILE_DEFINITIONS -Werror)

include_directories(
  /opt/vc/include
  /opt/vc/include/interface/vcos/pthreads
  /opt/vc/include/interface/vmcs_host/linux
  /usr/local/include/freetype2
)

link_directories(
  /opt/vc/lib
)

set(RPi_LIBS
  libbcm_host.so
  libvcos.so
  wiringPi
  rt
  freetype
)

set(GL_LIBS
  GLESv2
  EGL
)

file(GLOB_RECURSE SRC src/*.c src/*.cpp)
add_executable(lagpi ${SRC})
target_link_libraries(lagpi
  ${RPi_LIBS}
  ${GL_LIBS}
)

file(
  COPY
  src/simplefragshader.glsl
  src/simplevertshader.glsl
  src/fonts
  DESTINATION ${CMAKE_BINARY_DIR}
)
