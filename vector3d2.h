#ifndef vec3_h
#define vec3_h

#ifdef _OPENACC
#include <accelmath.h>
#else
#include <math.h>
#endif

typedef struct {
	double x, y, z;
} vec3;

#define SET_POINT(p,v) { p.x=v[0]; p.y=v[1]; p.z=v[2]; }

#define SUBTRACT_POINT(p,v,u) {		\
	p.x=(v[0])-(u[0]);				\
	p.y=(v[1])-(u[1]);				\
	p.z=(v[2])-(u[2]);				\
}

#define NORMALIZE(p) {								\
	double fMag = ( p.x*p.x + p.y*p.y + p.z*p.z );	\
	if (fMag != 0) {								\
		double fMult = 1.0/sqrt(fMag);				\
		p.x *= fMult;								\
		p.y *= fMult;								\
		p.z *= fMult;								\
	}												\
}

#define MAGNITUDE(m,p) 	({ m=sqrt( p.x*p.x + p.y*p.y + p.z*p.z ); })

#define DOT(d,p) {  d=( p.x*p.x + p.y*p.y + p.z*p.z ); }

#define MAX(a,b) ( ((a)>(b))? (a):(b))

#define VEC(v,a,b,c) { v.x = a; v.y = b; v.z = c; }

#define VEC_MINUS_DOUBLE(p,v,u) {		\
	p.x = v.x - u;						\
	p.y = v.y - u;						\
	p.z = v.z - u;						\
}

#define VEC_MINUS_VEC(r,p,q) {			\
	r.x = p.x - q.x;					\
	r.y = p.y - q.y;					\
	r.z = p.z - q.z;					\
}

#define VEC_DOT_VEC(d,p,q) {  d=( p.x*q.x + p.y*q.y + p.z*q.z ); }

#define VEC_MUL_DOUBLE(r,p,d) {			\
	r.x = p.x * d;						\
	r.y = p.y * d;						\
	r.z = p.z * d;						\
}

#define VEC_MUL_VEC(r,p,q) {			\
	r.x = p.x * q.x;					\
	r.y = p.y * q.y;					\
	r.z = p.z * q.z;					\
}

#define VEC_ADD_DOUBLE(r,p,d) {			\
	r.x = p.x + d;						\
	r.y = p.y + d;						\
	r.z = p.z + d;						\
}

#define VEC_ADD_VEC(r,p,q) {			\
	r.x = p.x + q.x;					\
	r.y = p.y + q.y;					\
	r.z = p.z + q.z;					\
}

#define VEC_DIV_DOUBLE(r,p,d) {			\
	double fInv = 1.0/d;				\
	r.x = p.x * fInv;					\
	r.y = p.y * fInv;					\
	r.z = p.z * fInv;					\
}

#define INTERNAL_CLAMP(d,min,max) {		\
	double f = (d < min ? min : d);		\
	d = (f > max ? max : f);			\
}

#define CLAMP(v,min,max) {				\
	INTERNAL_CLAMP(v.x, min, max);		\
	INTERNAL_CLAMP(v.y, min, max);		\
	INTERNAL_CLAMP(v.z, min, max);		\
}

#endif
