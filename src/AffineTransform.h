/*
scidb4geo - A SciDB plugin for managing spacetime earth-observation arrays
Copyright (C) 2016 Marius Appel <marius.appel@uni-muenster.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/


#ifndef AFFINE_TRANSFORM_H
#define AFFINE_TRANSFORM_H


#include <sstream>
#include <string>

namespace scidb4geo
{

    using namespace std;


    /**
     * Affine transformations are used to map array integer coordinates to world SRS coordinates.
     * This class stores six parameters of a 2D affine transformation
     * and provides basic serialization functions.
     * @see GDAL data model specification http://www.gdal.org/gdal_datamodel.html
     */
    class AffineTransform
    {
    public:

        struct double2 {
            double2() : x ( 0 ), y ( 0 ) {}
            double2 ( double x, double y ) {
                this->x = x;
                this->y = y;
            }
            double x;
            double y;
        };

        /**
         * Default constructor, creates an identity transformation
         */
        AffineTransform();

        /**
         * Constructor for translation only
         */
        AffineTransform ( double x0, double y0 );

        /**
         * Constructor for translation and scaling, no rotation, shear
        */
        AffineTransform ( double x0, double y0, double a11, double a22 );

        /**
         * Constructor for specification of all parameters
         */
        AffineTransform ( double x0, double y0, double a11, double a22, double a12, double a21 ) ;

        /**
         * Constructor for parsing string representations
         */
        AffineTransform ( const string &astr );

        /**
         * Default destructor
         */
        ~AffineTransform ( );



        /**
         * Creates a string representation
         */
        string toString();

        /**
         * Transformation parameters, _x0, _y0 represent translation. _a11,_a12,_a21,_a22 desribe the 2x2 transformation matrix
         */
        double _x0, _y0, _a11, _a22, _a12, _a21;


        /**
          * Inverse transformation
          */
        AffineTransform *_inv;


        /**
         * Checks whether an affine transformation is the identity function
         */
        bool isIdentity();

        /**
          * Applies transformation to given pointer
          */
        double2 f ( const double2 &v );

        /**
              * Applies transformation to given pointer
              */
        void f ( const double2 &v_in, double2 &v_out );

        void f ( double2 &v );


        /**
          * Applies inverse transformation to a given point
          */
        double2 fInv ( const double2 &v );

        void fInv ( const double2 &v_in, double2 &v_out );

        void fInv ( double2 &v );

        /**
              * Computes the determinant of the linear transformation matrix (without translation)
              */
        double det();
    };




}

#endif

