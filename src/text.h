#include "./freetype/texture-font.h"

int initText();
void addText(texture_font_t *font, const wchar_t *text, vec4 * color, vec2 * loc);
void render(int programHandle);
