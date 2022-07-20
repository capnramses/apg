#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h> // `bool` in C99
#include <stdint.h>

typedef struct apg_line_chart_params_t {
  int w, h;                         // pixels dims
  float min_x, max_x, min_y, max_y; // chart bounds on each axis
} apg_line_chart_params_t;

typedef struct apg_line_chart_t {
  uint8_t* rgb_ptr; // pixels
  apg_line_chart_params_t params;
} apg_line_chart_t;

apg_line_chart_t apg_line_chart_init( apg_line_chart_params_t chart_params );

bool apg_line_chart_free( apg_line_chart_t* chart_ptr );

bool apg_line_chart_clear( apg_line_chart_t* chart_ptr );

bool apg_line_chart_plot_lines( apg_line_chart_t* chart_ptr, float* xy_ptr, int n );

bool apg_line_chart_plot_points( apg_line_chart_t* chart_ptr, float* xy_ptr, int n );

bool apg_line_chart_x_axis_draw( apg_line_chart_t* chart_ptr, float y_value );

bool apg_line_chart_y_axis_draw( apg_line_chart_t* chart_ptr, float x_value );

#ifdef __cplusplus
}
#endif
