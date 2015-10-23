/*
scidb4geo - A SciDB plugin for managing spatially referenced arrays
Copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>

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


#ifndef UTILS_H
#define UTILS_H



#include <sstream>
#include <string>
#include <cmath>

namespace scidb4geo
{


    namespace utils
    {


        double min ( const double *v, const size_t &n )
        {
            if ( n == 0 ) return NAN;
            double m = v[0];
            for ( size_t i = 1; i < n; ++i ) {
                if ( v[i] < m ) m = v[i];
            }
            return m;
        }

        double max ( const double *v, const size_t &n )
        {
            if ( n == 0 ) return NAN;
            double m = v[0];
            for ( size_t i = 1; i < n; ++i ) {
                if ( v[i] > m ) m = v[i];
            }
            return m;
        }


    }




}




#endif
