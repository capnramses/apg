// Copyright Anton Gerdelan <antonofnote@gmail.com>. 2019
#include "gfx.h"
#include "glcontext.h"
#include "utils.h" // backtraces

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define GFX_A_POS_LOCATION 0
#define GFX_A_ST_LOCATION 1
#define GFX_A_NORM_LOCATION 2
#define GFX_A_COLOUR_LOCATION 3
#define GFX_A_PICKINDEX_LOCATION 4
#define GFX_A_HTAG_LOCATION 5
#define GFX_A_K_LOCATION 6
#define GFX_A_EXPLORED_FACTOR 7

// =================================================================================================
//                                  Externally Visible Globals ( more in sections below )
// =================================================================================================
GLFWwindow* g_window;
int gfx_win_width  = 1920;
int gfx_win_height = 1080;
int gfx_fb_width   = 1920;
int gfx_fb_height  = 1080;
bool gfx_fb_resized;
// can be set to 0.5 to downscale on high DPI systems, or up to 2.0 for SSAA on low-res systems with high power
// run-time changes to this factor would need to first set the x and y scale for existing framebuffers, then regenerate them
float gfx_framebuffer_scale = 1.0f;

gfx_framestats_t gfx_framestats;
uint8_t gfx_fallback_pixels[16] = { 255, 0, 255, 255, 64, 0, 64, 255, 64, 0, 64, 255, 255, 0, 255, 255 };

// =================================================================================================
//                                      Internal Globals ( more in sections below )
// =================================================================================================
static bool _g_fullscreen;
static int _g_monitor_to_use;

// =================================================================================================
//                                           Extras
// =================================================================================================
void gfx_encode_idx_to_rgb( uint32_t idx, float* r, float* g, float* b ) {
  assert( idx < 16777216 ); // 24-bit precision max for glsl highp
  assert( r && g && b );

  int ri = idx / 65536;
  int gi = ( idx - ri * 65536 ) / 256;
  int bi = ( idx - ri * 65536 - gi * 256 );
  *r     = (float)ri / 255.0f;
  *g     = (float)gi / 255.0f;
  *b     = (float)bi / 255.0f;
}

uint32_t decode_rgb_to_idx( unsigned char r, unsigned char g, unsigned char b ) {
  uint32_t idx = ( uint32_t )( b + g * 256 + r * 256 * 256 );
  return idx;
}

