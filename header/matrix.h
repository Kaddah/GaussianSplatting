/***************************************************************
 * 
 * TEMPLATIZED MULTIDIMENSIONAL MATRICES
 *
 * Usage example:
 *
 *  Mat4 id = { 1, 0, 0, 0
 *            , 0, 1, 0, 0
 *            , 0, 0, 1, 0
 *            , 0, 0, 0, 1 };
 *
 *  id *= 1;
 *
 *  Vec4 pos = { 0, 0, 0, 1 };
 *  pos = id * pos;
 *
 *  id = transpose( id );
 *
 **************************************************************/

#pragma once

#include "vector.h"

namespace gml {
    template <int rows, int cols> struct Mat {
        union {
            float m[rows][cols];
            float flat[rows * cols];
        };

        float& operator[]( int i ) {
            return this->flat[i];
        }

        const float& operator[]( int i ) const {
            return this->flat[i];
        }
    };

    template<> struct Mat<3,3> {
        union {
            float m[3][3];
            float flat[9];
            struct { float m00, m01, m02, m10, m11, m12, m20, m21, m22; };
            struct { float r00, r01, r02, r10, r11, r12, r20, r21, r22; };
        };

        float& operator[]( int i ) {
            return this->flat[i];
        }

        const float& operator[]( int i ) const {
            return this->flat[i];
        }
    };


    template<> struct Mat<4,4> {
        union {
            float m[4][4];
            float flat[16];
            struct { float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33; };
            struct { float r00, r01, r02,  tx, r10, r11, r12,  ty, r20, r21, r22,  tz; };
        };

        float& operator[]( int i ) {
            return this->flat[i];
        }

        const float& operator[]( int i ) const {
            return this->flat[i];
        }
    };

    typedef Mat<3,3> Mat3;
    typedef Mat<4,4> Mat4;

    Mat3  constructMat3FromRows( Vec3 r0, Vec3 r1, Vec3 r2 );
    Mat3  constructMat3FromCols( Vec3 c0, Vec3 c1, Vec3 c2 );
    Mat4  constructMat4FromRows( Vec4 r0, Vec4 r1, Vec4 r2, Vec4 r3 );
    Mat4  constructMat4FromCols( Vec4 c0, Vec4 c1, Vec4 c2, Vec4 c3 );

    Vec3  getRow0( const Mat3& m );
    Vec3  getRow1( const Mat3& m );
    Vec3  getRow2( const Mat3& m );
    Vec3  getCol0( const Mat3& m );
    Vec3  getCol1( const Mat3& m );
    Vec3  getCol2( const Mat3& m );

    Vec4  getRow0( const Mat4& m );
    Vec4  getRow1( const Mat4& m );
    Vec4  getRow2( const Mat4& m );
    Vec4  getRow3( const Mat4& m );
    Vec4  getCol0( const Mat4& m );
    Vec4  getCol1( const Mat4& m );
    Vec4  getCol2( const Mat4& m );
    Vec4  getCol3( const Mat4& m );

    float determinant( const Mat3& m );
    float determinant( const Mat4& m );
    Mat4  invert     ( const Mat4& m );
    Mat3  invert     ( const Mat3& m );

    Mat4  makePerspective( float fov, float aspectRatio, float near, float far );

    template <int rows, int cols>     void             zero( Mat<rows, cols>& in_out );
    template <int rows, int cols>     Mat<rows, cols>  zero();
    template <int rows, int cols>     void             transpose( const Mat<rows,cols>& in, Mat<cols,rows>& out );
    template <int rows, int cols>     Mat<cols, rows>  transpose( const Mat<rows,cols>& in );
    template <int dim>                Mat<dim, dim>    identity();

    template <int r1, int c1, int c2> Mat<r1, c2>      operator* ( const Mat<r1, c1>& lhs    , const Mat<c1, c2>& rhs     );
    template <int rows, int cols>     Vec<cols>        operator* ( const Vec<rows>& lhs      , const Mat<rows, cols>& rhs ); // row-vector * matrix
    template <int rows, int cols>     Vec<rows>        operator* ( const Mat<rows, cols>& lhs, const Vec<cols>& rhs       ); // matrix * column-vector
    template <int rows, int cols>     Mat<rows, cols>  operator* ( const Mat<rows, cols>& lhs, const float rhs            ); // matrix * scalar
    template <int rows, int cols>     Mat<rows, cols>  operator* ( const float lhs           , const Mat<rows, cols>& rhs ); // scalar * matrix
    template <int rows, int cols>     Mat<rows, cols>  operator+ ( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );
    template <int rows, int cols>     Mat<rows, cols>& operator+=(       Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );
    template <int rows, int cols>     Mat<rows, cols>  operator- ( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );
    template <int rows, int cols>     Mat<rows, cols>& operator-=(       Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );
    template <int rows, int cols>     bool             operator==( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );
    template <int rows, int cols>     bool             operator!=( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs );

    // scale
    // rotate
    // translate

} // namespace gml

#include "matrix.inc"