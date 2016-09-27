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

#include "plugin.h"

#include "ErrorCodes.h"

#include <SciDBAPI.h>
#include <system/Constants.h>
//#include <util/PluginManager.h>
//#include <query/Parser.h>

EXPORTED_FUNCTION void GetPluginVersion(uint32_t &major, uint32_t &minor, uint32_t &patch, uint32_t &build) {
    // Must match SciDB version, otherwise the plugin cannot be loaded
    major = scidb::SCIDB_VERSION_MAJOR();
    minor = scidb::SCIDB_VERSION_MINOR();
    patch = scidb::SCIDB_VERSION_PATCH();
    build = scidb::SCIDB_VERSION_BUILD();
    //     major = SCIDB4GEO_VERSION_MAJOR;
    //     minor = SCIDB4GEO_VERSION_MINOR;
    //     patch = 0; // TODO: set via cmake
    //     build = 0; // TODO: set via cmake
}

class Instance {
   public:
    Instance() {
//register error messages

#define X(name, code, msg) _msg[code] = msg;
#include "Errors.def"

#undef X

        scidb::ErrorsLibrary::getInstance()->registerErrors(SCIDB4GEO_ERRORNAMESPACE, &_msg);

        // Load macro automatically

        //scidb::loadModule ( "/home/scidb/.scidb4geo/scidb4geo_macro.afl" );
    }

    ~Instance() {
        scidb::ErrorsLibrary::getInstance()->unregisterErrors(SCIDB4GEO_ERRORNAMESPACE);
    }

   private:
    scidb::ErrorsLibrary::ErrorsMessages _msg;

} _instance;
