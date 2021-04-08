#ifndef MAZE_H
#define MAZE_H

contour_t * djikstra_solve_maze( mono_buffer_t * img, point_t start, point_t end, mono_buffer_t * cost_map_visual );
#endif
