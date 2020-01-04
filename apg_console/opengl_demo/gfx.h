// Graphics API Platform Interface
// Copyright Anton Gerdelan <antonofnote@gmail.com>. 2019
//
// -- putting all gl... calls behind an interface, for a change.
// if we want to port to eg Metal later should be easier
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// =================================================================================================
//                                           TYPES
// =================================================================================================
#define GFX_MAX_MESH_FILENAME 2048
#define GFX_MAX_SHADER_FILENAME 2048
#define GFX_MAX_TEXTURE_FILENAME 2048
#define GFX_MAX_FB_OUTPUTS 16 // framebuffer

// per-frame graphics statistics
typedef struct gfx_framestats_t {
  uint32_t n_draws;
  uint32_t n_verts;
  uint32_t n_uniforms;
  uint32_t n_posts;
} gfx_framestats_t;

typedef enum gfx_polygon_t { GFX_TRIANGLES, GFX_TRIANGLE_STRIP } gfx_polygon_t;
typedef enum gfx_draw_mode_t { GFX_STREAM_DRAW, GFX_STATIC_DRAW, GFX_DYNAMIC_DRAW } gfx_draw_mode_t;

// 3d geometry descriptor struct
typedef struct gfx_mesh_t {
  char filename[GFX_MAX_MESH_FILENAME]; // could be used to reload at run-time
  uint32_t vao, vbo, n_verts;           // OpenGL handles
  gfx_draw_mode_t draw_mode;
  gfx_polygon_t polygon_type;
} gfx_mesh_t;

// various attribute memory lauout configurations supported
typedef enum gfx_geom_mem_layout_t {
  GFX_MEM_POS,                 // 3 floats
  GFX_MEM_POS_ST,              // 5 floats, 2 attribs
  GFX_MEM_POS_NOR,             // 6 floats, 2 attribs
  GFX_MEM_POS_NOR_RGB,         // 9 floats, 3 attribs
  GFX_MEM_POS_NOR_ST_RGB,      // 11 floats, 4 attribs
  GFX_MEM_POS_ST_RG,           // 7 floats, 3 attribs
  GFX_MEM_POS_ST_NOR,          // 8 floats, 3 attribs
  GFX_MEM_POS_ST_NOR_RGB,      // 11 floats, 4 attribs
  GFX_MEM_POS_ST_NOR_RGB_H_K_E // 14 floats, 7 attribs
} gfx_geom_mem_layout_t;

// shader program descriptor
typedef struct gfx_shader_t {
  // filenames used to reload shader
  char vs_filename[GFX_MAX_SHADER_FILENAME], fs_filename[GFX_MAX_SHADER_FILENAME];
  // GL handles
  uint32_t vertex_shader, fragment_shader, program;
  // commonly used uniforms' locations
  int u_main_scene_texture_loc, u_secondary_texture_loc, u_tertiary_texture_loc, u_day_gradient_texture_loc;
  int u_P_loc, u_V_loc, u_M_loc, u_PV_loc, u_PVM_loc;
  int u_colour_loc, u_background_colour_loc, u_scale_loc, u_st_scale_loc, u_pos_loc, u_time_loc, u_walk_time_loc, u_progress_factor_loc;
  int u_scroll_val_loc, u_tile_index_loc;
  int u_sun_dir_wor_loc, u_day_gradient_factor_loc;
  int u_framebuffer_dims_loc;
  int u_opacity_loc;
} gfx_shader_t;

// descriptor for a loaded texture
typedef struct gfx_texture_t {
  unsigned int handle;                     // used by GL
  char filename[GFX_MAX_TEXTURE_FILENAME]; // could be used to reload at run-time
  bool mag_linear;                         // bi-linear sampling otherwise nearest neighbour
  bool min_linear;                         // bi-linear sampling otherwise nearest neighbour
  bool mipmaps;                            // upgrades bi-linear to tri-linear
  bool clamp;                              // clamp to edge. otherwise repeat
  bool srgb;                               // determines format rgb or srgb or if n_chans 4 then rgba or srgba
  bool is_depth;

  // if loading is threaded then do not touch the below until loaded is true
  bool loaded;
  // ----------------------- thread touches the below ----------------------------
  unsigned char* pixels_ptr; // only used whilst loading
  int w, h, n_chans;         // dims. chans determines red, rg, rgb, rgba format
} gfx_texture_t;

