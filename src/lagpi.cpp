/*
lagpi.cpp
Chase McCarthy
@code0100fun
2016
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <wiringPi.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <bcm_host.h>
#include "./LoadShaders.h"

// WiringPI pins (use `$ gpio readall` to find the pin)
#define BUTTON_PIN      7
#define LED_PIN         15
#define DETECTOR_PIN    29

#define check() assert(glGetError() == 0)

struct timespec pressedAt;
volatile int pressed = 0;
volatile int measure = 0;
volatile int light = 0;

uint32_t GScreenWidth;
uint32_t GScreenHeight;
EGLDisplay GDisplay;
EGLSurface GSurface;
EGLContext GContext;
EGL_DISPMANX_WINDOW_T nativewindow;

volatile GLuint shaderProgramID;

void draw();
void updateScreen();

void startMeasure() {
  measure = 1;
  clock_gettime(CLOCK_REALTIME, &pressedAt);
  digitalWrite(LED_PIN, HIGH);
}

void buttonChanged(void) {
  pressed = digitalRead(BUTTON_PIN) == HIGH;
  measure = 0;
  if (pressed) {
    if (light) {
      printf("It's not dark enough!\n");
    } else {
      startMeasure();
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  draw();
  updateScreen();
}

void detectorChanged(void) {
  light = digitalRead(DETECTOR_PIN) == LOW;
  if (light) {
    if (measure) {
      struct timespec detectedAt;
      clock_gettime(CLOCK_REALTIME, &detectedAt);
      double usLag;
      usLag = (detectedAt.tv_sec - pressedAt.tv_sec) * 1000000.0;
      usLag += (detectedAt.tv_nsec - pressedAt.tv_nsec) / 1000.0;
      measure = 0;
      printf("lag: %f us\n", usLag);
    }
  }
}

int initDetector() {
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }

  pinMode(LED_PIN, OUTPUT);

  if (wiringPiISR (BUTTON_PIN, INT_EDGE_BOTH, &buttonChanged) < 0) {
      fprintf (stderr, "Unable to setup button ISR: %s\n", strerror (errno));
      return 1;
  }

  if (wiringPiISR (DETECTOR_PIN, INT_EDGE_BOTH, &detectorChanged) < 0) {
      fprintf (stderr, "Unable to setup detector ISR: %s\n", strerror (errno));
      return 1;
  }

  return 0;
}

void updateScreen() {
  eglSwapBuffers(GDisplay,GSurface);
}

void setViewport() {
  glViewport(0, 0, GScreenWidth, GScreenHeight);
}

int initGraphics() {
  bcm_host_init();

  int32_t success = 0;
  EGLBoolean result;
  EGLint num_config;

  DISPMANX_ELEMENT_HANDLE_T dispman_element;
  DISPMANX_DISPLAY_HANDLE_T dispman_display;
  DISPMANX_UPDATE_HANDLE_T dispman_update;

  VC_RECT_T dst_rect;
  VC_RECT_T src_rect;

  static const EGLint attribute_list[] =
  {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, 16,   // You need this line for depth buffering to work
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  static const EGLint context_attributes[] = 
  {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };

  EGLConfig config;

  // get an EGL display connection
  GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  assert(GDisplay!=EGL_NO_DISPLAY);
  check();

  // initialize the EGL display connection
  result = eglInitialize(GDisplay, NULL, NULL);
  assert(EGL_FALSE != result);
  check();

  // get an appropriate EGL frame buffer configuration
  result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
  assert(EGL_FALSE != result);
  check();

  // get an appropriate EGL frame buffer configuration
  result = eglBindAPI(EGL_OPENGL_ES_API);
  assert(EGL_FALSE != result);
  check();

  // create an EGL rendering context
  GContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
  assert(GContext!=EGL_NO_CONTEXT);
  check();

  // create an EGL window surface
  success = graphics_get_display_size(0 /* LCD */, &GScreenWidth, &GScreenHeight);
  assert(success >= 0);

  dst_rect.x = 0;
  dst_rect.y = 0;
  dst_rect.width = GScreenWidth;
  dst_rect.height = GScreenHeight;

  src_rect.x = 0;
  src_rect.y = 0;
  src_rect.width = GScreenWidth << 16;
  src_rect.height = GScreenHeight << 16;

  dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
  dispman_update = vc_dispmanx_update_start( 0 );

  dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
    0/*layer*/, &dst_rect, 0/*src*/,
    &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

  nativewindow.element = dispman_element;
  nativewindow.width = GScreenWidth;
  nativewindow.height = GScreenHeight;
  vc_dispmanx_update_submit_sync( dispman_update );

  check();

  GSurface = eglCreateWindowSurface( GDisplay, config, &nativewindow, NULL );
  assert(GSurface != EGL_NO_SURFACE);
  check();

  // connect the context to the surface
  result = eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);
  assert(EGL_FALSE != result);
  check();

  // Set background color and clear buffers
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  setViewport();

  check();

  shaderProgramID = LoadShaders("simplevertshader.glsl", "simplefragshader.glsl");

  return 0;
}

GLfloat g_vertex_buffer_data[] = {
  -0.3f, -0.3f, 0.0f,
   0.3f, -0.3f, 0.0f,
   0.3f,  0.3f, 0.0f,
  -0.3f,  0.3f, 0.0f,
  -0.3f, -0.3f, 0.0f,
   0.3f,  0.3f, 0.0f,
};

void draw() {
  glClear(GL_COLOR_BUFFER_BIT);
  if (pressed) {
    glUseProgram(shaderProgramID);
    glVertexAttribPointer(
      0,                   //vertexPosition_modelspaceID, // The attribute we want to configure
      3,                   // size
      GL_FLOAT,            // type
      GL_FALSE,            // normalized?
      0,                   // stride
      g_vertex_buffer_data // (void*)0                   // array buffer offset
    );
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 2*3);
  }
}

int main(void) {

  initDetector();
  initGraphics();

  while (1) {
    draw();
    updateScreen();
    delay(0);
  }

  return 0;
}
