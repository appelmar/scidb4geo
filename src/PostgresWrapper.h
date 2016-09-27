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

#ifndef POSTGRES_WRAPPER_H
#define POSTGRES_WRAPPER_H

#include <log4cxx/logger.h>
#include <boost/concept_check.hpp>
#include <iostream>
#include <map>
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include "AffineTransform.h"
#include "ErrorCodes.h"
#include "TemporalReference.h"

namespace scidb4geo {

    using namespace std;

    /**
     * Data structure for SRS information of arrays
     */
    typedef struct {
        string arrayname;
        string xdim;
        string ydim;
        string auth_name;
        int auth_srid;
        string srtext;
        string proj4text;
        AffineTransform A;
    } SpatialArrayInfo;

    typedef struct {
        string arrayname;
        string setting;
    } EOArrayInfo;

    /**
     * Data structure for TRS information of arrays
     */
    typedef struct {
        string arrayname;
        string tdim;
        TReference *tref;
    } TemporalArrayInfo;

    /**
     * Data structure for spatial reference systems
     */
    typedef struct {
        string auth_name;
        int auth_srid;
        string srtext;
        string proj4text;
    } SRSInfo;

    /**
     * Data structure for bounding boxes (parallel to axes)
     */
    typedef struct {
        double xmin, ymin, xmax, ymax, tmin, tmax, vmin, vmax;
    } EOExtentInfo;

    /**
     * @brief Class for interfacing the Postgres system catalog used for storing spatial reference information
     * This class must be replaced if SRS information should be stored in arrays instead of Postgres.
     */
    class PostgresWrapper {
       public:
        /**
         * Returns the singleton instance
         */
        static PostgresWrapper *instance();

        /**
         * Sets spatial reference of a given arrayname
         */
        void dbSetSpatialRef(const string &arrayname, const string &dim_x, const string &dim_y, const string &auth_name, int auth_srid, AffineTransform &A);

        /**
         * Gets spatial reference systems of a variable list of array namespace
         */
        vector<SpatialArrayInfo> dbGetSpatialRef(const vector<string> &arraynames);

        /**
         * Returns spatial reference information for a single array
         */
        SpatialArrayInfo dbGetSpatialRef(const string &arrayname);

        SpatialArrayInfo dbGetSpatialRefOrEmpty(const string &arrayname);

        /**
         * Returns spatial reference information for all registered arrays
         */
        vector<SpatialArrayInfo> dbGetSpatialRef();

        /**
         * Counts the number of spatially referenced arrays based on a list of array names. Result might be less than arraynames.size()
         */
        int dbGetSpatialRefCount(const vector<string> &arraynames);

        /**
         * Returns the number of spatially referenced arrays
         */
        int dbGetSpatialRefCount();

        /**
         * Inserts a custom reference system to the system catalog
         */
        void dbRegNewSRS(const SRSInfo &info);

        /**
         * Tries to automatically find and add a SRS with given authority name and id from spatialreference.org 
         */
        bool dbRegSRSFromSRORG(const string &auth_name, const int &auth_id);

        /**
         * Sets temporal reference of an array
         * @param
         */
        void dbSetTemporalRef(const string &arrayName, const string &dim_t, const string &t0, const string &dt);

        /**
        * Gets temporal reference systems of a variable list of array namespace
        */
        vector<TemporalArrayInfo> dbGetTemporalRef(const vector<string> &arraynames);

        /**
        * Returns temporal reference information for a single array
        */
        TemporalArrayInfo dbGetTemporalRef(const string &arrayname);

        TemporalArrayInfo dbGetTemporalRefOrEmpty(const string &arrayname);

        /**
         * Returns temporal reference information for all registered arrays
         */
        vector<TemporalArrayInfo> dbGetTemporalRef();

        /**
             * Counts the number of temporally referenced arrays based on a list of array names. Result might be less than arraynames.size()
             */
        int dbGetTemporalRefCount(const vector<string> &arraynames);

        /**
         * Returns the number of temporally referenced arrays
         */
        int dbGetTemporalRefCount();

        /**
            * Returns a list of all referenced arrays and their settings (e.g. st, t, s, sv, ...)
            */
        vector<EOArrayInfo> dbGetArrays();

        /**
             * Gets the total number of referenced arrays, be it spatially, temporally, or vertically
             */
        int dbGetArrayCount();

        /**
         * Gets a key value map of array metadata
         */
        map<string, string> dbGetArrayMD(const string &arrayname, const string &domain = "");

        /**
         * Sets or adds metadata to an array, values for existing key swill be overwritten, values for nonexisting keys will be added.
         */
        void dbSetArrayMD(const string &arrayname, map<string, string> &kv, const string &domain = "");

        /**
         * Gets a key value map of attribute metadata
         */
        map<string, string> dbGetAttributeMD(const string &arrayname, const string &attrname, const string &domain = "");

        /**
         * Sets or adds metadata to an attribute, values for existing keys will be overwritten, values for nonexisting keys will be added.
         */
        void dbSetAttributeMD(const string &arrayname, const string &attrname, map<string, string> &kv, const string &domain = "");

       private:
        static PostgresWrapper *_instance;

        pqxx::connection *_c;

        PostgresWrapper();
        PostgresWrapper(const PostgresWrapper &);
        ~PostgresWrapper();
    };
}

#endif