// TODO(Anton) bind default framebuffer in here
// using Sean Barrett's stb_image_write.h to write framebuffer to PNG
void gfx_screenshot() {
  uint8_t* buffer = scratch_mem_a( gfx_fb_width * gfx_fb_height * 3 );
  if ( !buffer ) {
    glog_err( "ERROR: could not allocate memory for screen capture\n" );
    return;
  }

  glPixelStorei( GL_PACK_ALIGNMENT, 1 ); // for irregular display sizes in RGB
  glReadPixels( 0, 0, gfx_fb_width, gfx_fb_height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
  char name[1024];
  long int t = time( NULL );
  sprintf( name, "screens/screenshot_%ld.png", t );
  uint8_t* last_row = buffer + ( gfx_fb_width * 3 * ( gfx_fb_height - 1 ) );
  if ( !stbi_write_png( name, gfx_fb_width, gfx_fb_height, 3, last_row, -3 * gfx_fb_width ) ) {
    glog_err( "ERROR: could not write screenshot file %s\n", name );
    return;
  }
}

// TODO(Anton) determine if fb scale should be used here otherwise bind default fb here
gfx_texture_t gfx_render_to_texture() {
  uint8_t* buffer = scratch_mem_a( gfx_fb_width * gfx_fb_height * 3 );
  assert( buffer );

  glPixelStorei( GL_PACK_ALIGNMENT, 1 ); // for irregular display sizes in RGB
  glReadPixels( 0, 0, gfx_fb_width, gfx_fb_height, GL_RGB, GL_UNSIGNED_BYTE, buffer );
  bool mag_linear   = false;
  bool min_linear   = false;
  gfx_texture_t tex = gfx_load_image_mem_to_texture( buffer, gfx_fb_width, gfx_fb_height, 3, true, mag_linear, min_linear, false, true );
  return tex;
}

void gfx_resize_image( const unsigned char* src_pixels, int src_w, int src_h, int scale_factor, unsigned char* dst_pixels ) {
  assert( src_pixels && dst_pixels );
  assert( src_w >= 1 && src_h >= 1 && scale_factor >= 1 );

  size_t sz_in  = src_w * src_h;
  size_t sz_out = sz_in * scale_factor;

  if ( sz_in == sz_out ) {
    memcpy( dst_pixels, src_pixels, sz_out );
    return;
  }

  // loops over src image
  for ( int src_y = 0; src_y < src_h; src_y++ ) {
    for ( int src_x = 0; src_x < src_w; src_x++ ) {
      int src_idx = src_y * src_w + src_x;

      for ( int dst_y = src_y * scale_factor; dst_y < src_y * scale_factor + scale_factor; dst_y++ ) {
        for ( int dst_x = src_x * scale_factor; dst_x < src_x * scale_factor + scale_factor; dst_x++ ) {
          int dst_idx         = dst_y * src_w * scale_factor + dst_x;
          dst_pixels[dst_idx] = src_pixels[src_idx];
        }
      }
    } // endfor
  }   // endfor
}

// =================================================================================================
//                                           Geometry
// =================================================================================================
gfx_mesh_t gfx_unit_cube_mesh, gfx_ss_quad_mesh;
gfx_mesh_t gfx_managed_meshes[GFX_MAX_MANAGED_MESHES];
int g_n_managed_meshes;

static void _init_ss_quad() {
  // clang-format off
  GLfloat ss_quad_pos[] = {
    -1.0,  1.0, // tl
    -1.0, -1.0, // bl
     1.0,  1.0, // tr
     1.0, -1.0  // br
  };
  // clang-format on
  gfx_ss_quad_mesh = gfx_create_mesh( ss_quad_pos, sizeof( ss_quad_pos ), GFX_MEM_POS, 4, GFX_STATIC_DRAW, GFX_TRIANGLE_STRIP );
}

static void _init_unit_cube() {
  // this has xyz positions and xyz normals interleaved
  // clang-format off
	//   x     y     z  nx  ny  nz     x     y     z  nx  ny  nz     x     y     z  nx  ny  nz
	GLfloat unit_cube_pos[18 * 12] = {
     1.0, -1.0,  1.0,  0, -1,  0, -1.0, -1.0,  1.0,  0, -1,  0, -1.0, -1.0, -1.0,  0, -1,  0, // bottom
    -1.0,  1.0, -1.0,  0,  1,  0, -1.0,  1.0,  1.0,  0,  1,  0,  1.0,  1.0,  1.0,  0,  1,  0, // top
     1.0,  1.0, -1.0,  1,  0,  0,  1.0,  1.0,  1.0,  1,  0,  0,  1.0, -1.0,  1.0,  1,  0,  0, // right
     1.0,  1.0,  1.0,  0,  0,  1, -1.0,  1.0,  1.0,  0,  0,  1, -1.0, -1.0,  1.0,  0,  0,  1, // front
    -1.0, -1.0,  1.0, -1,  0,  0, -1.0,  1.0,  1.0, -1,  0,  0, -1.0,  1.0, -1.0, -1,  0,  0, // left
     1.0, -1.0, -1.0,  0,  0, -1, -1.0, -1.0, -1.0,  0,  0, -1, -1.0,  1.0, -1.0,  0,  0, -1, // back
     1.0, -1.0, -1.0,  0, -1,  0,  1.0, -1.0,  1.0,  0, -1,  0, -1.0, -1.0, -1.0,  0, -1,  0, // bottom 2
     1.0,  1.0, -1.0,  0,  1,  0, -1.0,  1.0, -1.0,  0,  1,  0,  1.0,  1.0,  1.0,  0,  1,  0, // top 2
     1.0, -1.0, -1.0,  1,  0,  0,  1.0,  1.0, -1.0,  1,  0,  0,  1.0, -1.0,  1.0,  1,  0,  0, // right 2
     1.0, -1.0,  1.0,  0,  0,  1,  1.0,  1.0,  1.0,  0,  0,  1, -1.0, -1.0,  1.0,  0,  0,  1, // front 2
    -1.0, -1.0, -1.0, -1,  0,  0, -1.0, -1.0,  1.0, -1,  0,  0, -1.0,  1.0, -1.0, -1,  0,  0, // left 2
     1.0,  1.0, -1.0,  0,  0, -1,  1.0, -1.0, -1.0,  0,  0, -1, -1.0,  1.0, -1.0,  0,  0, -1  // back 2
  };
  // clang-format on
  gfx_unit_cube_mesh = gfx_create_mesh( unit_cube_pos, sizeof( unit_cube_pos ), GFX_MEM_POS_NOR, 36, GFX_STATIC_DRAW, GFX_TRIANGLES );
}

int gfx_managed_mesh_find_index( const char* filename ) {
  for ( int i = 0; i < g_n_managed_meshes; i++ ) {
    if ( 0 == strncmp( filename, gfx_managed_meshes[i].filename, GFX_MAX_MESH_FILENAME ) ) { return i; }
  }
  return -1;
}

int gfx_create_managed_mesh_from_ply( const char* filename ) {
  assert( g_n_managed_meshes < GFX_MAX_MANAGED_MESHES );
  assert( filename );

  int idx = gfx_managed_mesh_find_index( filename );
  if ( idx > -1 ) {
    glog( "mesh for `%s` already managed. reusing...\n", filename );
    return idx;
  }

  gfx_managed_meshes[g_n_managed_meshes] = gfx_create_mesh_from_ply( filename );
  return g_n_managed_meshes++;
}

void gfx_reload_all_managed_meshes() {
  for ( int i = 0; i < g_n_managed_meshes; i++ ) { gfx_reload_mesh( &gfx_managed_meshes[i] ); }
}

void gfx_delete_all_managed_meshes() {
  for ( int i = 0; i < g_n_managed_meshes; i++ ) {
    // avoid continuously deleting the same fallback if set several times
    if ( gfx_ss_quad_mesh.vao == gfx_managed_meshes[i].vao ) { continue; }
    if ( gfx_unit_cube_mesh.vao == gfx_managed_meshes[i].vao ) { continue; }
    gfx_delete_mesh( &gfx_managed_meshes[i] );
  }
  g_n_managed_meshes = 0;
}

gfx_mesh_t gfx_create_mesh( const void* data, size_t sz, gfx_geom_mem_layout_t layout, unsigned int n_verts, gfx_draw_mode_t mode, gfx_polygon_t polygon_type ) {
  gfx_mesh_t mesh;
  memset( &mesh, 0, sizeof( gfx_mesh_t ) );
  mesh.n_verts      = n_verts;
  mesh.draw_mode    = mode;
  mesh.polygon_type = polygon_type;

  GLenum gl_draw_mode = 0;
  switch ( mesh.draw_mode ) {
  case GFX_STATIC_DRAW: {
    gl_draw_mode = GL_STATIC_DRAW;
  } break;
  case GFX_DYNAMIC_DRAW: {
    gl_draw_mode = GL_DYNAMIC_DRAW;
  } break;
  case GFX_STREAM_DRAW: {
    gl_draw_mode = GL_STREAM_DRAW;
  } break;
  }

  glGenVertexArrays( 1, &mesh.vao );
  glGenBuffers( 1, &mesh.vbo );

  glBindVertexArray( mesh.vao );
  glBindBuffer( GL_ARRAY_BUFFER, mesh.vbo );
  {
    glBufferData( GL_ARRAY_BUFFER, sz, data, gl_draw_mode );

    switch ( layout ) {
    case GFX_MEM_POS: {
      int ncomps            = sz / ( n_verts * sizeof( float ) );
      GLsizei vertex_stride = sizeof( float ) * ncomps;
      assert( vertex_stride * n_verts == sz );
      glVertexAttribPointer( 0, ncomps, GL_FLOAT, GL_FALSE, 0, NULL ); // XYZ so 3
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
    } break;
    case GFX_MEM_POS_ST: {
      GLsizei vertex_stride = sizeof( float ) * 5;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_texcoord_offset = 3 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
    } break;
    case GFX_MEM_POS_NOR: {
      GLsizei vertex_stride = sizeof( float ) * 6;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_normal_offset   = 3 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
    } break;
    case GFX_MEM_POS_NOR_RGB: {
      GLsizei vertex_stride = sizeof( float ) * 9;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_normal_offset   = 3 * sizeof( float );
      GLintptr vertex_colour_offset   = 6 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glVertexAttribPointer( GFX_A_COLOUR_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_colour_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
      glEnableVertexAttribArray( GFX_A_COLOUR_LOCATION );
    } break;
    case GFX_MEM_POS_NOR_ST_RGB: {
      GLsizei vertex_stride = sizeof( float ) * 11;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_normal_offset   = 3 * sizeof( float );
      GLintptr vertex_texcoord_offset = 6 * sizeof( float );
      GLintptr vertex_colour_offset   = 8 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glVertexAttribPointer( GFX_A_COLOUR_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_colour_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
      glEnableVertexAttribArray( GFX_A_COLOUR_LOCATION );
    } break;
    case GFX_MEM_POS_ST_RG: {
      GLsizei vertex_stride = sizeof( float ) * 7;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_texcoord_offset = 3 * sizeof( float );
      GLintptr vertex_redgreen_offset = 5 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glVertexAttribPointer( GFX_A_COLOUR_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_redgreen_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
      glEnableVertexAttribArray( GFX_A_COLOUR_LOCATION );
    } break;
    case GFX_MEM_POS_ST_NOR: {
      GLsizei vertex_stride = sizeof( float ) * 8;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_texcoord_offset = 3 * sizeof( float );
      GLintptr vertex_normal_offset   = 5 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
    } break;
    case GFX_MEM_POS_ST_NOR_RGB: {
      GLsizei vertex_stride = sizeof( float ) * 11;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_texcoord_offset = 3 * sizeof( float );
      GLintptr vertex_normal_offset   = 5 * sizeof( float );
      GLintptr vertex_rgb_offset      = 8 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glVertexAttribPointer( GFX_A_PICKINDEX_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_rgb_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
      glEnableVertexAttribArray( GFX_A_PICKINDEX_LOCATION );
    } break;

    case GFX_MEM_POS_ST_NOR_RGB_H_K_E: {
      GLsizei vertex_stride = sizeof( float ) * 14;
      assert( vertex_stride * n_verts == sz );
      GLintptr vertex_position_offset = 0 * sizeof( float );
      GLintptr vertex_texcoord_offset = 3 * sizeof( float );
      GLintptr vertex_normal_offset   = 5 * sizeof( float );
      GLintptr vertex_rgb_offset      = 8 * sizeof( float );
      GLintptr vertex_h_offset        = 11 * sizeof( float );
      GLintptr vertex_k_offset        = 12 * sizeof( float );
      GLintptr vertex_e_offset        = 13 * sizeof( float );
      glVertexAttribPointer( GFX_A_POS_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_position_offset );
      glVertexAttribPointer( GFX_A_ST_LOCATION, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_texcoord_offset );
      glVertexAttribPointer( GFX_A_NORM_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_normal_offset );
      glVertexAttribPointer( GFX_A_PICKINDEX_LOCATION, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_rgb_offset );
      glVertexAttribPointer( GFX_A_HTAG_LOCATION, 1, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_h_offset );
      glVertexAttribPointer( GFX_A_K_LOCATION, 1, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_k_offset );
      glVertexAttribPointer( GFX_A_EXPLORED_FACTOR, 1, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)vertex_e_offset );
      glEnableVertexAttribArray( GFX_A_POS_LOCATION );
      glEnableVertexAttribArray( GFX_A_ST_LOCATION );
      glEnableVertexAttribArray( GFX_A_NORM_LOCATION );
      glEnableVertexAttribArray( GFX_A_PICKINDEX_LOCATION );
      glEnableVertexAttribArray( GFX_A_HTAG_LOCATION );
      glEnableVertexAttribArray( GFX_A_K_LOCATION );
      glEnableVertexAttribArray( GFX_A_EXPLORED_FACTOR );
    } break;
    default: { glog_err( "ERROR: unhandled vertex format!\n" ); } break;
    } // endswitch
  }   // endblock
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );

  return mesh;
}

gfx_mesh_t gfx_create_mesh_from_ply( const char* filename ) {
  assert( filename );

  float* vert_element_data     = NULL;
  float* vbo_data              = NULL;
  int pcount                   = 0;
  int nproperties              = 0;
  gfx_geom_mem_layout_t layout = 0;

  FILE* fin = fopen( filename, "r" );
  if ( !fin ) {
    glog_err( "ERROR: reading mesh from file `%s`\n", filename );
    return gfx_unit_cube_mesh;
  }
  { // file i/o block
    //
    // parse header. NOTE(Anton) not handling the 'ascii' format tag
    char line[1024];
    // only true in the file. output may differ in counts
    int nvertex_elements = 0;
    int nface_elements   = 0;
    while ( fgets( line, 1024, fin ) ) {
      if ( strncmp( line, "element", 7 ) == 0 ) {
        char type[256];
        int count = 0;
        sscanf( line, "element %s %i", type, &count );
        if ( strncmp( type, "vertex", 6 ) == 0 ) {
          nvertex_elements = count;
        } else if ( strncmp( type, "face", 4 ) == 0 ) {
          nface_elements = count;
        }
      } else if ( strncmp( line, "property", 8 ) == 0 ) {
        if ( strncmp( line, "property list", 13 ) != 0 ) { // don't count `property list ...types`
          nproperties++;
        }
      } else if ( strncmp( line, "end_header", 10 ) == 0 ) {
        break;
      }
    }

    if ( 11 != nproperties && 9 != nproperties && 6 != nproperties ) {
      glog_err( "ERROR: mesh file `%s` has %i vertex properties. Should be (x y z nx ny nz s t r g b) (x y z nx ny nz r g b) or (x y z nx ny nz) \n", filename, nproperties );
      fclose( fin );
      return gfx_unit_cube_mesh;
    }
    // scan all the elements into a big buffer of floats -- probably an array of structs would have been nicer
    size_t vert_el_line_sz = nproperties * sizeof( float );
    vert_element_data      = scratch_mem_a( vert_el_line_sz * nvertex_elements );
    assert( vert_element_data );

    for ( int i = 0; i < nvertex_elements; i++ ) {
      int vproperty_idx = i * nproperties;
      if ( !fgets( line, 1024, fin ) ) {
        glog_err( "ERROR: Parsing mesh `%s` vertex data - elements did not match expected number.\n", filename );
        fclose( fin );
        return gfx_unit_cube_mesh;
      }
      if ( 11 == nproperties ) {
        layout  = GFX_MEM_POS_NOR_ST_RGB;
        float x = 0.0f, y = 0.0f, z = 0.0f, nx = 0.0f, ny = 0.0f, nz = 0.0f, s = 0.0f, t = 0.0f;
        int r = 0, g = 0, b = 0;
        int nscanned = sscanf( line, "%f %f %f %f %f %f %f %f %i %i %i", &x, &y, &z, &nx, &ny, &nz, &s, &t, &r, &g, &b );
        if ( nscanned != 11 ) {
          glog_err( "ERROR: Parsing mesh `%s` vertex data - properties did not match expected number.\n", filename );
          fclose( fin );
          return gfx_unit_cube_mesh;
        }
        vert_element_data[vproperty_idx]      = x;
        vert_element_data[vproperty_idx + 1]  = y;
        vert_element_data[vproperty_idx + 2]  = z;
        vert_element_data[vproperty_idx + 3]  = nx;
        vert_element_data[vproperty_idx + 4]  = ny;
        vert_element_data[vproperty_idx + 5]  = nz;
        vert_element_data[vproperty_idx + 6]  = s;
        vert_element_data[vproperty_idx + 7]  = t;
        vert_element_data[vproperty_idx + 8]  = (float)r / 255.0f;
        vert_element_data[vproperty_idx + 9]  = (float)g / 255.0f;
        vert_element_data[vproperty_idx + 10] = (float)b / 255.0f;
      } else if ( 9 == nproperties ) {
        layout  = GFX_MEM_POS_NOR_RGB;
        float x = 0.0f, y = 0.0f, z = 0.0f, nx = 0.0f, ny = 0.0f, nz = 0.0f;
        int r = 0, g = 0, b = 0;
        int nscanned = sscanf( line, "%f %f %f %f %f %f %i %i %i", &x, &y, &z, &nx, &ny, &nz, &r, &g, &b );
        if ( nscanned != 9 ) {
          glog_err( "ERROR: Parsing mesh `%s` vertex data - properties did not match expected number.\n", filename );
          fclose( fin );
          return gfx_unit_cube_mesh;
        }
        vert_element_data[vproperty_idx]     = x;
        vert_element_data[vproperty_idx + 1] = y;
        vert_element_data[vproperty_idx + 2] = z;
        vert_element_data[vproperty_idx + 3] = nx;
        vert_element_data[vproperty_idx + 4] = ny;
        vert_element_data[vproperty_idx + 5] = nz;
        vert_element_data[vproperty_idx + 6] = (float)r / 255.0f;
        vert_element_data[vproperty_idx + 7] = (float)g / 255.0f;
        vert_element_data[vproperty_idx + 8] = (float)b / 255.0f;
      } else {
        layout  = GFX_MEM_POS_NOR;
        float x = 0.0f, y = 0.0f, z = 0.0f, nx = 0.0f, ny = 0.0f, nz = 0.0f;
        int nscanned = sscanf( line, "%f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz );
        if ( nscanned != 6 ) {
          glog_err( "ERROR: Parsing mesh `%s` vertex data - properties did not match expected number.\n", filename );
          fclose( fin );
          return gfx_unit_cube_mesh;
        }
        vert_element_data[vproperty_idx]     = x;
        vert_element_data[vproperty_idx + 1] = y;
        vert_element_data[vproperty_idx + 2] = z;
        vert_element_data[vproperty_idx + 3] = nx;
        vert_element_data[vproperty_idx + 4] = ny;
        vert_element_data[vproperty_idx + 5] = nz;
      }
    }

    size_t vbo_data_reserved_sz = sizeof( float ) * nproperties * 3 * nface_elements * 2; // x2 just in case it's all quads
    vbo_data                    = scratch_mem_b( vbo_data_reserved_sz );
    if ( !vbo_data ) {
      glog_err( "ERROR: Out of memory. Could not allocate memory to load mesh `%s`\n", filename );
      fclose( fin );
      return gfx_unit_cube_mesh;
    }
    int vbo_float_idx = 0;
    for ( int i = 0; i < nface_elements; i++ ) {
      char* dummy_out = fgets( line, 1024, fin ); // TODO safety check
      APG_UNUSED( dummy_out );
      int nvert_element_idxs = 0, idx_a = 0, idx_b = 0, idx_c = 0, idx_d = 0;
      int nscanned = sscanf( line, "%i %i %i %i %i", &nvert_element_idxs, &idx_a, &idx_b, &idx_c, &idx_d );

      // GFX_TRIANGLES in
      if ( 3 == nvert_element_idxs ) {
        if ( nscanned != 4 ) {
          glog_err( "ERROR: triangle face in mesh `%s` has missing tokens (%i/4) for face #%i\n", filename, nscanned, i );
          fclose( fin );
          return gfx_unit_cube_mesh;
        }
        int vert_element_float_idx = idx_a * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_b * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_c * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;

        pcount += 3;

        // quads in
      } else if ( 4 == nvert_element_idxs ) {
        if ( nscanned != 5 ) {
          glog_err( "ERROR: quad face in mesh `%s` has missing tokens (%i/5) for face #%i\n", filename, nscanned, i );
          fclose( fin );
          return gfx_unit_cube_mesh;
        }
        int vert_element_float_idx = idx_a * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_b * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_c * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_c * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_d * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;
        vert_element_float_idx = idx_a * nproperties;
        memcpy( &vbo_data[vbo_float_idx], &vert_element_data[vert_element_float_idx], sizeof( float ) * nproperties );
        vbo_float_idx += nproperties;

        pcount += 6;

        // unexpected
      } else {
        glog_err( "ERROR: mesh `%s` has unhandled number of vertex elements (%i) for face #%i\n", filename, nvert_element_idxs, i );
        fclose( fin );
        return gfx_unit_cube_mesh;
      } // endif handle quads
    }   // endfor faces
  }     // end of file i/o block
  fclose( fin );

  size_t vbo_data_used_sz = sizeof( float ) * pcount * nproperties;
  gfx_mesh_t mesh         = gfx_create_mesh( vbo_data, vbo_data_used_sz, layout, pcount, GFX_STATIC_DRAW, GFX_TRIANGLES );
  strncat( mesh.filename, filename, GFX_MAX_MESH_FILENAME - 1 );

  return mesh;
}

void gfx_update_mesh( gfx_mesh_t* mesh, const void* data, size_t sz, size_t n_verts ) {
  assert( mesh );
  assert( data );

  GLenum gl_draw_mode = 0;
  switch ( mesh->draw_mode ) {
  case GFX_STATIC_DRAW: {
    gl_draw_mode = GL_STATIC_DRAW;
  } break;
  case GFX_DYNAMIC_DRAW: {
    gl_draw_mode = GL_DYNAMIC_DRAW;
  } break;
  case GFX_STREAM_DRAW: {
    gl_draw_mode = GL_STREAM_DRAW;
  } break;
  }

  mesh->n_verts = n_verts;

  // TODO(Anton) can't be called from thread -- needs to be done in cleanup for threaded version
  glBindBuffer( GL_ARRAY_BUFFER, mesh->vbo );
  glBufferData( GL_ARRAY_BUFFER, sz, data, gl_draw_mode );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void gfx_reload_mesh( gfx_mesh_t* mesh ) {
  assert( mesh );

  if ( '\0' == mesh->filename[0] ) { return; }
  printf( " reloading mesh %s\n", mesh->filename );

  gfx_mesh_t tmp = *mesh;

  // delete but don't delete the fallback by accident
  if ( gfx_ss_quad_mesh.vao != mesh->vao && gfx_unit_cube_mesh.vao != mesh->vao ) { gfx_delete_mesh( mesh ); }

  *mesh             = gfx_create_mesh_from_ply( tmp.filename );
  mesh->filename[0] = '\0';
  strncat( mesh->filename, tmp.filename, GFX_MAX_MESH_FILENAME - 1 ); // just in case reverted to fallback's filename
}

void gfx_delete_mesh( gfx_mesh_t* mesh ) {
  assert( mesh );

  glDeleteBuffers( 1, &mesh->vbo );
  glDeleteVertexArrays( 1, &mesh->vao );
  memset( mesh, 0, sizeof( gfx_mesh_t ) );
}

// =================================================================================================
//                                           Shaders
// =================================================================================================
gfx_shader_t gfx_fallback_shader;
gfx_shader_t gfx_managed_shaders[GFX_MAX_MANAGED_SHADERS];
int g_n_managed_shaders;

// doesn't revert to fallback shader - you do that on fail
static bool _create_shader_from_strings( const char* vs_str, const char* fs_str, gfx_shader_t* shader_out ) {
  assert( vs_str && fs_str && shader_out );

  gfx_shader_t shader;
  memset( &shader, 0, sizeof( gfx_shader_t ) );

  shader.vertex_shader = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( shader.vertex_shader, 1, &vs_str, NULL );
  glCompileShader( shader.vertex_shader );
  int params = -1;
  glGetShaderiv( shader.vertex_shader, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    glog_err( "ERROR: vertex shader index %u did not compile\n", shader.vertex_shader );
    int max_length    = 2048;
    int actual_length = 0;
    char slog[2048];
    glGetShaderInfoLog( shader.vertex_shader, max_length, &actual_length, slog );
    glog_err( "shader info log for GL index %u:\n%s\n", shader.vertex_shader, slog );
    if ( 0 != shader.vertex_shader ) {
      glDeleteShader( shader.vertex_shader );
      shader.vertex_shader = 0;
    }
    return false;
  }

  shader.fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( shader.fragment_shader, 1, &fs_str, NULL );
  glCompileShader( shader.fragment_shader );
  params = -1;
  glGetShaderiv( shader.fragment_shader, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    glog_err( "ERROR: fragment shader index %u did not compile\n", shader.fragment_shader );
    int max_length    = 2048;
    int actual_length = 0;
    char slog[2048];
    glGetShaderInfoLog( shader.fragment_shader, max_length, &actual_length, slog );
    glog_err( "shader info log for GL index %u:\n%s\n", shader.fragment_shader, slog );
    if ( 0 != shader.fragment_shader ) {
      glDeleteShader( shader.fragment_shader );
      shader.fragment_shader = 0;
    }
    return false;
  }

  shader.program = glCreateProgram();
  glAttachShader( shader.program, shader.vertex_shader );
  glAttachShader( shader.program, shader.fragment_shader );

  glBindAttribLocation( shader.program, GFX_A_POS_LOCATION, "a_pos" );
  glBindAttribLocation( shader.program, GFX_A_ST_LOCATION, "a_st" );
  glBindAttribLocation( shader.program, GFX_A_NORM_LOCATION, "a_norm" );
  glBindAttribLocation( shader.program, GFX_A_COLOUR_LOCATION, "a_colour" );
  glBindAttribLocation( shader.program, GFX_A_PICKINDEX_LOCATION, "a_pickindex" );
  glBindAttribLocation( shader.program, GFX_A_HTAG_LOCATION, "a_htag" );
  glBindAttribLocation( shader.program, GFX_A_K_LOCATION, "a_k" );
  glBindAttribLocation( shader.program, GFX_A_EXPLORED_FACTOR, "GFX_A_EXPLORED_FACTOR" );
  glBindFragDataLocation( shader.program, 0, "o_frag_colour" );
  glBindFragDataLocation( shader.program, 1, "o_second_output_colour" );
  glBindFragDataLocation( shader.program, 2, "o_third_output_colour" );
  glBindFragDataLocation( shader.program, 3, "o_fourth_output_colour" );
  glLinkProgram( shader.program );

  { // get typical normals
    shader.u_main_scene_texture_loc   = glGetUniformLocation( shader.program, "u_main_scene_texture" );
    shader.u_secondary_texture_loc    = glGetUniformLocation( shader.program, "u_secondary_texture" );
    shader.u_tertiary_texture_loc     = glGetUniformLocation( shader.program, "u_tertiary_texture" );
    shader.u_day_gradient_texture_loc = glGetUniformLocation( shader.program, "u_day_gradient_texture" );
    shader.u_progress_factor_loc      = glGetUniformLocation( shader.program, "u_progress_factor" );
    shader.u_colour_loc               = glGetUniformLocation( shader.program, "u_colour" );
    shader.u_background_colour_loc    = glGetUniformLocation( shader.program, "u_background_colour" );
    shader.u_scale_loc                = glGetUniformLocation( shader.program, "u_scale" );
    shader.u_st_scale_loc             = glGetUniformLocation( shader.program, "u_st_scale" );
    shader.u_opacity_loc              = glGetUniformLocation( shader.program, "u_opacity" );
    shader.u_pos_loc                  = glGetUniformLocation( shader.program, "u_pos" );
    shader.u_time_loc                 = glGetUniformLocation( shader.program, "u_time" );
    shader.u_walk_time_loc            = glGetUniformLocation( shader.program, "u_walk_time" );
    shader.u_framebuffer_dims_loc     = glGetUniformLocation( shader.program, "u_framebuffer_dims" );
    shader.u_P_loc                    = glGetUniformLocation( shader.program, "u_P" );
    shader.u_V_loc                    = glGetUniformLocation( shader.program, "u_V" );
    shader.u_M_loc                    = glGetUniformLocation( shader.program, "u_M" );
    shader.u_PV_loc                   = glGetUniformLocation( shader.program, "u_PV" );
    shader.u_PVM_loc                  = glGetUniformLocation( shader.program, "u_PVM" );
    shader.u_scroll_val_loc           = glGetUniformLocation( shader.program, "u_scroll_val" );
    shader.u_tile_index_loc           = glGetUniformLocation( shader.program, "u_tile_index" );
    shader.u_sun_dir_wor_loc          = glGetUniformLocation( shader.program, "u_sun_dir_wor" );
    shader.u_day_gradient_factor_loc  = glGetUniformLocation( shader.program, "u_day_gradient_factor" );
  }
  {
    float ident[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    if ( shader.u_P_loc > -1 ) { gfx_uniform_mat4( shader, shader.u_P_loc, ident ); }
    if ( shader.u_V_loc > -1 ) { gfx_uniform_mat4( shader, shader.u_V_loc, ident ); }
    if ( shader.u_M_loc > -1 ) { gfx_uniform_mat4( shader, shader.u_M_loc, ident ); }
    if ( shader.u_PV_loc > -1 ) { gfx_uniform_mat4( shader, shader.u_PV_loc, ident ); }
    if ( shader.u_PVM_loc > -1 ) { gfx_uniform_mat4( shader, shader.u_PVM_loc, ident ); }
    if ( shader.u_secondary_texture_loc > -1 ) { gfx_uniform_1i( shader, shader.u_secondary_texture_loc, 1 ); }
    if ( shader.u_tertiary_texture_loc > -1 ) { gfx_uniform_1i( shader, shader.u_tertiary_texture_loc, 2 ); }
    if ( shader.u_day_gradient_texture_loc > -1 ) { gfx_uniform_1i( shader, shader.u_day_gradient_texture_loc, 1 ); }
  }

  params = -1;
  glGetProgramiv( shader.program, GL_LINK_STATUS, &params );
  if ( GL_TRUE != params ) {
    glog_err( "ERROR: could not link shader program GL index %u\n", shader.program );
    int max_length    = 2048;
    int actual_length = 0;
    char plog[2048];
    glGetProgramInfoLog( shader.program, max_length, &actual_length, plog );
    glog_err( "program info log for GL index %u:\n%s", shader.program, plog );
    gfx_delete_shader( &shader );
    return false;
  }

  *shader_out = shader;
  return true;
}

static gfx_shader_t _create_shader_from_files( const char* vertex_shader_filename, const char* fragment_shader_filename ) {
  assert( vertex_shader_filename && fragment_shader_filename );

  glog( "loading shader from `%s`, `%s`\n", vertex_shader_filename, fragment_shader_filename );

#define MAX_SHADER_SZ 100000
  char vs_shader_str[MAX_SHADER_SZ];
  char fs_shader_str[MAX_SHADER_SZ];
  vs_shader_str[0] = fs_shader_str[0] = '\0';
  { // read vertex shader file into a buffer
    FILE* fp = fopen( vertex_shader_filename, "r" );
    if ( !fp ) {
      fprintf( stderr, "ERROR: could not open vertex shader file `%s`\n", vertex_shader_filename );
      return gfx_fallback_shader;
    }
    size_t count = fread( vs_shader_str, 1, MAX_SHADER_SZ - 1, fp );
    assert( count < MAX_SHADER_SZ - 1 ); // file was too long
    vs_shader_str[count] = '\0';
    fclose( fp );
  }
  { // read fragment shader file into a buffer
    FILE* fp = fopen( fragment_shader_filename, "r" );
    if ( !fp ) {
      fprintf( stderr, "ERROR: could not open fragment shader file `%s`\n", fragment_shader_filename );
      return gfx_fallback_shader;
    }
    size_t count = fread( fs_shader_str, 1, MAX_SHADER_SZ - 1, fp );
    assert( count < MAX_SHADER_SZ - 1 ); // file was too long
    fs_shader_str[count] = '\0';
    fclose( fp );
  }

  gfx_shader_t shader;
  bool success = _create_shader_from_strings( vs_shader_str, fs_shader_str, &shader );
  strncat( shader.vs_filename, vertex_shader_filename, GFX_MAX_SHADER_FILENAME - 1 );
  strncat( shader.fs_filename, fragment_shader_filename, GFX_MAX_SHADER_FILENAME - 1 );
  if ( success ) { return shader; }
  return gfx_fallback_shader;
}

static void _init_fallback_shader() {
  // TODO uniform block for camera like in Storm
  const char* vs_str =
    "#version 410\n"
    "in vec3 a_pos;\n"
    "uniform mat4 u_P, u_V;\n"
    "uniform mat4 u_M;"
    "void main() {\n"
    "	gl_Position = u_P * u_V * u_M * vec4( a_pos * 0.1, 1.0 );\n"
    "}\n";
  const char* fs_str =
    "#version 410\n"
    "out vec4 o_frag_colour;\n"
    "void main() {\n"
    "	o_frag_colour = vec4( 1.0, 0.5, 0.5, 1.0 );\n"
    "}\n";
  bool success = _create_shader_from_strings( vs_str, fs_str, &gfx_fallback_shader );
  assert( success );
}

void gfx_delete_shader( gfx_shader_t* shader ) {
  assert( shader );

  if ( 0 != shader->vertex_shader ) { glDeleteShader( shader->vertex_shader ); }
  if ( 0 != shader->fragment_shader ) { glDeleteShader( shader->fragment_shader ); }
  if ( 0 != shader->program ) { glDeleteProgram( shader->program ); }
  memset( shader, 0, sizeof( gfx_shader_t ) );
}

void gfx_reload_shader( gfx_shader_t* shader ) {
  assert( shader );

  gfx_shader_t tmp = *shader;

  // delete but don't delete the fallback shader by accident
  if ( gfx_fallback_shader.program != shader->program ) { gfx_delete_shader( shader ); }

  *shader = _create_shader_from_files( tmp.vs_filename, tmp.fs_filename );
  // write into shader again, just in case was replace with fallback here on a fail
  shader->vs_filename[0] = '\0';
  shader->fs_filename[0] = '\0';
  strncat( shader->vs_filename, tmp.vs_filename, GFX_MAX_SHADER_FILENAME - 1 );
  strncat( shader->fs_filename, tmp.fs_filename, GFX_MAX_SHADER_FILENAME - 1 );
}

int gfx_uniform_loc( gfx_shader_t shader, const char* name ) {
  assert( name );
  assert( shader.program );

  return glGetUniformLocation( shader.program, name );
}

void gfx_uniform_mat4( gfx_shader_t shader, int loc, const float* m ) {
  assert( m );
  if ( loc < 0 ) { return; }

  glProgramUniformMatrix4fv( shader.program, loc, 1, GL_FALSE, m );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_1i( gfx_shader_t shader, int loc, int x ) {
  if ( loc < 0 ) { return; }

  glProgramUniform1i( shader.program, loc, x );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_1f( gfx_shader_t shader, int loc, float x ) {
  if ( loc < 0 ) { return; }

  glProgramUniform1f( shader.program, loc, x );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_2f( gfx_shader_t shader, int loc, float x, float y ) {
  if ( loc < 0 ) { return; }

  glProgramUniform2f( shader.program, loc, x, y );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_3f( gfx_shader_t shader, int loc, float x, float y, float z ) {
  if ( loc < 0 ) { return; }

  glProgramUniform3f( shader.program, loc, x, y, z );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_3fv( gfx_shader_t shader, int loc, int count, float* v ) {
  if ( loc < 0 ) { return; }
  assert( v );

  glProgramUniform3fv( shader.program, loc, count, v );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_4f( gfx_shader_t shader, int loc, float x, float y, float z, float w ) {
  if ( loc < 0 ) { return; }

  glProgramUniform4f( shader.program, loc, x, y, z, w );
  gfx_framestats.n_uniforms++;
}

void gfx_uniform_4fv( gfx_shader_t shader, int loc, int count, float* v ) {
  if ( loc < 0 ) { return; }
  assert( v );

  glProgramUniform4fv( shader.program, loc, count, v );
  gfx_framestats.n_uniforms++;
}

int gfx_managed_shader_find_index( const char* vs_filename, const char* fs_filename ) {
  if ( !vs_filename || !fs_filename ) { return -1; }

  for ( int i = 0; i < g_n_managed_shaders; i++ ) {
    if ( 0 == strncmp( vs_filename, gfx_managed_shaders[i].vs_filename, GFX_MAX_SHADER_FILENAME ) &&
         0 == strncmp( fs_filename, gfx_managed_shaders[i].fs_filename, GFX_MAX_SHADER_FILENAME ) ) {
      glog( "shaders for `%s` & `%s` already managed. reusing...\n", vs_filename, fs_filename );
      return i;
    }
  }
  return -1;
}

int gfx_create_managed_shader_from_files( const char* vs_filename, const char* fs_filename ) {
  assert( g_n_managed_shaders < GFX_MAX_MANAGED_SHADERS );
  assert( vs_filename && fs_filename );

  gfx_managed_shaders[g_n_managed_shaders] = _create_shader_from_files( vs_filename, fs_filename );
  return g_n_managed_shaders++;
}

void gfx_reload_all_managed_shaders() {
  for ( int i = 0; i < g_n_managed_shaders; i++ ) { gfx_reload_shader( &gfx_managed_shaders[i] ); }
}

void gfx_delete_all_managed_shaders() {
  for ( int i = 0; i < g_n_managed_shaders; i++ ) {
    // avoid continuously deleting the same fallback shader if set several times across shaders
    if ( gfx_fallback_shader.program == gfx_managed_shaders[i].program ) { continue; }
    gfx_delete_shader( &gfx_managed_shaders[i] );
  }
  g_n_managed_shaders = 0;
}

// =================================================================================================
//                                           Textures
// =================================================================================================
gfx_texture_t gfx_fallback_texture;
gfx_texture_t gfx_managed_textures[MAX_MANAGED_TEXTURES];
int gfx_n_managed_textures;

int gfx_managed_texture_find_index( const char* filename ) {
  for ( int i = 0; i < gfx_n_managed_textures; i++ ) {
    if ( 0 == strncmp( filename, gfx_managed_textures[i].filename, GFX_MAX_TEXTURE_FILENAME ) ) { return i; }
  }
  return -1;
}

int gfx_create_managed_texture_from_file( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb ) {
  assert( gfx_n_managed_textures < MAX_MANAGED_TEXTURES );
  assert( filename );

  int idx = gfx_managed_texture_find_index( filename );
  if ( idx >= 0 ) {
    glog( "texture for `%s` already managed. reusing...\n", filename );
    return idx;
  }

  //  load_image_file_to_gfx_texture_threaded( filename, clamp, mag_linear, min_linear, mipmaps, srgb, &gfx_managed_textures[gfx_n_managed_textures] );
  gfx_managed_textures[gfx_n_managed_textures] = gfx_load_image_file_to_texture_serial( filename, clamp, mag_linear, min_linear, mipmaps, srgb );

  return gfx_n_managed_textures++;
}

int gfx_create_managed_texture_from_image_mem( const unsigned char* pixels, int w, int h, int n_chans, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb ) {
  assert( gfx_n_managed_textures < MAX_MANAGED_TEXTURES );
  assert( pixels );

  gfx_managed_textures[gfx_n_managed_textures] = gfx_load_image_mem_to_texture( pixels, w, h, n_chans, clamp, mag_linear, min_linear, mipmaps, srgb );
  return gfx_n_managed_textures++;
}

void gfx_reload_all_managed_textures() {
  for ( int i = 0; i < gfx_n_managed_textures; i++ ) { gfx_reload_texture( &gfx_managed_textures[i] ); }
}

void gfx_delete_all_managed_textures() {
  for ( int i = 0; i < gfx_n_managed_textures; i++ ) {
    // avoid continuously deleting the same fallback if set several times
    if ( gfx_fallback_texture.handle == gfx_managed_textures[i].handle ) { continue; }
    gfx_delete_texture( &gfx_managed_textures[i] );
  }
  gfx_n_managed_textures = 0;
}

void gfx_reload_texture( gfx_texture_t* texture ) {
  assert( texture );

  if ( '\0' == texture->filename[0] ) { return; }

  gfx_texture_t tmp = *texture;

  // delete but don't delete the fallback texture by accident
  if ( gfx_fallback_texture.handle != texture->handle ) { gfx_delete_texture( texture ); }

  // load_image_file_to_gfx_texture_threaded( tmp.filename, tmp.clamp, tmp.mag_linear, tmp.min_linear, tmp.mipmaps, tmp.srgb, texture );
  *texture = gfx_load_image_file_to_texture_serial( tmp.filename, tmp.clamp, tmp.mag_linear, tmp.min_linear, tmp.mipmaps, tmp.srgb );
}

void gfx_update_texture( gfx_texture_t* texture, const unsigned char* pixels ) {
  assert( texture && texture->handle );

  GLint internal_format = GL_RGBA;
  GLenum format         = GL_RGBA;
  GLint wrap_param      = texture->clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
  GLint mag_filter      = texture->mag_linear ? GL_LINEAR : GL_NEAREST;
  GLint min_filter      = texture->min_linear ? GL_LINEAR : GL_NEAREST;
  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
  switch ( texture->n_chans ) {
  case 4: {
    internal_format = texture->srgb ? GL_SRGB_ALPHA : GL_RGBA;
    format          = GL_RGBA;
  } break;
  case 3: {
    internal_format = texture->srgb ? GL_SRGB : GL_RGB;
    format          = GL_RGB;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // for small 1-channel npot images and framebuffer reading
  } break;
  case 1: {
    internal_format = GL_RED;
    format          = GL_RED;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // for small 1-channel npot images and framebuffer reading
  } break;
  default: {
    glog_err( "WARNING: unhandled texture channel number: %i\n", texture->n_chans );
    gfx_delete_texture( texture );
    *texture = gfx_fallback_texture;
    return;
  } break;
  } // endswitch
    // NOTE can use 32-bit, GL_FLOAT depth component for eg DOF
  if ( texture->is_depth ) {
    internal_format = GL_DEPTH_COMPONENT;
    format          = GL_DEPTH_COMPONENT;
  }

  glBindTexture( GL_TEXTURE_2D, texture->handle );
  {
    glTexImage2D( GL_TEXTURE_2D, 0, internal_format, texture->w, texture->h, 0, format, GL_UNSIGNED_BYTE, pixels );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_param );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_param );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
    if ( texture->mipmaps ) {
      min_filter = texture->min_linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
      glGenerateMipmap( GL_TEXTURE_2D );
    }
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter ); // depends on change by mipmap, above
    /* produces intense moire pattern so commented out
    GLfloat max_aniso = 0.0f;
    glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso );*/
  }
  glBindTexture( GL_TEXTURE_2D, 0 );
}

// TODO(Anton) rename: create texture from image mem
gfx_texture_t
gfx_load_image_mem_to_texture( const unsigned char* pixels, int w, int h, int n_chans, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb ) {
  assert( 4 == n_chans || 3 == n_chans || 1 == n_chans ); // 2 not used yet so not impl

  gfx_texture_t texture = { .handle = 0, .w = w, .h = h, .n_chans = n_chans, .clamp = clamp, .mag_linear = mag_linear, .min_linear = min_linear, .mipmaps = mipmaps, .srgb = srgb };
  glGenTextures( 1, &texture.handle );
  gfx_update_texture( &texture, pixels );

  return texture;
}

// single-threaded version.
// 0. gen opengl handle first (blank texture)
// 1. stb bits -> thread func
// 2. opengl bits & free mem -> on done func. upload texture
// 3. may not need update func - let them pop in
gfx_texture_t gfx_load_image_file_to_texture_serial( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb ) {
  assert( filename );

  gfx_texture_t texture;

  int width = 0, height = 0, comps = 0, req_comps = 0;
  unsigned char* pixels = stbi_load( filename, &width, &height, &comps, req_comps );
  stbi__vertical_flip( pixels, width, height, comps );
  if ( !pixels ) {
    glog_err( "ERROR: loading image from file `%s`\n", filename );
    return gfx_fallback_texture;
  }
  {
    texture = gfx_load_image_mem_to_texture( pixels, width, height, comps, clamp, mag_linear, min_linear, mipmaps, srgb );
    strncpy( texture.filename, filename, GFX_MAX_TEXTURE_FILENAME - 1 );

    glog( "loaded texture %s with %ix%i@%i\n", filename, width, height, comps );
  }
  stbi_image_free( pixels );

  return texture;
}

static void _load_texture_sr( int worker_idx, void* args ) {
  struct gfx_texture_t* texture_ptr = (struct gfx_texture_t*)args;
  glog( "loading `%s` on worker %i\n", texture_ptr->filename, worker_idx );
  texture_ptr->pixels_ptr = stbi_load( texture_ptr->filename, &texture_ptr->w, &texture_ptr->h, &texture_ptr->n_chans, 0 );
  stbi__vertical_flip( texture_ptr->pixels_ptr, texture_ptr->w, texture_ptr->h, texture_ptr->n_chans );
}

static void _load_texture_finished_cb( const char* name, void* args ) {
  APG_UNUSED( name );
  struct gfx_texture_t* texture_ptr = (struct gfx_texture_t*)args;

  if ( !texture_ptr->pixels_ptr ) {
    glog_err( "WARNING: image failed to load from file `%s`\n", texture_ptr->filename );
    gfx_delete_texture( texture_ptr );
    // TODO(Anton) replace internals? to use default texture
    // *texture = gfx_fallback_texture;
    return;
  }

  GLint internal_format = GL_RGBA;
  GLenum format         = GL_RGBA;
  GLint wrap_param      = texture_ptr->clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT;
  GLint mag_filter      = texture_ptr->mag_linear ? GL_LINEAR : GL_NEAREST;
  GLint min_filter      = texture_ptr->min_linear ? GL_LINEAR : GL_NEAREST;
  glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
  switch ( texture_ptr->n_chans ) {
  case 4: {
    internal_format = texture_ptr->srgb ? GL_SRGB_ALPHA : GL_RGBA;
    format          = GL_RGBA;
  } break;
  case 3: {
    internal_format = texture_ptr->srgb ? GL_SRGB : GL_RGB;
    format          = GL_RGB;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // for small 1-channel npot images and framebuffer reading
  } break;
  case 1: {
    internal_format = GL_RED;
    format          = GL_RED;
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // for small 1-channel npot images and framebuffer reading
  } break;
  default: {
    glog_err( "WARNING: unhandled texture channel number: %i\n", texture_ptr->n_chans );
    gfx_delete_texture( texture_ptr );
    // TODO(Anton) replace internals? to use default texture
    // *texture = gfx_fallback_texture;
    return;
  } break;
  } // endswitch
    // NOTE can use 32-bit, GL_FLOAT depth component for eg DOF
  if ( texture_ptr->is_depth ) {
    internal_format = GL_DEPTH_COMPONENT;
    format          = GL_DEPTH_COMPONENT;
  }

  // can only upload to OpenGL on main thread
  glBindTexture( GL_TEXTURE_2D, texture_ptr->handle );
  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, texture_ptr->w, texture_ptr->h, 0, format, GL_UNSIGNED_BYTE, texture_ptr->pixels_ptr );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_param );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_param );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
  if ( texture_ptr->mipmaps ) {
    min_filter = texture_ptr->min_linear ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
    glGenerateMipmap( GL_TEXTURE_2D );
  }
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter ); // depends on change by mipmap, above
  glBindTexture( GL_TEXTURE_2D, 0 );

  stbi_image_free( texture_ptr->pixels_ptr );

  texture_ptr->loaded = true;

  // printf( "job finished %s for handlegl %i\n", name, texture_ptr->handle );
}

#ifdef _THREADING_SUPPORT
void gfx_load_image_file_to_gfx_texture_threaded( const char* filename, bool clamp, bool mag_linear, bool min_linear, bool mipmaps, bool srgb, struct gfx_texture_t* texture ) {
  assert( filename && texture );

  memset( texture, 0, sizeof( gfx_texture_t ) );
  strncat( texture->filename, filename, 255 );
  texture->clamp      = clamp;
  texture->mag_linear = mag_linear;
  texture->min_linear = min_linear;
  texture->mipmaps    = mipmaps;
  texture->srgb       = srgb;

  glGenTextures( 1, &texture->handle );
  glBindTexture( GL_TEXTURE_2D, texture->handle );
  {
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
    // actual parameters set and mipmaps generated after image loaded
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  }
  glBindTexture( GL_TEXTURE_2D, 0 );

  job_description_t job;
  memset( &job, 0, sizeof( job_description_t ) );
  job.job_function_ptr  = _load_texture_sr;
  job.job_function_args = texture;
  job.on_finished_cb    = _load_texture_finished_cb;
  strncat( job.name, filename, MAX_THREAD_JOB_NAME - 1 );
  // printf( "starting job - texture %s\n", texture->filename );
  worker_pool_push_job( job );
}
#endif

void gfx_delete_texture( gfx_texture_t* texture ) {
  assert( texture );

  glDeleteTextures( 1, &texture->handle );
  memset( texture, 0, sizeof( gfx_texture_t ) );
}

// =================================================================================================
//                                          Draw Helpers
// =================================================================================================
void gfx_draw_mesh( gfx_mesh_t mesh, gfx_shader_t shader ) {
  GLenum mode = GL_TRIANGLES;
  if ( GFX_TRIANGLE_STRIP == mesh.polygon_type ) { mode = GL_TRIANGLE_STRIP; }
  glUseProgram( shader.program );
  {
    glBindVertexArray( mesh.vao );
    glDrawArrays( mode, 0, mesh.n_verts );
    gfx_framestats.n_draws++;
    gfx_framestats.n_verts += mesh.n_verts;
    glBindVertexArray( 0 );
  }
  glUseProgram( 0 );
}

void gfx_draw_gfx_mesh_texturedv( gfx_mesh_t mesh, gfx_shader_t shader, gfx_texture_t* textures, int ntextures ) {
  assert( textures );

  GLenum mode = GL_TRIANGLES;
  if ( GFX_TRIANGLE_STRIP == mesh.polygon_type ) { mode = GL_TRIANGLE_STRIP; }

  glUseProgram( shader.program );
  for ( int i = 0; i < ntextures; i++ ) {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture( GL_TEXTURE_2D, textures[i].handle );
  }
  {
    glBindVertexArray( mesh.vao );
    glDrawArrays( mode, 0, mesh.n_verts );
    gfx_framestats.n_draws++;
    gfx_framestats.n_verts += mesh.n_verts;
    glBindVertexArray( 0 );
  }
  for ( int i = 0; i < ntextures; i++ ) {
    glActiveTexture( GL_TEXTURE0 + i );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
  glUseProgram( 0 );
}

// =================================================================================================
//                                           Post Processing
// =================================================================================================
bool gfx_create_framebuffer( gfx_framebuffer_meta_t* fb_meta ) {
  assert( fb_meta );
  assert( fb_meta->width > 0 );
  assert( fb_meta->height > 0 );

  fb_meta->x_scale = (float)fb_meta->width / (float)gfx_fb_width;
  fb_meta->y_scale = (float)fb_meta->height / (float)gfx_fb_height;
  {
    glGenFramebuffers( 1, &fb_meta->handle );
    for ( int i = 0; i < fb_meta->n_output_textures; i++ ) {
      fb_meta->output_textures[i] = ( gfx_texture_t ){ .w = fb_meta->width, .h = fb_meta->height, .n_chans = 3, .clamp = true };
      glGenTextures( fb_meta->n_output_textures, &fb_meta->output_textures[i].handle );
    }
    fb_meta->depth_texture = ( gfx_texture_t ){ .w = fb_meta->width, .h = fb_meta->height, .n_chans = 1, .clamp = true, .is_depth = true };
    glGenTextures( 1, &fb_meta->depth_texture.handle );

    if ( !gfx_rebuild_framebuffer( fb_meta ) ) { return false; }
  }

  return true;
}

// was ~1.5ms before fullscreen -> 1.4 without all the depth32 textures. -> 1.3 with /2 sized refraction map
bool gfx_rebuild_framebuffer( gfx_framebuffer_meta_t* fb_meta ) {
  {
    assert( fb_meta );
    assert( fb_meta->width > 0 );
    assert( fb_meta->height > 0 );
    assert( fb_meta->handle > 0 );
    assert( fb_meta->depth_texture.handle > 0 );
  }
  int w = fb_meta->width = (int)( (float)fb_meta->x_scale * (float)gfx_fb_width );
  int h = fb_meta->height = (int)( (float)fb_meta->y_scale * (float)gfx_fb_height );
  glBindFramebuffer( GL_FRAMEBUFFER, fb_meta->handle );
  {
    for ( int i = 0; i < fb_meta->n_output_textures; i++ ) {
      fb_meta->output_textures[i].w = w;
      fb_meta->output_textures[i].h = h;
      gfx_update_texture( &fb_meta->output_textures[i], NULL );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fb_meta->output_textures[i].handle, 0 );
    } // endfor output textures
    {
      fb_meta->depth_texture.w = w;
      fb_meta->depth_texture.h = h;
      gfx_update_texture( &fb_meta->depth_texture, NULL );
      glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb_meta->depth_texture.handle, 0 );
    }
    GLenum draw_bufs[GFX_MAX_FB_OUTPUTS];
    for ( int i = 0; i < GFX_MAX_FB_OUTPUTS; i++ ) { draw_bufs[i] = GL_COLOR_ATTACHMENT0 + i; }
    glDrawBuffers( fb_meta->n_output_textures, draw_bufs );

    {
      GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
      if ( GL_FRAMEBUFFER_COMPLETE != status ) {
        glog_err( "ERROR: incomplete framebuffer\n" );
        if ( GL_FRAMEBUFFER_UNDEFINED == status ) {
          glog_err( "GL_FRAMEBUFFER_UNDEFINED\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n" );
        } else if ( GL_FRAMEBUFFER_UNSUPPORTED == status ) {
          glog_err( "GL_FRAMEBUFFER_UNSUPPORTED\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n" );
        } else if ( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS == status ) {
          glog_err( "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n" );
        } else {
          glog_err( "glCheckFramebufferStatus unspecified error\n" );
        }
        return false;
      }
    } // endblock checks
  }
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  return true;
}

void gfx_bind_framebuffer_with_viewport( const gfx_framebuffer_meta_t* fb_meta ) {
  if ( !fb_meta ) {
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( 0, 0, gfx_fb_width, gfx_fb_height );
    return;
  }

  glBindFramebuffer( GL_FRAMEBUFFER, fb_meta->handle );
  int w = fb_meta->width;
  int h = fb_meta->height;
  glViewport( 0, 0, w, h );
}

bool gfx_read_idx_from_fb( const gfx_framebuffer_meta_t* fb_meta, uint32_t* hovered_idx ) {
  assert( fb_meta );
  assert( hovered_idx );
  // happens when window minimised
  if ( gfx_win_width <= 0 || gfx_win_height <= 0 ) { return false; }

  double xpos, ypos;
  glfwGetCursorPos( g_window, &xpos, &ypos );
  float w_fac = fb_meta->width / gfx_win_width;
  float h_fac = fb_meta->height / gfx_win_height;
  int mx      = (int)( xpos * w_fac );
  int my      = (int)( fb_meta->height - ypos * h_fac );
  if ( mx < 0 || my < 0 || mx >= fb_meta->width || my >= fb_meta->height ) { return false; }

  float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
  gfx_get_buffer_colour( fb_meta, &r, &g, &b, &a );
  const uint32_t background_colour = ( r * 255 ) + ( g * 255 ) * 256 + ( b * 255 ) * 256 * 256;

  uint32_t idx = 0;
  glBindFramebuffer( GL_FRAMEBUFFER, fb_meta->handle );
  glReadBuffer( GL_COLOR_ATTACHMENT2 );
  {
    unsigned char data[4] = { 0, 0, 0, 0 };

    glPixelStorei( GL_PACK_ALIGNMENT, 1 );                          // for irregular display sizes in RGB
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );                        // affects glReadPixels and subsequent texture calls alignment format
    glReadPixels( mx, my, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data ); // Note(Anton) RGBA
    idx = decode_rgb_to_idx( data[0], data[1], data[2] );
    if ( background_colour == idx ) { return false; }
  }
  glReadBuffer( GL_COLOR_ATTACHMENT0 );
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  *hovered_idx = idx;
  return true;
}

// =================================================================================================
//                                              Time
// =================================================================================================
double gfx_get_time_s() { return glfwGetTime(); }

// =================================================================================================
//                                           Context
// =================================================================================================
static float _g_background_colour_rgba[4];

static void _window_update() {
  GLFWmonitor* mon       = NULL;
  int refresh_rate       = GLFW_DONT_CARE;
  int width              = gfx_win_width;
  int height             = gfx_win_height;
  int count              = 0;
  GLFWmonitor** monitors = glfwGetMonitors( &count );
  if ( _g_fullscreen ) {
    int monitor_to_use = _g_monitor_to_use;
    if ( monitor_to_use >= 0 && monitor_to_use < count ) {
      mon = monitors[monitor_to_use];
      glog( "window_update - using monitor %i\n", monitor_to_use );
    }
    const GLFWvidmode* mode = glfwGetVideoMode( mon );
    width                   = mode->width;
    height                  = mode->height;
    refresh_rate            = mode->refreshRate;
  }
  // stop window jumping around by re-using same values
  int xpos = 0;
  int ypos = 0;
  glfwGetWindowPos( g_window, &xpos, &ypos );

  glfwSetWindowMonitor( g_window, mon, xpos, ypos, width, height, refresh_rate );
}

static void _glfw_error_callback( int error, const char* description ) { glog_err( "GLFW ERROR: code %i msg: %s\n", error, description ); }

static void _glfw_window_size_callback( GLFWwindow* window, int width, int height ) {
  APG_UNUSED( window );
  glog( "window dims changed to %ix%i\n", width, height );
  gfx_win_width  = width;
  gfx_win_height = height;
}

static void _glfw_fb_size_callback( GLFWwindow* window, int width, int height ) {
  APG_UNUSED( window );
  glog( "framebuffer dims changed to %ix%i\n", width, height );
  gfx_fb_width   = width;
  gfx_fb_height  = height;
  gfx_fb_resized = true; // flag to tell matrices and framebuffers to resize

  // on high-DPI detect super-high res and downscale by default for perf
  gfx_framebuffer_scale = 1.0f;
  if ( gfx_fb_height > 1440 ) {
    gfx_framebuffer_scale = 0.5f;
    glog( " >1440p high-DPI viewport detected. Downscaling framebuffers to 0.5x for performance.\n" );
  }
}

#ifdef DEBUG
static void _debug_gl_callback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
  APG_UNUSED( userParam );

  char src_str[2048], type_str[2048], sev_str[2048];

  switch ( source ) {
  case 0x8246: {
    strncpy( src_str, "API", 2048 );
  } break;
  case 0x8247: {
    strncpy( src_str, "WINDOW_SYSTEM", 2048 );
  } break;
  case 0x8248: {
    strncpy( src_str, "SHADER_COMPILER", 2048 );
  } break;
  case 0x8249: {
    strncpy( src_str, "THIRD_PARTY", 2048 );
  } break;
  case 0x824A: {
    strncpy( src_str, "APPLICATION", 2048 );
  } break;
  case 0x824B: {
    strncpy( src_str, "OTHER", 2048 );
  } break;
  default: { strncpy( src_str, "undefined", 2048 ); } break;
  }

  switch ( type ) {
  case 0x824C: {
    strncpy( type_str, "ERROR", 2048 );
  } break;
  case 0x824D: {
    strncpy( type_str, "DEPRECATED_BEHAVIOR", 2048 );
  } break;
  case 0x824E: {
    strncpy( type_str, "UNDEFINED_BEHAVIOR", 2048 );
  } break;
  case 0x824F: {
    strncpy( type_str, "PORTABILITY", 2048 );
  } break;
  case 0x8250: {
    // strncpy (type_str, "PERFORMANCE", 2048);
    return; // ignore these messages
  } break;
  case 0x8251: {
    strncpy( type_str, "OTHER", 2048 );
  } break;
  case 0x8268: {
    strncpy( type_str, "MARKER", 2048 );
  } break;
  case 0x8269: {
    strncpy( type_str, "PUSH_GROUP", 2048 );
  } break;
  case 0x826A: {
    strncpy( type_str, "POP_GROUP", 2048 );
  } break;
  default: { strncpy( type_str, "undefined", 2048 ); } break;
  }

  switch ( severity ) {
  case 0x9146: {
    strncpy( sev_str, "HIGH", 2048 );
  } break;
  case 0x9147: {
    strncpy( sev_str, "MEDIUM", 2048 );
  } break;
  case 0x9148: {
    strncpy( sev_str, "LOW", 2048 );
  } break;
  case 0x826B: {
    // strncpy (sev_str, "NOTIFICATION", 2048);
    return; // ignore these messages
  } break;
  default: { strncpy( sev_str, "undefined", 2048 ); } break;
  }

  glog_err( "src: %s type: %s id: %u severity: %s len: %i msg: %s\n", src_str, type_str, id, sev_str, length, message );
  print_trace( stderr );
}
#endif

static bool _extension_checks() {
  bool has_all_vital = true;

  // non-core but should be available on a toaster since GL 1.1
  if ( GLEW_EXT_texture_compression_s3tc ) {
    glog( "EXT_texture_compression_s3tc = yes\n" );
  } else {
    glog( "EXT_texture_compression_s3tc = no\n" );
  }

  // non-core (IP issue) but should be available on a toaster since GL 1.2
  if ( GLEW_EXT_texture_filter_anisotropic ) {
    glog( "EXT_texture_filter_anisotropic = yes\n" );
  } else {
    glog( "EXT_texture_filter_anisotropic = no\n" );
  }

  // (core since GL 3.0) 2004 works with opengl 1.5
  if ( GLEW_ARB_texture_float ) { // 94% support
    glog( "ARB_texture_float = yes\n" );
  } else {
    glog( "ARB_texture_float = no\n" );
    // has_all_vital = false; // APPLE reports it doesn't have this
  }

  // core since 3.0
  if ( GLEW_ARB_framebuffer_object ) { // glcapsviewer gives 89% support
    glog( "ARB_framebuffer_object = yes\n" );
  } else {
    glog( "ARB_framebuffer_object = no\n" );
    has_all_vital = false;
  }

  // core since 3.0
  if ( GLEW_ARB_vertex_array_object ) { // 87% support
    glog( "ARB_vertex_array_object = yes\n" );
  } else {
    glog( "ARB_vertex_array_object = no\n" );
    has_all_vital = false;
  }

  // core since 3.1
  if ( GLEW_ARB_uniform_buffer_object ) { // 80%
    glog( "ARB_uniform_buffer_object = yes\n" );
  } else {
    glog( "ARB_uniform_buffer_object = no\n" );
    has_all_vital = false;
  }

  // core since 3.3
  if ( GLEW_ARB_timer_query ) {
    glog( "ARB_timer_query = yes\n" );
  } else {
    glog( "ARB_timer_query = no\n" );
  }

  // this is core in 4.3+ but i'm using 4.1
  if ( GLEW_ARB_debug_output ) { // glcapsviewer reports 47% support
    glog( "ARB_debug_output = yes\n" );
#ifdef DEBUG
    // Enabling synchronous debug output greatly simplifies the responsibilities of
    // the application for making its callback functions thread-safe, but may
    // potentially result in drastically reduced driver performance.
    glDebugMessageCallbackARB( _debug_gl_callback, NULL );
    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB );
    glog( "~~~~DEBUG OUTPUT EXTENSION ENGAGED!~~~~\n" );
#else
    glog( "debug build not enabled\n" );
#endif
  } else {
    glog( "ARB_debug_output = no\n" );
  }
  /*
    if ( GL_NVX_gpu_memory_info ) {
      GLint dedmem = 0, totalavailmem = 0, currentavail = 0, eviction = 0, evicted = 0;
      glGetIntegerv( GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &dedmem );               // NOTE(Anton) invalid enum here on my laptop
      glGetIntegerv( GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalavailmem );  // NOTE(Anton) invalid enum here on my laptop
      glGetIntegerv( GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentavail ); // NOTE(Anton) invalid enum here on my laptop
      glGetIntegerv( GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &eviction );               // NOTE(Anton) invalid enum here on my laptop
      glGetIntegerv( GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evicted );                // NOTE(Anton) invalid enum here on my laptop
      glog(
        "NVX_gpu_memory_info = yes\n  dedicated %i kB\n  total avail %i kB\n  "
        "current avail %i kB\n  evictions %i\n  evicted %i kB\n",
        dedmem, totalavailmem, currentavail, eviction, evicted );
    } else {
      glog( "NVX_gpu_memory_info = no\n" );
    }*/

  /* Need to parse out that we do in fact an ATI first or this runs and produces
  enum errors
  if ( GL_ATI_meminfo ) {
    glog(  "GL_ATI_meminfo = yes\n");

    int param[4] = { 0 };
    glGetIntegerv( GL_VBO_FREE_MEMORY_ATI, param );
    glog( " GL_VBO_FREE_MEMORY_ATI:\n");
    glog( "  total dedicted gfx mem: %i kB\n", param[0]);
    glog( "   largest available block: %i kB\n", param[1]);
    glog( "  total free shared system mem: %i kB\n", param[2]);
    glog( "   largest available block: %i kB\n", param[3]);

    glGetIntegerv( GL_TEXTURE_FREE_MEMORY_ATI, param );
    glog( " GL_TEXTURE_FREE_MEMORY_ATI:\n");
    glog( "  total dedicted gfx mem: %i kB\n", param[0]);
    glog( "   largest available block: %i kB\n", param[1]);
    glog( "  total free shared system mem: %i kB\n", param[2]);
    glog( "   largest available block: %i kB\n", param[3]);

    glGetIntegerv( GL_RENDERBUFFER_FREE_MEMORY_ATI, param );
    glog( " GL_RENDERBUFFER_FREE_MEMORY_ATI:\n");
    glog( "  total dedicted gfx mem: %i kB\n", param[0]);
    glog( "   largest available block: %i kB\n", param[1]);
    glog( "  total free shared system mem: %i kB\n", param[2]);
    glog( "   largest available block: %i kB\n", param[3]);
  } else {
    glog(  "GL_ATI_meminfo = no\n");
  }
  */

  return has_all_vital;
}

void gfx_wireframe_mode() { glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); }

