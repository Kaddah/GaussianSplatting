/***************************************************************
 * 
 * TEMPLATIZED MULTIDIMENSIONAL VECTORS
 *
 * Usage example:
 *
 *  Vec4 pos = { 0, 0, 0, 1 };
 *
 *  pos += { 1, 1, 1, 0 };
 *  pos /= { 1, 1, 1, 1 };
 *  pos *= { 1, 1, 1, 1 };
 *
 *  float x = pos.x;
 *  float y = pos.y;
 *  float z = pos.z;
 *  float w = pos.w;
 *
 *  pos.w = 0;
 *  Vec4 normalized = normalize( pos );
 *
 **************************************************************/

#pragma once

namespace gml {
    // base vector struct
    template <int n> struct Vec {
        float v[n];

        float& operator[]( int i ) {
            return this->v[i];
        }

        const float& operator[]( int i ) const {
            return this->v[i];
        }
    };

    template<> struct Vec<2> {
        union {
            float v[2];
            struct { float x, y; };
            struct { float r, g; };
        };

        Vec( float v = 0 ): x( v ), y( v ) {}
        Vec( float x_, float y_ ): x( x_ ), y( y_ ) {}

        float& operator[]( int i ) {
            return this->v[i];
        }

        const float& operator[]( int i ) const {
            return this->v[i];
        }
    };

    template<> struct Vec<3> {
        union {
            float v[3];
            struct { float x, y, z; };
            struct { float r, g, b; };
            Vec<2> xy;
        };

        Vec( float v = 0 ): x( v ), y( v ), z( v ) {}
        Vec( float _x, float _y, float _z ): x( _x ), y( _y ), z( _z ) {}

        float& operator[]( int i ) {
            return this->v[i];
        }

        const float& operator[]( int i ) const {
            return this->v[i];
        }
    };

    template<> struct Vec<4> {
        union {
            float v[4];
            struct { float x, y, z, w; };
            struct { float r, g, b, a; };
            Vec<2> xy;
            Vec<3> xyz;
            Vec<3> rgb;
        };

        Vec( float v = 0 ): x( v ) , y( v ), z( v ), w( v )  {}
        Vec( float _x, float _y, float _z, float _w ): x( _x ), y( _y ), z( _z ), w( _w ) {}

        float& operator[]( int i ) {
            return this->v[i];
        }

        const float& operator[]( int i ) const {
            return this->v[i];
        }
    };

    typedef Vec<2> Vec2;
    typedef Vec<3> Vec3;
    typedef Vec<4> Vec4;

    // generic operators
    template <int n> Vec<n>  operator+ ( const Vec<n> lhs , const Vec<n> rhs );
    template <int n> Vec<n>& operator+=( Vec<n>& lhs      , const Vec<n> rhs );
    template <int n> Vec<n>  operator- ( const Vec<n> lhs , const Vec<n> rhs );
    template <int n> Vec<n>& operator-=( Vec<n>& lhs      , const Vec<n> rhs );
    template <int n> Vec<n>  operator- ( const Vec<n> lhs                    );
    template <int n> Vec<n>  operator* ( const Vec<n> lhs , const Vec<n> rhs );
    template <int n> Vec<n>& operator*=( Vec<n>& lhs      , const Vec<n> rhs );
    template <int n> Vec<n>  operator* ( const Vec<n> lhs , const float rhs  );
    template <int n> Vec<n>& operator*=( Vec<n>& lhs      , const float rhs  );
    template <int n> Vec<n>  operator* ( const float lhs  , const Vec<n> rhs );
    template <int n> Vec<n>  operator/ ( const Vec<n> lhs , const Vec<n> rhs );
    template <int n> Vec<n>& operator/=( Vec<n>& lhs      , const Vec<n> rhs );
    template <int n> Vec<n>  operator/ ( const Vec<n> lhs , const float rhs  );
    template <int n> Vec<n>& operator/=( Vec<n>& lhs      , const float rhs  );
    template <int n> Vec<n>  operator^ ( const Vec<n> lhs , const Vec<n> rhs );
    template <int n> Vec<n>& operator^=( Vec<n>& lhs      , const Vec<n> rhs );
    template <int n> Vec<n>  operator^ ( const Vec<n> lhs , const float rhs  );
    template <int n> Vec<n>& operator^=( Vec<n>& lhs      , const float rhs  );
    template <int n> bool    operator==( const Vec<n>& lhs, const Vec<n>& rhs );
    template <int n> bool    operator!=( const Vec<n>& lhs, const Vec<n>& rhs );

    // vector-specific operations
    template <int n> float   dot       ( const Vec<n> lhs, const Vec<n> rhs );
    template <int n> Vec<n>  cross     ( const Vec<n> lhs, const Vec<n> rhs );
                     float   cross2D   ( const Vec2   lhs, const Vec2   rhs ); // only for vec2s
    template <int n> float   lengthsq  ( const Vec<n> in );
    template <int n> float   length    ( const Vec<n> in );
    template <int n> Vec<n>  normalize ( const Vec<n> in );

    // free-floating constructors
    template <int n> Vec<n>  zero      ( void );
    template <int n> void    zero      ( Vec<n>& in_out );

    // picks points on the surface of the n-sphere (n minus one-sphere)
    template <int n> Vec<n>  randomOnSphere ( float radius = 1.f );
    template <int n> void    randomOnSphere ( Vec<n>& in_out, float radius = 1.f );

    // picks points inside the n-sphere (n minus one-sphere)
    template <int n> Vec<n>  randomInSphere ( float radius = 1.f );
    template <int n> void    randomInSphere ( Vec<n>& in_out, float radius = 1.f );

    // picks a point inside a 2D disk
                     Vec2    randomInDisk   ( float radius = 1.f );

} // namespace gml

#include "vector.inc"