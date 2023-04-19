/*
OpenGL Debug Drawing Functions v0.4.1
https://github.com/capnramses/opengl_debug_draw
Anton Gerdelan <antonofnote@gmail.com>
LICENCE - See bottom of header file.
*/

#include "apg_gldb.h"
#include "apg_glcontext.h" // REPLACE THIS INCLUDE WITH YOUR PROJECT'S OPENGL HEADER
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset

#define APG_GLDB_PI 3.14159265358979323846

static uint32_t _count_lines;        // current num of debug lines in buffer
static GLuint _lines_vao;            // vao for drawing properties of lines
static GLuint _lines_vbo;            // handle to GPU buffer
static GLuint _lines_shader_program; // shader prog for lines
static GLint _PV_loc = -1;           // camera matrix uniform id

static const char* gl_db_lines_vs_str =
  "#version 120\n"
  "attribute vec3 vp;\n"
  "attribute vec4 vc;\n"
  "uniform mat4 PV;\n"
  "varying vec4 fc;\n"
  "void main () {\n"
  "  gl_Position = PV * vec4 (vp, 1.0);\n"
  "  fc = vc;\n"
  "}";

static const char* gl_db_lines_fs_str =
  "#version 120\n"
  "varying vec4 fc;\n"
  "void main () {\n"
  "  gl_FragColor = fc;\n"
  "}";

void apg_gldb_reset_lines( void ) { _count_lines = 0; }

bool apg_gldb_init( void ) {
  // vao for drawing properties of lines
  glGenVertexArrays( 1, &_lines_vao );
  glBindVertexArray( _lines_vao );

  // create GPU-side buffer
  // size is 32-bits for GLfloat * num lines * 7 comps per vert * 2 per lines
  glGenBuffers( 1, &_lines_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, _lines_vbo );
  glBufferData( GL_ARRAY_BUFFER, 4 * APG_GLDB_MAX_LINES * 14, NULL, GL_DYNAMIC_DRAW );

  GLsizei stride = 4 * 7;
  GLintptr offs  = 4 * 3;
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, NULL );          // point
  glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offs ); // colour
  glEnableVertexAttribArray( 0 );                                           // point
  glEnableVertexAttribArray( 1 );                                           // colour

  GLuint vs = glCreateShader( GL_VERTEX_SHADER );
  GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( vs, 1, (const char**)&gl_db_lines_vs_str, NULL );
  glCompileShader( vs );
  glShaderSource( fs, 1, (const char**)&gl_db_lines_fs_str, NULL );
  glCompileShader( fs );
  _lines_shader_program = glCreateProgram();
  glAttachShader( _lines_shader_program, fs );
  glAttachShader( _lines_shader_program, vs );
  glBindAttribLocation( _lines_shader_program, 0, "vp" );
  glBindAttribLocation( _lines_shader_program, 1, "vc" );
  glLinkProgram( _lines_shader_program );
  _PV_loc = glGetUniformLocation( _lines_shader_program, "PV" );
  assert( _PV_loc >= -1 );
  // flag that shaders can be deleted whenever the program is deleted
  glDeleteShader( fs );
  glDeleteShader( vs );
  float PV[16];
  memset( PV, 0, 16 * sizeof( float ) );
  PV[0] = PV[5] = PV[10] = PV[15] = 1.0f;
  glUseProgram( _lines_shader_program );
  glUniformMatrix4fv( _PV_loc, 1, GL_FALSE, PV );

  return true;
}

void apg_gldb_free( void ) {
  glDeleteBuffers( 1, &_lines_vbo );
  glDeleteVertexArrays( 1, &_lines_vao );
  // attached shaders have prev been flagged to delete so will also be deleted
  glDeleteProgram( _lines_shader_program );
}

int apg_gldb_add_line( const float* start_xyz, const float* end_xyz, const float* colour_rgba ) {
  if ( _count_lines >= APG_GLDB_MAX_LINES ) {
    fprintf( stderr, "ERROR: too many apg_gldb lines\n" );
    return -1;
  }

  float sd[14];
  sd[0]  = start_xyz[0];
  sd[1]  = start_xyz[1];
  sd[2]  = start_xyz[2];
  sd[3]  = colour_rgba[0];
  sd[4]  = colour_rgba[1];
  sd[5]  = colour_rgba[2];
  sd[6]  = colour_rgba[3];
  sd[7]  = end_xyz[0];
  sd[8]  = end_xyz[1];
  sd[9]  = end_xyz[2];
  sd[10] = colour_rgba[0];
  sd[11] = colour_rgba[1];
  sd[12] = colour_rgba[2];
  sd[13] = colour_rgba[3];

  glBindBuffer( GL_ARRAY_BUFFER, _lines_vbo );
  GLintptr os = sizeof( sd ) * _count_lines;
  GLsizei sz  = sizeof( sd );
  glBufferSubData( GL_ARRAY_BUFFER, os, sz, sd );

  return _count_lines++;
}

