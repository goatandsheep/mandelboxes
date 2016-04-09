#ifndef vec3_h
#define vec3_h

#ifdef _OPENACC
#include <accelmath.h>
#else
#include <cmath>
#endif

typedef struct vec3
{
    // Data
    double x, y, z;

    // Ctors
    vec3( double InX, double InY, double InZ ) : x( InX ), y( InY ), z( InZ )
    {
    }
    void SetDoublePoint( const double *v )
    {
        x=v[0];
        y=v[1];
        z=v[2];
    }

    vec3( ) : x(0), y(0), z(0)
    {
    }

    // Operator Overloads
    inline bool operator== (const vec3& V2) const
    {
        return (x == V2.x && y == V2.y && z == V2.z);
    }

    inline vec3 operator+ (const vec3& V2) const
    {
        return vec3( x + V2.x,  y + V2.y,  z + V2.z);
    }
    inline vec3 operator- (const vec3& V2) const
    {
        return vec3( x - V2.x,  y - V2.y,  z - V2.z);
    }
    inline vec3 SubP(const double *v) const
    {
          return vec3( x - v[0],  y - v[1],  z - v[2]);
    }

    inline vec3 operator- ( ) const
    {
        return vec3(-x, -y, -z);
    }

    inline vec3 operator/ (double S ) const
    {
        double fInv = 1.0 / S;
        return vec3 (x * fInv , y * fInv, z * fInv);
    }
    inline vec3 operator/ (const vec3& V2) const
    {
        return vec3 (x / V2.x,  y / V2.y,  z / V2.z);
    }
    inline vec3 operator* (const vec3& V2) const
    {
        return vec3 (x * V2.x,  y * V2.y,  z * V2.z);
    }
    inline vec3 operator* (double S) const
    {
        return vec3 (x * S,  y * S,  z * S);
    }
    inline vec3 operator+ (double S) const
    {
        return vec3 (x + S,  y + S,  z + S);
    }
    inline vec3 operator- (double S) const
    {
        return vec3 (x - S,  y - S,  z - S);
    }

    inline void operator+= ( const vec3& V2 )
    {
        x += V2.x;
        y += V2.y;
        z += V2.z;
    }
    inline void operator-= ( const vec3& V2 )
    {
        x -= V2.x;
        y -= V2.y;
        z -= V2.z;
    }

    inline double operator[] ( int i )
    {
        if ( i == 0 ) return x;
        else if ( i == 1 ) return y;
        else return z;
    }

    // Functions
    inline double Dot( const vec3 &V1 ) const
    {
        return V1.x*x + V1.y*y + V1.z*z;
    }

    // These require math.h for the sqrt function
    double Magnitude( ) const
    {
        return sqrt( x*x + y*y + z*z );
    }

    inline void Normalize()
    {
        double fMag = ( x*x + y*y + z*z );
        if (fMag == 0)
        {
            return;
        }

        double fMult = 1.0/sqrt(fMag);
        x *= fMult;
        y *= fMult;
        z *= fMult;
        return;
    }
};

inline vec3 SubtractDoubleDouble(const double *d1, const double *d2)
{
  return vec3(d1[0]-d2[0], d1[1]-d2[1], d1[2]-d2[2]);
}

#define INTERNAL_CLAMP(d,min,max) {     \
    double f = (d < min ? min : d);     \
    d = (f > max ? max : f);            \
}

#define CLAMP(v,min,max) {              \
    INTERNAL_CLAMP(v.x, min, max);      \
    INTERNAL_CLAMP(v.y, min, max);      \
    INTERNAL_CLAMP(v.z, min, max);      \
}

/*
inline double clamp(double d, double min, double max)
{
  if (d < min)
    return min;
  if (d > max)
    return max;
  return d;
}

inline void clamp(vec3 &v, double min, double max)
{
  v.x = clamp(v.x,min,max);
  v.y = clamp(v.y,min,max);
  v.z = clamp(v.z,min,max);
}*/

#define SET_POINT(p,v) {p.x=v[0]; p.y=v[1]; p.z=v[2]; }

#define SUBTRACT_POINT(p,v,u) {     \
    p->x=(v[0])-(u[0]);             \
    p->y=(v[1])-(u[1]);             \
    p->z=(v[2])-(u[2]);             \
}

#define NORMALIZE(p) {                              \
    double fMag = ( p->x*p->x + p->y*p->y + p->z*p->z );    \
    if (fMag != 0) {                                \
        double fMult = 1.0/sqrt(fMag);              \
        p->x *= fMult;                              \
        p->y *= fMult;                              \
        p->z *= fMult;                              \
    }                                               \
}

#define MAGNITUDE(m,p)  ({ m=sqrt( p.x*p.x + p.y*p.y + p.z*p.z ); })

#define DOT(d,p) {  d=( p.x*p.x + p.y*p.y + p.z*p.z ); }

#define MAX(a,b) ( ((a)>(b))? (a):(b))

#define VEC(v,a,b,c) { v.x = a; v.y = b; v.z = c; }

#define VEC_MINUS_DOUBLE(p,v,u) {       \
    p.x = v.x - u;                      \
    p.y = v.y - u;                      \
    p.z = v.z - u;                      \
}

#define VEC_MINUS_VEC(r,p,q) {          \
    r.x = p.x - q.x;                    \
    r.y = p.y - q.y;                    \
    r.z = p.z - q.z;                    \
}

#define VEC_DOT_VEC(d,p,q) {  d=( p.x*q.x + p.y*q.y + p.z*q.z ); }

#define VEC_MUL_DOUBLE(r,p,d) {         \
    r.x = p.x * d;                      \
    r.y = p.y * d;                      \
    r.z = p.z * d;                      \
}

#define VEC_MUL_VEC(r,p,q) {            \
    r.x = p.x * q.x;                    \
    r.y = p.y * q.y;                    \
    r.z = p.z * q.z;                    \
}

#define VEC_ADD_DOUBLE(r,p,d) {         \
    r.x = p.x + d;                      \
    r.y = p.y + d;                      \
    r.z = p.z + d;                      \
}

#define VEC_ADD_VEC(r,p,q) {            \
    r.x = p.x + q.x;                    \
    r.y = p.y + q.y;                    \
    r.z = p.z + q.z;                    \
}

#define VEC_DIV_DOUBLE(r,p,d) {         \
    double fInv = 1.0/d;                \
    r.x = p.x * fInv;                   \
    r.y = p.y * fInv;                   \
    r.z = p.z * fInv;                   \
}



#endif