// main properties and bounds outputs of a framebuffer rendering pass. use to daisy-chain passes
typedef struct gfx_framebuffer_meta_t {
  int width, height;                                 // some multiple of viewport dims
  gfx_texture_t output_textures[GFX_MAX_FB_OUTPUTS]; //
  int n_output_textures;                             //
  gfx_texture_t depth_texture;                       //
  float x_scale, y_scale;                            // used in recreation. don't need to specify
  float background_colour_rgba[4];                   // used in glClearColor()
  unsigned int handle;                               //
} gfx_framebuffer_meta_t;

// =================================================================================================
//                                           Extras
// =================================================================================================
void gfx_screenshot();

void gfx_encode_idx_to_rgb( uint32_t idx, float* r, float* g, float* b );
uint32_t gfx_decode_rgb_to_idx( unsigned char r, unsigned char g, unsigned char b );

// =================================================================================================
//                                           Context
// =================================================================================================

// main framebuffer dims. used for calculating aspect and transforming 2D UI elements.
extern int gfx_fb_width, gfx_fb_height;
extern int gfx_win_width, gfx_win_height;

// in your drawing loop, check if this is set to true and if so recalc cameras' aspect and set false
extern bool gfx_fb_resized;
extern float gfx_framebuffer_scale;

extern gfx_framestats_t gfx_framestats;

// if icon_filename is NULL then doesn't try to set a window icon
// returns false if can't start context
bool gfx_start( int win_w, int win_h, bool msaa, const char* window_title, const char* icon_filename );
void gfx_stop();

bool gfx_should_window_close();
void gfx_close_window();
void gfx_show_system_cursor();
void gfx_hide_system_cursor();

void gfx_clear_colour_and_depth_buffers();
void gfx_clear_depth_buffer();
// use for UI screens etc to go easy on the CPU
void gfx_swap_buffer_and_wait_events();
// use for loops containing simulation code that draws w/o user input
void gfx_swap_buffer_and_poll_events();
void gfx_wireframe_mode();
void gfx_polygon_mode();
void gfx_blend_one_minus_src_alpha( bool enabled );
void gfx_depth_testing( bool enabled ); // reading from depth buffer
void gfx_depth_masking( bool enabled ); // writing to depth buffer

// =================================================================================================
//                                           Geometry
// =================================================================================================
#define GFX_MAX_MANAGED_MESHES 1024

// primitive meshes for 3D and 2D stuff
extern gfx_mesh_t gfx_unit_cube_mesh, gfx_ss_quad_mesh;
extern gfx_mesh_t gfx_managed_meshes[GFX_MAX_MANAGED_MESHES];
extern int gfx_n_managed_meshes;

int gfx_managed_mesh_find_index( const char* filename );

int gfx_create_managed_mesh_from_ply( const char* filename );

void gfx_reload_all_managed_meshes();

// called by stop_gfx()
void gfx_delete_all_managed_meshes();

// sz can be 0 and data can be NULL, and n_verts can be 0
gfx_mesh_t gfx_create_mesh( const void* data, size_t sz, gfx_geom_mem_layout_t layout, unsigned int n_verts, gfx_draw_mode_t mode, gfx_polygon_t polygon_type );

// returns default unit cube mesh on error
gfx_mesh_t gfx_create_mesh_from_ply( const char* filename );

void gfx_update_mesh( gfx_mesh_t* mesh, const void* data, size_t sz, size_t n_verts );

void gfx_reload_mesh( gfx_mesh_t* mesh );

// also resets memory struct memory and handles to zero
// asserts if mesh ptr is NULL
void gfx_delete_mesh( gfx_mesh_t* mesh );

// =================================================================================================
//                                           Shaders
// =================================================================================================

#define GFX_MAX_MANAGED_SHADERS 1024

extern gfx_shader_t gfx_managed_shaders[GFX_MAX_MANAGED_SHADERS];
extern int gfx_n_managed_shaders;
extern gfx_shader_t gfx_fallback_shader;

int gfx_managed_shader_find_index( const char* vs_filename, const char* fs_filename );

// returns index of managed shader
// asserts of invalid input or full array
int gfx_create_managed_shader_from_files( const char* vs_filename, const char* fs_filename );

void gfx_reload_all_managed_shaders();

// called by stop_gfx()
void gfx_delete_all_managed_shaders();

void gfx_reload_shader( gfx_shader_t* shader );

// also resets memory struct memory and handles to zero
// asserts if mesh ptr is NULL
void gfx_delete_shader( gfx_shader_t* shader );

int gfx_uniform_loc( gfx_shader_t shader, const char* name );

