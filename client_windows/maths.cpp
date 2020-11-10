#include "maths.h"
#include <math.h>


#define RAD2DEG(x) ((float)(x) * (float)(180.f / 3.14159265358979323846f))
#define DEG2RAD(x) ((float)(x) * (float)(3.14159265358979323846f / 180.f))


void sin_cos(float radians, float *sine, float *cosine)
{
    *sine = (float)sin(radians);
    *cosine = (float)cos(radians);
}

void angle_vec(vec3 angles, vec3 *forward)
{
    float sp, sy, cp, cy;
    sin_cos(DEG2RAD(angles.x), &sp, &cp);
    sin_cos(DEG2RAD(angles.y), &sy, &cy);
    forward->x = cp * cy;
    forward->y = cp * sy;
    forward->z = -sp;
}


float vec_dot(vec3 v0, vec3 v1)
{
    return (v0.x * v1.x + v0.y * v1.y + v0.z * v1.z);
}


float vec_length(vec3 v)
{
    return (v.x * v.x + v.y * v.y + v.z * v.z);
}


static vec3 vec_sub(vec3 p0, vec3 p1)
{
    vec3 r;

    r.x = p0.x - p1.x;
    r.y = p0.y - p1.y;
    r.z = p0.z - p1.z;
    return r;
}


float vec_length_sqrt(vec3 p0)
{
    return ( float )sqrt(p0.x * p0.x + p0.y * p0.y + p0.z * p0.z);
}


vec3 vec_delta(vec3 p0, vec3 p1)
{
    vec3  d;
    float l;

    d   = vec_sub(p0, p1);
    l   = (float)vec_length_sqrt(d);
    d.x /= l; d.y /= l; d.z /= l;
    return d;
}


float vec_distance(vec3 p0, vec3 p1)
{
    return vec_length_sqrt( vec_sub(p0, p1) ) ;
}


void vec_clamp(vec3 *v)
{
    if ( v->x > 89.0f && v->x <= 180.0f ) {
        v->x = 89.0f;
    }
    if ( v->x > 180.0f ) {
        v->x = v->x - 360.0f;
    }
    if( v->x < -89.0f ) {
        v->x = -89.0f;
    }
    v->y = fmodf(v->y + 180, 360) - 180;
    v->z = 0;
}



void vec_normalize(vec3 *vec)
{
    float radius;

    radius = 1.f / (float)(sqrt(vec->x*vec->x + vec->y*vec->y + vec->z*vec->z) + 1.192092896e-07f);
    vec->x *= radius, vec->y *= radius, vec->z *= radius;
}


vec3 vec_transform(vec3 p0, matrix3x4_t p1)
{
    vec3 v;

    v.x = (p0.x * p1[0][0] + p0.y * p1[0][1] + p0.z * p1[0][2]) + p1[0][3];
    v.y = (p0.x * p1[1][0] + p0.y * p1[1][1] + p0.z * p1[1][2]) + p1[1][3];
    v.z = (p0.x * p1[2][0] + p0.y * p1[2][1] + p0.z * p1[2][2]) + p1[2][3];
    return v;
}


vec3 vec_atd(vec3 vangle)
{
    double y[2], p[2];

    vangle.x *= (3.14159265358979323846f / 180.f);
    vangle.y *= (3.14159265358979323846f / 180.f);
    y[0]     = sin(vangle.y), y[1] = cos(vangle.y);
    p[0]     = sin(vangle.x), p[1] = cos(vangle.x);
    vangle.x = (float)(p[1] * y[1]);
    vangle.y = (float)(p[1] * y[0]);
    vangle.z = (float)-p[0];
    return vangle;
}


bool vec_min_max(vec3 eye, vec3 dir, vec3 min, vec3 max, float radius)
{
    vec3     delta;
    uint32_t i;
    vec3     q;
    float    v;


    //
    // original maths by superdoc1234
    //
    delta = vec_delta(max, min);
    for ( i = 0; i < vec_distance(min, max); i++ ) {
        q.x = min.x + delta.x * (float)i - eye.x;
        q.y = min.y + delta.y * (float)i - eye.y;
        q.z = min.z + delta.z * (float)i - eye.z;
        if ((v = vec_dot(q, dir)) < 1.0f) {
            return 0;
        }
        v = radius * radius - (vec_length(q) - v * v);

        if ( v <= -100.f ) {
            return 0;
        }
        if (v >= 1.19209290E-07F) {
            return 1;
        }
    }
    return 0;
}


void vec_angles(vec3 forward, vec3 *angles)
{
    float tmp, yaw, pitch;

    if (forward.y == 0.f && forward.x == 0.f) {
        yaw = 0;
        if (forward.z > 0) {
            pitch = 270;
        } else {
            pitch = 90.f;
        }
    } else {
        yaw = (float)(atan2(forward.y, forward.x) * 180.f / 3.14159265358979323846f);
        if (yaw < 0) {
            yaw += 360.f;
        }
        tmp = (float)sqrt(forward.x * forward.x + forward.y * forward.y);
        pitch = (float)(atan2(-forward.z, tmp) * 180.f / 3.14159265358979323846f);
        if (pitch < 0) {
            pitch += 360.f;
        }
    }
    angles->x = pitch;
    angles->y = yaw;
    angles->z = 0.f;
}

float get_fov(vec3 vangle, vec3 angle)
{
    vec3 a0, a1;

    angle_vec(vangle, &a0);
    angle_vec(angle, &a1);
    return RAD2DEG(acos(vec_dot(a0, a1) / vec_length(a0)));
}

float get_fov_distance(vec3 vangle, vec3 angle, float distance)
{
	vec3 a0, a1;

	angle_vec(vangle, &a0);
	angle_vec(angle, &a1);
	return RAD2DEG((acos(vec_dot(a0, a1) / vec_length(a0))) * distance);
}

vec3 calc_angle(vec3 src, vec3 dst)
{
	vec3 m = dst;
	vec3 c = src;
	c.x = m.x - c.x;
	c.y = m.y - c.y;
	c.z = m.z - c.z;
	vec_normalize(&c);
	vec_angles(c, &c);
	vec_clamp(&c);
	return c;
}

void CalcAngle( float *src, float *dst, float *angles )
{
	double delta[3] = { (src[0]-dst[0]), (src[1]-dst[1]), (src[2] - dst[2]) };
	double hyp = sqrt(delta[0]*delta[0] + delta[1]*delta[1]);
	angles[0] = (float) (asinf( (float)(delta[2]/hyp) ) * 57.295779513082f);
	angles[1] = (float) (atanf( (float)(delta[1]/delta[0]) ) * 57.295779513082f);

	if(delta[0] >= 0.0)
		angles[1] += 180.0f;
}