void gfx_polygon_mode() { glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); }

void gfx_blend_one_minus_src_alpha( bool enabled ) {
  if ( enabled ) {
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
  } else {
    glDisable( GL_BLEND );
  }
}

void gfx_depth_testing( bool enabled ) {
  glDepthFunc( GL_LESS );
  if ( enabled ) {
    glEnable( GL_DEPTH_TEST );
  } else {
    glDisable( GL_DEPTH_TEST );
  }
}

void gfx_depth_masking( bool enabled ) {
  if ( enabled ) {
    glDepthMask( GL_TRUE );
  } else {
    glDepthMask( GL_FALSE );
  }
}

bool gfx_start( int win_w, int win_h, bool msaa, const char* window_title, const char* icon_filename ) {
  gfx_win_width  = win_w;
  gfx_win_height = win_h;
  {
    glfwSetErrorCallback( _glfw_error_callback );
    if ( !glfwInit() ) {
      glog_err( "ERROR: could not start GLFW3\n" );
      return false;
    }
    { // OpenGL Version hint
      glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
      glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
      glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
      glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    }
    if ( msaa ) { // MSAA hint
      int max_samples = 1;
      glGetIntegerv( GL_MAX_COLOR_TEXTURE_SAMPLES, &max_samples ); // was 32 on my 1080ti
      glog( "GL_MAX_COLOR_TEXTURE_SAMPLES = %i\n", max_samples );
      int nsamples = 32 < max_samples ? 32 : max_samples;
      glfwWindowHint( GLFW_SAMPLES, nsamples );
    }
#ifdef DEBUG
    glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );
#endif

    g_window = glfwCreateWindow( gfx_win_width, gfx_win_height, window_title, NULL, NULL );
    if ( !g_window ) {
      glog_err( "ERROR: could not open window with GLFW3\n" );
      glfwTerminate();
      return false;
    }
    _window_update();
    glfwSetWindowSizeCallback( g_window, _glfw_window_size_callback );
    glfwSetFramebufferSizeCallback( g_window, _glfw_fb_size_callback );
    glfwMakeContextCurrent( g_window );

    if ( icon_filename ) { // load window icon
      int width = 0, height = 0, comps = 0, req_comps = 0;
      unsigned char* pixels = stbi_load( icon_filename, &width, &height, &comps, req_comps );
      if ( !pixels ) { glog_err( "ERROR: loading image from file `%s`\n", icon_filename ); }
      GLFWimage icon_image = { .width = width, .height = height, .pixels = pixels };
      glfwSetWindowIcon( g_window, 1, &icon_image );
      stbi_image_free( pixels );
    }

    // glfwSwapInterval( 1 ); // disable vsync. can be called any time after context is current

    glfwGetWindowSize( g_window, &gfx_win_width, &gfx_win_height );
    glog( "initial window dims %ix%i\n", gfx_win_width, gfx_win_height );
    glfwGetFramebufferSize( g_window, &gfx_fb_width, &gfx_fb_height );
    glog( "initial framebuffer dims %ix%i\n", gfx_fb_width, gfx_fb_height );

    // on high-DPI detect super-high res and downscale by default for perf
    gfx_framebuffer_scale = 1.0f;
    if ( gfx_fb_height > 1440 ) {
      gfx_framebuffer_scale = 0.5f;
      glog( " >1440p high-DPI viewport detected. Downscaling framebuffers to 0.5x for performance.\n" );
    }
  }
  glewExperimental = GL_TRUE;
  glewInit();

  const GLubyte* renderer = glGetString( GL_RENDERER );
  const GLubyte* version  = glGetString( GL_VERSION );
  glog( "Renderer: %s\n", renderer );
  glog( "OpenGL version supported %s\n", version );

  if ( !_extension_checks() ) { return false; }

  bool mag_linear    = false;
  bool min_linear    = false;
  gfx_fallback_texture = gfx_load_image_mem_to_texture( gfx_fallback_pixels, 2, 2, 4, false, mag_linear, min_linear, false, false );

  _init_fallback_shader();
  _init_ss_quad();
  _init_unit_cube();

  // set default so that at least _g_background_colour_rgba is valid if get_buffer_colour() is called before a call to buffer_colour()
  glClearColor( _g_background_colour_rgba[0], _g_background_colour_rgba[1], _g_background_colour_rgba[2], _g_background_colour_rgba[3] );

  glEnable( GL_FRAMEBUFFER_SRGB ); // enable sRGB texture writing
  // glEnable( GL_MULTISAMPLE );      // https://www.khronos.org/opengl/wiki/Multisampling

  return true;
}