int apg_gldb_add_normal( const float* n_xyz, const float* pos_xyz, float scale, const float* colour_rgba ) {
  if ( _count_lines >= APG_GLDB_MAX_LINES ) {
    fprintf( stderr, "ERROR: too many apg_gldb lines\n" );
    return -1;
  }
  float end[3];
  end[0] = pos_xyz[0] + n_xyz[0] * scale;
  end[1] = pos_xyz[1] + n_xyz[1] * scale;
  end[2] = pos_xyz[2] + n_xyz[2] * scale;

  float sd[14];
  sd[0]  = pos_xyz[0];
  sd[1]  = pos_xyz[1];
  sd[2]  = pos_xyz[2];
  sd[3]  = 0.0f;
  sd[4]  = 0.0f;
  sd[5]  = 0.0f;
  sd[6]  = 1.0f;
  sd[7]  = end[0];
  sd[8]  = end[1];
  sd[9]  = end[2];
  sd[10] = colour_rgba[0];
  sd[11] = colour_rgba[1];
  sd[12] = colour_rgba[2];
  sd[13] = colour_rgba[3];

  glBindBuffer( GL_ARRAY_BUFFER, _lines_vbo );
  GLintptr os = sizeof( sd ) * _count_lines;
  GLsizei sz  = sizeof( sd );
  glBufferSubData( GL_ARRAY_BUFFER, os, sz, sd );

  return _count_lines++;
}

