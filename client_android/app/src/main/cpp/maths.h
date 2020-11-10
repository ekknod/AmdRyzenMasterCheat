#ifndef MATHS_H
#define MATHS_H


#include <inttypes.h>

#ifndef __cplusplus
typedef int bool;
#endif

typedef struct {
	float x, y;
} vec2 ;

typedef struct {
    int x, y;
} vec2i ;

typedef struct {
	float x, y, z;
} vec3 ;
typedef float matrix3x4_t[3][4];

/* translate maths to only X, Y */


void angle_vec(vec3 angles, vec3 *forward);
float vec_dot(vec3 v0, vec3 v1);
float vec_length(vec3 v);
float vec_length_sqrt(vec3 p0);
vec3 vec_delta(vec3 p0, vec3 p1);
float vec_distance(vec3 p0, vec3 p1);
void vec_clamp(vec3 *v);
void vec_normalize(vec3 *vec);
vec3 vec_transform(vec3 p0, matrix3x4_t p1);
vec3 vec_atd(vec3 vangle);
bool vec_min_max(vec3 eye, vec3 dir, vec3 min, vec3 max, float radius);
void vec_angles(vec3 forward, vec3 *angles);
float get_fov(vec3 vangle, vec3 angle);
float get_fov_distance(vec3 vangle, vec3 angle, float distance);
vec3 calc_angle(vec3 src, vec3 dst);
void CalcAngle( float *src, float *dst, float *angles );
#endif