void gfx_stop() {
  gfx_delete_all_managed_meshes();
  gfx_delete_mesh( &gfx_unit_cube_mesh );
  gfx_delete_mesh( &gfx_ss_quad_mesh );
  gfx_delete_all_managed_textures();
  gfx_delete_texture( &gfx_fallback_texture );
  gfx_delete_all_managed_shaders();
  gfx_delete_shader( &gfx_fallback_shader );
  glfwTerminate();
}

bool gfx_should_window_close() {
  assert( g_window );

  return glfwWindowShouldClose( g_window );
}

void gfx_close_window() { glfwSetWindowShouldClose( g_window, GLFW_TRUE ); }

void gfx_show_system_cursor() {
  assert( g_window );

  glfwSetInputMode( g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
}

void gfx_hide_system_cursor() {
  assert( g_window );

  glfwSetInputMode( g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN ); // NB _HIDDEN lets it leave the window, _DISABLED hides&grabs
}

void gfx_buffer_colour( gfx_framebuffer_meta_t* fb_meta, float r, float g, float b, float a ) {
  if ( !fb_meta ) {
    _g_background_colour_rgba[0] = r;
    _g_background_colour_rgba[1] = g;
    _g_background_colour_rgba[2] = b;
    _g_background_colour_rgba[3] = a;
  } else {
    fb_meta->background_colour_rgba[0] = r;
    fb_meta->background_colour_rgba[1] = g;
    fb_meta->background_colour_rgba[2] = b;
    fb_meta->background_colour_rgba[3] = a;
  }
  glClearColor( r, g, b, a );
}

void gfx_get_buffer_colour( const gfx_framebuffer_meta_t* fb_meta, float* r, float* g, float* b, float* a ) {
  assert( r && g && b && a );

  if ( !fb_meta ) {
    *r = _g_background_colour_rgba[0];
    *g = _g_background_colour_rgba[1];
    *b = _g_background_colour_rgba[2];
    *a = _g_background_colour_rgba[3];
  } else {
    *r = fb_meta->background_colour_rgba[0];
    *g = fb_meta->background_colour_rgba[1];
    *b = fb_meta->background_colour_rgba[2];
    *a = fb_meta->background_colour_rgba[3];
  }
}

void gfx_clear_colour_and_depth_buffers() {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  {
    glDepthFunc( GL_LESS );
    glEnable( GL_DEPTH_TEST );
    glCullFace( GL_BACK );
    glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
  }
}

void gfx_clear_depth_buffer() { glClear( GL_DEPTH_BUFFER_BIT ); }

void gfx_swap_buffer_and_wait_events() {
  assert( g_window );

  glfwWaitEvents();
  glfwSwapBuffers( g_window );
}

void gfx_swap_buffer_and_poll_events() {
  assert( g_window );

  glfwPollEvents();
  glfwSwapBuffers( g_window );
}