int apg_gldb_add_pos( const float* pos_xyz, float scale, const float* colour_rgba ) {
  int rid = -1;
  float start[3], end[3];

  start[0] = pos_xyz[0] - scale;
  start[1] = pos_xyz[1];
  start[2] = pos_xyz[2];
  end[0]   = pos_xyz[0] + scale;
  end[1]   = pos_xyz[1];
  end[2]   = pos_xyz[2];
  rid      = apg_gldb_add_line( start, end, colour_rgba );
  start[0] = pos_xyz[0];
  start[1] = pos_xyz[1] - scale;
  start[2] = pos_xyz[2];
  end[0]   = pos_xyz[0];
  end[1]   = pos_xyz[1] + scale;
  end[2]   = pos_xyz[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = pos_xyz[0];
  start[1] = pos_xyz[1];
  start[2] = pos_xyz[2] - scale;
  end[0]   = pos_xyz[0];
  end[1]   = pos_xyz[1];
  end[2]   = pos_xyz[2] + scale;
  apg_gldb_add_line( start, end, colour_rgba );

  return rid;
}

int apg_gldb_add_aabb( const float* min_xyz, const float* max_xyz, const float* colour_rgba ) {
  int rid = -1;
  float start[3], end[3];

  // bottom ring
  // rear
  start[0] = min_xyz[0];          // L
  end[0]   = max_xyz[0];          // R
  start[1] = end[1] = min_xyz[1]; // B
  start[2] = end[2] = min_xyz[2]; // P
  rid               = apg_gldb_add_line( start, end, colour_rgba );
  // right
  start[0] = max_xyz[0]; // R
  end[2]   = max_xyz[2]; // A
  apg_gldb_add_line( start, end, colour_rgba );
  // front
  end[0]   = min_xyz[0]; // L
  start[2] = max_xyz[2]; // A
  apg_gldb_add_line( start, end, colour_rgba );
  // left
  start[0] = min_xyz[0]; // L
  end[2]   = min_xyz[2]; // P
  apg_gldb_add_line( start, end, colour_rgba );

  // top ring
  start[0] = min_xyz[0];          // L
  end[0]   = max_xyz[0];          // R
  start[1] = end[1] = max_xyz[1]; // T
  start[2] = end[2] = min_xyz[2]; // P
  apg_gldb_add_line( start, end, colour_rgba );
  // right
  start[0] = max_xyz[0]; // R
  end[2]   = max_xyz[2]; // A
  apg_gldb_add_line( start, end, colour_rgba );
  // front
  end[0]   = min_xyz[0]; // L
  start[2] = max_xyz[2]; // A
  apg_gldb_add_line( start, end, colour_rgba );
  // left
  start[0] = min_xyz[0]; // L
  end[2]   = min_xyz[2]; // P
  apg_gldb_add_line( start, end, colour_rgba );

  // 4 side edges
  start[0] = end[0] = min_xyz[0]; // L
  start[1]          = min_xyz[1];
  end[1]            = max_xyz[1];
  start[2] = end[2] = min_xyz[2]; // P
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = end[0] = max_xyz[0]; // R
  apg_gldb_add_line( start, end, colour_rgba );
  start[2] = end[2] = max_xyz[2]; // A
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = end[0] = min_xyz[0]; // L
  apg_gldb_add_line( start, end, colour_rgba );

  return rid;
}

int apg_gldb_add_rad_circle( const float* centre_xyz, float radius, const float* colour_rgba ) {
  int rid = -1;
  float start[3], end[3];
  // 3 radius lines in a cross first
  start[0] = centre_xyz[0];
  start[1] = centre_xyz[1] - radius;
  start[2] = centre_xyz[2];
  end[0]   = centre_xyz[0];
  end[1]   = centre_xyz[1] + radius;
  end[2]   = centre_xyz[2];
  rid      = apg_gldb_add_line( start, end, colour_rgba );
  start[0] = centre_xyz[0] - radius;
  start[1] = centre_xyz[1];
  start[2] = centre_xyz[2];
  end[0]   = centre_xyz[0] + radius;
  end[1]   = centre_xyz[1];
  end[2]   = centre_xyz[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = centre_xyz[0];
  start[1] = centre_xyz[1];
  start[2] = centre_xyz[2] - radius;
  end[0]   = centre_xyz[0];
  end[1]   = centre_xyz[1];
  end[2]   = centre_xyz[2] + radius;
  apg_gldb_add_line( start, end, colour_rgba );
  // circles of 12 segments
  int segs = 12;
  // x,y around z loop
  for ( int i = 0; i < segs; i++ ) {
    start[0] = centre_xyz[0] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    start[1] = centre_xyz[1] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    start[2] = centre_xyz[2];
    end[0]   = centre_xyz[0] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    end[1]   = centre_xyz[1] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    end[2]   = centre_xyz[2];
    apg_gldb_add_line( start, end, colour_rgba );
  }
  // x,z around y loop
  for ( int i = 0; i < segs; i++ ) {
    start[0] = centre_xyz[0] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    start[1] = centre_xyz[1];
    start[2] = centre_xyz[2] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    end[0]   = centre_xyz[0] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    end[1]   = centre_xyz[1];
    end[2]   = centre_xyz[2] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    apg_gldb_add_line( start, end, colour_rgba );
  }
  // y,z around xloop
  for ( int i = 0; i < segs; i++ ) {
    start[0] = centre_xyz[0];
    start[1] = centre_xyz[1] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    start[2] = centre_xyz[2] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)i / (float)segs );
    end[0]   = centre_xyz[0];
    end[1]   = centre_xyz[1] + radius * cosf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    end[2]   = centre_xyz[2] + radius * sinf( 2.0f * (float)APG_GLDB_PI * (float)( i + 1 ) / (float)segs );
    apg_gldb_add_line( start, end, colour_rgba );
  }

  return rid;
}

