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

#include "plugin.h"  // Must be first

#include "AffineTransform.h"
#include "ErrorCodes.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

#include "ErrorCodes.h"

namespace scidb4geo {

    using namespace std;

    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("scidb4geo.AffineTransform"));

    AffineTransform::AffineTransform() : _x0(0), _y0(0), _a11(1), _a22(1), _a12(0), _a21(0) {
        deriveInv();
    }
    AffineTransform::AffineTransform(double x0, double y0) : _x0(x0), _y0(y0), _a11(1), _a22(1), _a12(0), _a21(0) {
        deriveInv();
    }
    AffineTransform::AffineTransform(double x0, double y0, double a11, double a22) : _x0(x0), _y0(y0), _a11(a11), _a22(a22), _a12(0), _a21(0) {
        deriveInv();
    }
    AffineTransform::AffineTransform(double x0, double y0, double a11, double a22, double a12, double a21) : _x0(x0), _y0(y0), _a11(a11), _a22(a22), _a12(a12), _a21(a21) {
        deriveInv();
    }
    AffineTransform::AffineTransform(string astr) : _x0(0), _y0(0), _a11(1), _a22(1), _a12(0), _a21(0) {
        vector<string> parts;
        boost::split(parts, astr, boost::is_any_of(",; "));
        for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it) {
            vector<string> kv;
            boost::split(kv, *it, boost::is_any_of("=:"));
            if (kv.size() != 2) {
                stringstream serr;
                serr << "Cannot read affine transformation string '" << astr << "'";
                SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_INVALID_AFFINETRANSFORM_STRING);
                break;
            } else {
                if (kv[0].compare("x0") == 0)
                    _x0 = boost::lexical_cast<double>(kv[1]);
                else if (kv[0].compare("y0") == 0)
                    _y0 = boost::lexical_cast<double>(kv[1]);
                else if (kv[0].compare("a11") == 0)
                    _a11 = boost::lexical_cast<double>(kv[1]);
                else if (kv[0].compare("a22") == 0)
                    _a22 = boost::lexical_cast<double>(kv[1]);
                else if (kv[0].compare("a12") == 0)
                    _a12 = boost::lexical_cast<double>(kv[1]);
                else if (kv[0].compare("a21") == 0)
                    _a21 = boost::lexical_cast<double>(kv[1]);
                else {
                    stringstream serr;
                    serr << "Unknown affine transformation parameter '" + kv[0] + "' will be ignored ";
                    SCIDB4GEO_WARN(serr.str());
                }
            }
        }
        deriveInv();
    }

    AffineTransform::~AffineTransform() {
    }

    void AffineTransform::deriveInv() {
        double d = det();
        if (fabs(d) < __DBL_EPSILON__) {
            SCIDB4GEO_ERROR("Affine transformation not invertible, det=0", SCIDB4GEO_ERR_SINGULARMATRIX);
        }
        double d1 = 1 / d;
        _inv_a11 = d1 * _a22;
        _inv_a12 = d1 * (-_a12);
        _inv_a21 = d1 * (-_a21);
        _inv_a22 = d1 * _a11;
        _inv_x0 = -_inv_a11 * _x0 + _inv_a12 * _y0;
        _inv_y0 = _inv_a21 * _x0 - _inv_a22 * _y0;
    }

    string AffineTransform::toString() const {
        stringstream sstr;
        sstr << setprecision(numeric_limits<double>::digits10)
             << "x0"
             << "=" << _x0 << " "
             << "y0"
             << "=" << _y0 << " "
             << "a11"
             << "=" << _a11 << " "
             << "a22"
             << "=" << _a22 << " "
             << "a12"
             << "=" << _a12 << " "
             << "a21"
             << "=" << _a21;
        return sstr.str();
    }

    bool AffineTransform::isIdentity() const {
        return (_a11 == 1 && _a12 == 0 && _a21 == 0 && _a22 == 1 && _x0 == 0 && _y0 == 0);
    }

    //     AffineTransform::double2 AffineTransform::f ( const double2 &v )
    //     {
    //         double2 result;
    //         result.x = _x0 + _a11 * v.x + _a12 * v.y;
    //         result.y = _y0 + _a21 * v.x + _a22 * v.y;
    //         return result;
    //     }

    void AffineTransform::f(AffineTransform::double2 &v_in, AffineTransform::double2 &v_out) const {
        v_out.x = _x0 + _a11 * v_in.x + _a12 * v_in.y;
        v_out.y = _y0 + _a21 * v_in.x + _a22 * v_in.y;
    }

    void AffineTransform::f(AffineTransform::double2 &v) const {
        double x = v.x;
        v.x = _x0 + _a11 * v.x + _a12 * v.y;
        v.y = _y0 + _a21 * x + _a22 * v.y;
    }

    //     AffineTransform::double2 AffineTransform::fInv ( const double2 &v )
    //     {
    //         if ( _inv == NULL ) {
    //             double d = det();
    //             if ( fabs ( d ) < __DBL_EPSILON__ ) {
    //                 SCIDB4GEO_ERROR ( "Affine transformation not invertible, det=0", SCIDB4GEO_ERR_SINGULARMATRIX );
    //             }
    //             double d1 = 1 / d;
    //             double inv_a11 = d1 * _a22;
    //             double inv_a12 = d1 * ( -_a12 );
    //             double inv_a21 = d1 * ( -_a21 );
    //             double inv_a22 = d1 * _a11;
    //             double inv_x0 = - inv_a11 * _x0 + inv_a12 * _y0;
    //             double inv_y0 =  inv_a21 * _x0 - inv_a22 * _y0;
    //
    //             _inv = new AffineTransform ( inv_x0, inv_y0, inv_a11, inv_a22, inv_a12, inv_a21 );
    //             SCIDB4GEO_DEBUG ( "Inv: " + _inv->toString() );
    //             _inv->_inv = this; // Prevent repreated computations of f, f-1, f, f-1, ... This is dangerous in destruction...
    //         }
    //         return _inv->f ( v );
    //     }

    void AffineTransform::fInv(AffineTransform::double2 &v_in, AffineTransform::double2 &v_out) const {
        v_out.x = _inv_x0 + _inv_a11 * v_in.x + _inv_a12 * v_in.y;
        v_out.y = _inv_y0 + _inv_a21 * v_in.x + _inv_a22 * v_in.y;
    }

    void AffineTransform::fInv(AffineTransform::double2 &v) const {
        double x = v.x;
        v.x = _inv_x0 + _inv_a11 * v.x + _inv_a12 * v.y;
        v.y = _inv_y0 + _inv_a21 * x + _inv_a22 * v.y;
    }

    double AffineTransform::det() const {
        return _a11 * _a22 - _a12 * _a21;
    }
}
