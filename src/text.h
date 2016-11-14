#ifdef __cplusplus
extern "C" {
#endif

struct font_t {
  unsigned int font_texture;
  float pt;
  float advance[128];
  float width[128];
  float height[128];
  float tex_x1[128];
  float tex_x2[128];
  float tex_y1[128];
  float tex_y2[128];
  float offset_x[128];
  float offset_y[128];
  int initialized;
};

typedef struct font_t font_t;

font_t* load_font(const char* path, int point_size, int dpi);

#ifdef __cplusplus  
}  
#endif 