int apg_gldb_add_frustum( const float* ftl, const float* ftr, const float* fbl, const float* fbr, const float* ntl, const float* ntr, const float* nbl,
  const float* nbr, const float* colour_rgba ) {
  int rid = -1;
  float start[3], end[3];
  start[0] = ftl[0];
  start[1] = ftl[1];
  start[2] = ftl[2];
  end[0]   = ftr[0];
  end[1]   = ftr[1];
  end[2]   = ftr[2];
  rid      = apg_gldb_add_line( start, end, colour_rgba );
  start[0] = ntl[0];
  start[1] = ntl[1];
  start[2] = ntl[2];
  end[0]   = ntr[0];
  end[1]   = ntr[1];
  end[2]   = ntr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = fbl[0];
  start[1] = fbl[1];
  start[2] = fbl[2];
  end[0]   = fbr[0];
  end[1]   = fbr[1];
  end[2]   = fbr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = nbl[0];
  start[1] = nbl[1];
  start[2] = nbl[2];
  end[0]   = nbr[0];
  end[1]   = nbr[1];
  end[2]   = nbr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  // sides of top/bottom panels
  start[0] = ftl[0];
  start[1] = ftl[1];
  start[2] = ftl[2];
  end[0]   = ntl[0];
  end[1]   = ntl[1];
  end[2]   = ntl[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = ftr[0];
  start[1] = ftr[1];
  start[2] = ftr[2];
  end[0]   = ntr[0];
  end[1]   = ntr[1];
  end[2]   = ntr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = fbl[0];
  start[1] = fbl[1];
  start[2] = fbl[2];
  end[0]   = nbl[0];
  end[1]   = nbl[1];
  end[2]   = nbl[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = fbr[0];
  start[1] = fbr[1];
  start[2] = fbr[2];
  end[0]   = nbr[0];
  end[1]   = nbr[1];
  end[2]   = nbr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  // edges for left/right panels
  start[0] = ftl[0];
  start[1] = ftl[1];
  start[2] = ftl[2];
  end[0]   = fbl[0];
  end[1]   = fbl[1];
  end[2]   = fbl[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = ntl[0];
  start[1] = ntl[1];
  start[2] = ntl[2];
  end[0]   = nbl[0];
  end[1]   = nbl[1];
  end[2]   = nbl[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = ftr[0];
  start[1] = ftr[1];
  start[2] = ftr[2];
  end[0]   = fbr[0];
  end[1]   = fbr[1];
  end[2]   = fbr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  start[0] = ntr[0];
  start[1] = ntr[1];
  start[2] = ntr[2];
  end[0]   = nbr[0];
  end[1]   = nbr[1];
  end[2]   = nbr[2];
  apg_gldb_add_line( start, end, colour_rgba );
  return rid;
}

bool apg_gldb_mod_line( uint32_t line_id, const float* start_xyz, const float* end_xyz, const float* colour_rgba ) {
  if ( line_id >= _count_lines ) {
    fprintf( stderr, "ERROR: modifying apg_gldb line - bad ID\n" );
    return false;
  }

  float sd[14];
  sd[0]  = start_xyz[0];
  sd[1]  = start_xyz[1];
  sd[2]  = start_xyz[2];
  sd[3]  = colour_rgba[0];
  sd[4]  = colour_rgba[1];
  sd[5]  = colour_rgba[2];
  sd[6]  = colour_rgba[3];
  sd[7]  = end_xyz[0];
  sd[8]  = end_xyz[1];
  sd[9]  = end_xyz[2];
  sd[10] = colour_rgba[0];
  sd[11] = colour_rgba[1];
  sd[12] = colour_rgba[2];
  sd[13] = colour_rgba[3];

  glBindBuffer( GL_ARRAY_BUFFER, _lines_vbo );
  GLintptr os = sizeof( sd ) * line_id;
  GLsizei sz  = sizeof( sd );
  glBufferSubData( GL_ARRAY_BUFFER, os, sz, sd );

  return true;
}

bool apg_gldb_mod_aabb( uint32_t line_id, const float* min_xyz, const float* max_xyz, const float* colour_rgba ) {
  if ( line_id + 12 > _count_lines ) {
    fprintf( stderr, "ERROR: modifying apg_gldb AABB - bad line ID\n" );
    return false;
  }
  {
    float start[3], end[3];

    // bottom ring
    // rear
    start[0] = min_xyz[0];          // L
    end[0]   = max_xyz[0];          // R
    start[1] = end[1] = min_xyz[1]; // B
    start[2] = end[2] = min_xyz[2]; // P
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // right
    start[0] = max_xyz[0]; // R
    end[2]   = max_xyz[2]; // A
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // front
    end[0]   = min_xyz[0]; // L
    start[2] = max_xyz[2]; // A
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // left
    start[0] = min_xyz[0]; // L
    end[2]   = min_xyz[2]; // P
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }

    // top ring
    start[0] = min_xyz[0];          // L
    end[0]   = max_xyz[0];          // R
    start[1] = end[1] = max_xyz[1]; // T
    start[2] = end[2] = min_xyz[2]; // P
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // right
    start[0] = max_xyz[0]; // R
    end[2]   = max_xyz[2]; // A
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // front
    end[0]   = min_xyz[0]; // L
    start[2] = max_xyz[2]; // A
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    // left
    start[0] = min_xyz[0]; // L
    end[2]   = min_xyz[2]; // P
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }

    // 4 side edges
    start[0] = end[0] = min_xyz[0]; // L
    start[1]          = min_xyz[1];
    end[1]            = max_xyz[1];
    start[2] = end[2] = min_xyz[2]; // P
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    start[0] = end[0] = max_xyz[0]; // R
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    start[2] = end[2] = max_xyz[2]; // A
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
    start[0] = end[0] = min_xyz[0]; // L
    if ( !apg_gldb_mod_line( line_id++, start, end, colour_rgba ) ) { return false; }
  }

  return true;
}

bool apg_gldb_mod_frustum( uint32_t line_id, const float* ftl, const float* ftr, const float* fbl, const float* fbr, const float* ntl, const float* ntr,
  const float* nbl, const float* nbr, const float* colour_rgba ) {
  if ( line_id + 12 > _count_lines ) {
    fprintf( stderr, "ERROR: modifying apg_gldb frustum - bad line ID\n" );
    return false;
  }
  {
    float start[3], end[3];
    start[0] = ftl[0];
    start[1] = ftl[1];
    start[2] = ftl[2];
    end[0]   = ftr[0];
    end[1]   = ftr[1];
    end[2]   = ftr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = ntl[0];
    start[1] = ntl[1];
    start[2] = ntl[2];
    end[0]   = ntr[0];
    end[1]   = ntr[1];
    end[2]   = ntr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = fbl[0];
    start[1] = fbl[1];
    start[2] = fbl[2];
    end[0]   = fbr[0];
    end[1]   = fbr[1];
    end[2]   = fbr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = nbl[0];
    start[1] = nbl[1];
    start[2] = nbl[2];
    end[0]   = nbr[0];
    end[1]   = nbr[1];
    end[2]   = nbr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    // sides of top/bottom panels
    start[0] = ftl[0];
    start[1] = ftl[1];
    start[2] = ftl[2];
    end[0]   = ntl[0];
    end[1]   = ntl[1];
    end[2]   = ntl[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = ftr[0];
    start[1] = ftr[1];
    start[2] = ftr[2];
    end[0]   = ntr[0];
    end[1]   = ntr[1];
    end[2]   = ntr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = fbl[0];
    start[1] = fbl[1];
    start[2] = fbl[2];
    end[0]   = nbl[0];
    end[1]   = nbl[1];
    end[2]   = nbl[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = fbr[0];
    start[1] = fbr[1];
    start[2] = fbr[2];
    end[0]   = nbr[0];
    end[1]   = nbr[1];
    end[2]   = nbr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    // edges for left/right panels
    start[0] = ftl[0];
    start[1] = ftl[1];
    start[2] = ftl[2];
    end[0]   = fbl[0];
    end[1]   = fbl[1];
    end[2]   = fbl[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = ntl[0];
    start[1] = ntl[1];
    start[2] = ntl[2];
    end[0]   = nbl[0];
    end[1]   = nbl[1];
    end[2]   = nbl[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = ftr[0];
    start[1] = ftr[1];
    start[2] = ftr[2];
    end[0]   = fbr[0];
    end[1]   = fbr[1];
    end[2]   = fbr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
    start[0] = ntr[0];
    start[1] = ntr[1];
    start[2] = ntr[2];
    end[0]   = nbr[0];
    end[1]   = nbr[1];
    end[2]   = nbr[2];
    apg_gldb_mod_line( line_id++, start, end, colour_rgba );
  }
  return true;
}

void apg_gldb_update_cam( const float* PV_mat4 ) {
  glUseProgram( _lines_shader_program );
  glUniformMatrix4fv( _PV_loc, 1, GL_FALSE, PV_mat4 );
}

void apg_gldb_draw( bool x_ray ) {
  GLboolean dwe = false;
  glGetBooleanv( GL_DEPTH_TEST, &dwe );
  if ( dwe && x_ray ) {
    glDisable( GL_DEPTH_TEST );
  } else if ( !dwe && !x_ray ) {
    glEnable( GL_DEPTH_TEST );
  }

  glUseProgram( _lines_shader_program );
  glBindVertexArray( _lines_vao );
  glDrawArrays( GL_LINES, 0, 2 * _count_lines );

  if ( dwe && x_ray ) {
    glEnable( GL_DEPTH_TEST );
  } else if ( !dwe && !x_ray ) {
    glDisable( GL_DEPTH_TEST );
  }
}
