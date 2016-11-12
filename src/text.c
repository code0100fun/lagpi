#include "./freetype/texture-atlas.h"

texture_atlas_t *atlas;

char * vert = 
  "uniform mat4    u_mvp;\n"
  "attribute vec3    a_position;\n"
  "attribute vec4    a_color;\n"
  "attribute vec2    a_st;\n"
  "varying vec2            v_frag_uv;\n"
  "varying vec4            v_color;\n"
  "void main(void) {\n"
  "       v_frag_uv = a_st;\n"
  "       gl_Position = u_mvp * vec4(a_position,1);\n"
  "       v_color = a_color;\n"
  "}\n";


char * frag = 
  "precision mediump float;\n"
  "uniform sampler2D    texture_uniform;\n"
  "varying vec2 v_frag_uv;\n"
  "varying vec4            v_color;\n"
  "void main()\n"
  "{\n"
  "    gl_FragColor = vec4(v_color.xyz, v_color.a * texture2D(texture_uniform, v_frag_uv).a);\n"
  "}\n";
