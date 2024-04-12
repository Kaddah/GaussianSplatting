// DO NOT INCLUDE THIS FILE DIRECTLY; it is automatically included
// with matrix.h

#define _USE_MATH_DEFINES // needed for M_PI to exist on Windows

#include "math_extensions.h"
#include <math.h>
#include <stdlib.h>

namespace gml {
    template <int rows, int cols> void zero( Mat<rows, cols>& in_out ) {
        for ( int i = 0; i < rows * cols; i++ ) {
            in_out[i] = 0;
        }
    }

    template <int rows, int cols> Mat<rows,cols> zero() {
        Mat<rows, cols> out;
        for ( int i = 0; i < rows * cols; i++ ) {
            out[i] = 0;
        }
        return out;
    }

    template<int n>
    Mat<n,n> identity() {
        Mat<n, n> out;
        for ( int i = 0; i < n * n; i++ ) out.flat[i] = 0;
        for ( int i = 0; i < n; i++ )     out.m[i][i] = 1;
        return out;
    }

    template<int rows, int cols>
    void transpose( const Mat<rows,cols>& in, Mat<cols,rows>& out ) {
        for ( int i = 0; i < cols; i++ ) {
            for ( int j = 0; j < rows; j++ ) {
                out.m[i][j] = in.m[j][i];
            }
        }
    }

    template<int rows, int cols>
    Mat<cols, rows> transpose( const Mat<rows,cols>& in ) {
        Mat<cols, rows> out;
        for ( int i = 0; i < cols; i++ ) {
            for ( int j = 0; j < rows; j++ ) {
                out.m[i][j] = in.m[j][i];
            }
        }
        return out;
    }

    template<int r1, int c1, int c2>
    Mat<r1, c2> operator*( const Mat<r1, c1>& lhs, const Mat<c1, c2>& rhs ) {
        Mat<r1, c2> out;
        for ( int i = 0; i < r1; i++ ) {
            for ( int j = 0; j < c2; j++ ) {
                float sum = 0;
                for ( int k = 0; k < c1; k++ ) {
                    sum += lhs.m[i][k] * rhs.m[k][j];
                }
                out.m[i][j] = sum;
            }
        }
        return out;
    }

    template<int rows, int cols>
    Vec<cols> operator*( const Vec<rows>& lhs, const Mat<rows, cols>& rhs ) {
        Vec<cols> out;
        for ( int i = 0; i < cols; i++ ) {
           float sum = 0;
           for ( int j = 0; j < rows; j++ ) {
               sum += lhs.v[j] * rhs.m[j][i];
           }
           out.v[i] = sum;
        }
        return out;
    }

    template<int rows, int cols>
    Vec<rows> operator*( const Mat<rows, cols>& lhs, const Vec<cols>& rhs ) {
        Vec<rows> out;
        for ( int i = 0; i < rows; i++ ) {
           float sum = 0;
           for ( int j = 0; j < cols; j++ ) {
               sum += lhs.m[i][j] * rhs.v[j];
           }
           out.v[i] = sum;
        }
        return out;
    }

    template<int rows, int cols>
    Mat<rows, cols> operator*( const Mat<rows, cols>& lhs, const float rhs ) {
        Mat<rows, cols> out;
        for ( int i = 0; i < rows * cols; i++ ) {
           out.flat[i] = lhs.flat[i] * rhs;
        }
        return out;
    }

    template <int rows, int cols>
    Mat<rows, cols>  operator+ ( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        Mat<rows, cols> out;
        for ( int i = 0; i < rows * cols; i++ ) {
            out.flat[i] = lhs.flat[i] + rhs.flat[i];
        }
        return out;
    }

    template <int rows, int cols>
    Mat<rows, cols> operator*( const float lhs, const Mat<rows, cols>& rhs ) {
        Mat<rows, cols> out;
        for ( int i = 0; i < rows * cols; i++ ) {
            out.flat[i] = lhs * rhs.flat[i];
        }
        return out;
    }

    template <int rows, int cols>
    Mat<rows, cols>& operator+=( Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        for ( int i = 0; i < rows * cols; i++ ) {
            lhs[i] += rhs[i];
        }
        return lhs;
    }

    template <int rows, int cols>
    Mat<rows, cols>  operator- ( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        Mat<rows, cols> out;
        for ( int i = 0; i < rows * cols; i++ ) {
            out[i] = lhs[i] - rhs[i];
        }
        return out;
    }

    template <int rows, int cols>
    Mat<rows, cols>& operator-=( Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        for ( int i = 0; i < rows * cols; i++ ) {
            lhs[i] -= rhs[i];
        }
        return lhs;
    }

    template <int rows, int cols>
    bool operator==( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        for ( int i = 0; i < rows * cols; i++ ) {
            if ( fabs( lhs[i] - rhs[i] ) > ERROR_EPSILON ) return false;
        }
        return true;
    }

    template <int rows, int cols>
    bool operator!=( const Mat<rows, cols>& lhs, const Mat<rows, cols>& rhs ) {
        for ( int i = 0; i < rows * cols; i++ ) {
            if ( fabs( lhs[i] - rhs[i] ) > ERROR_EPSILON ) return true;
        }
        return false;
    }
}