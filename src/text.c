#include <ft2build.h>
#include FT_FREETYPE_H
#include "./text.h"

font_t* load_font(const char* path, int point_size, int dpi) {
  FT_Library  library;
  FT_Face face;
  int c;
  int i, j;
  font_t* font;

  if(FT_Init_FreeType(&library)) {
    fprintf(stderr, "Error loading Freetype library\n");
    return NULL;
  }

  if (FT_New_Face(library, path, 0, &face)) {
    fprintf(stderr, "Error loading font %s\n", path);
    return NULL;
  }

  if(FT_Set_Char_Size ( face, point_size * 64, point_size * 64, dpi, dpi)) {
    fprintf(stderr, "Error initializing character parameters\n");
    return NULL;
  }

  font = (font_t*) malloc(sizeof(font_t));
  font->initialized = 0;
  
  glGenTextures(1, &(font->font_texture));


  // REST

  font->initialized = 1;
  return font;
}