void gfx_uniform_mat4( gfx_shader_t shader, int loc, const float* m );
void gfx_uniform_1i( gfx_shader_t shader, int loc, int x );
void gfx_uniform_1f( gfx_shader_t shader, int loc, float x );
void gfx_uniform_2f( gfx_shader_t shader, int loc, float x, float y );
void gfx_uniform_3f( gfx_shader_t shader, int loc, float x, float y, float z );
void gfx_uniform_3fv( gfx_shader_t shader, int loc, int count, float* v );
void gfx_uniform_4f( gfx_shader_t shader, int loc, float x, float y, float z, float w );
void gfx_uniform_4fv( gfx_shader_t shader, int loc, int count, float* v );

// =================================================================================================
//                                           Textures
// =================================================================================================

#define MAX_MANAGED_TEXTURES 1024

extern uint8_t gfx_fallback_pixels[16];
extern gfx_texture_t gfx_fallback_texture;
extern gfx_texture_t gfx_managed_textures[GFX_MAX_MANAGED_SHADERS];
extern int gfx_n_managed_textures;

// if a texture is already loaded from filename then function returns its index. otherwise -1
int gfx_managed_texture_find_index( const char* filename );

// calls appropriate load_ function, and manages resource internally to support reload_all behaviour etc.
// returns index to managed texture
int gfx_create_managed_texture_from_file( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb );

// calls appropriate load_ function, and manages resource internally to support reload_all behaviour etc.
// returns index to managed texture
int gfx_create_managed_texture_from_image_mem( const unsigned char* pixels, int w, int h, int n_chans, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb );

// reloads all textures loaded via create_managed_texture_from_file()
void gfx_reload_all_managed_textures();

// deletes all textures loaded via create_managed_texture_from_file()
// called by stop_gfx()
void gfx_delete_all_managed_textures();

// reloads a single resource, managed or not
void gfx_reload_texture( gfx_texture_t* texture );

// copies image data from pixels into existing texture. also updates texture filters, properties
// asserts on NULL texture or 0 texture handle. NULL pixels data is okay.
void gfx_update_texture( gfx_texture_t* texture, const unsigned char* pixels );

// create a texture using a image data in main memory. you are responsible for deleting this texture with delete_texture()
// pixels can be NULL. asserts on w,h, n_chans in valid range.
// returns filled gfx_texture_t struct
gfx_texture_t
gfx_load_image_mem_to_texture( const unsigned char* pixels, int w, int h, int n_chans, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb );

// create a texture from a file on disk. you are responsible for deleting this texture with delete_texture()
// asserts on NULl filename
// returns filled gfx_texture_t struct
gfx_texture_t gfx_load_image_file_to_texture_serial( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb );

// threaded version of load_image_file_to_texture()
// you shouldn't need to call this directly unless reloading a texture. use create_texture_from_file
void gfx_load_image_file_to_gfx_texture_threaded( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb, struct gfx_texture_t* texture );

// deletes a specific texture. no need to call this for textures created via create_managed_texture_from_file()
void gfx_delete_texture( gfx_texture_t* texture );

// capture current framebuffer in a texture
gfx_texture_t gfx_render_to_texture();

void gfx_resize_image( const unsigned char* src_pixels, int src_w, int src_h, int scale_factor, unsigned char* dst_pixels );

// =================================================================================================
//                                          Draw Helpers
// =================================================================================================
// draw one untextured mesh
void gfx_draw_mesh( gfx_mesh_t mesh, gfx_shader_t shader );

// draw one multi-textured mesh
void gfx_draw_gfx_mesh_texturedv( gfx_mesh_t mesh, gfx_shader_t shader, gfx_texture_t* textures, int ntextures );

// =================================================================================================
//                                       Post Processing
// =================================================================================================
// fb_meta can be NULL for default framebuffer
void gfx_buffer_colour( gfx_framebuffer_meta_t* fb_meta, float r, float g, float b, float a );

// fb_meta can be NULL for default framebuffer
void gfx_get_buffer_colour( const gfx_framebuffer_meta_t* fb_meta, float* r, float* g, float* b, float* a );

// call once for each post-processing stage
bool gfx_create_framebuffer( gfx_framebuffer_meta_t* fb_meta );

// call this whenever viewport dims have changed eg after a window resize
bool gfx_rebuild_framebuffer( gfx_framebuffer_meta_t* fb_meta );

// binds framebuffer and also sets viewport to matching size
void gfx_bind_framebuffer_with_viewport( const gfx_framebuffer_meta_t* fb_meta );

// returns false if mouse out of the house. otherwise gives an decoded rgb -> index value
bool gfx_read_idx_from_fb( const gfx_framebuffer_meta_t* fb_meta, uint32_t* hovered_idx );

// =================================================================================================
//                                              Time
// =================================================================================================
// returns current time (since program start i think) in seconds. it's in this file because it reuses GLFW's util
double gfx_get_time_s();
