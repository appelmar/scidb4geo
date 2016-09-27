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

#include "ErrorCodes.h"

namespace scidb4geo {

    // #define X(_name, _code, _msg ) _name = SCIDB_USER_ERROR_CODE_START + _code,
    //       #include "Errors.def"
    // #undef X

    ErrorManager *ErrorManager::_instance = 0;

    ErrorManager *ErrorManager::instance() {
        if (!_instance) _instance = new ErrorManager();
        return _instance;
    }

    ErrorManager::ErrorManager() {
#define X(name, code, msg) errNames.insert(std::pair<StatusCode, string>(name, #name));
#include "Errors.def"
#undef X

#define X(name, code, msg) errMsgs.insert(std::pair<StatusCode, string>(name, msg));
#include "Errors.def"
#undef X

#define X(name, code, msg) errKeys.insert(std::pair<StatusCode, int>(name, code));
#include "Errors.def"
#undef X
    }

    //     void ErrorManager::error ( const string &msg, const StatusCode &code )
    //     {
    //       log4cxx::Logger::getLogger ( SCIDB4GEO_LOGGERNAME )->error ( msg );
    //       throw scidb::UserException(REL_FILE, __FUNCTION__, __LINE__, SCIDB4GEO_ERRORNAMESPACE, scidb::SCIDB_SE_EXECUTION, errKeys[code], "SCIDB_SE_EXECUTION", errNames[code].c_str()) << msg;
    //     }
    //
    //
    //     void ErrorManager::warn ( const string &msg)
    //     {
    //       log4cxx::Logger::getLogger ( SCIDB4GEO_LOGGERNAME )->warn ( msg );
    //     }
    //
    //      void ErrorManager::debug ( const string &msg)
    //     {
    //       log4cxx::Logger::getLogger ( SCIDB4GEO_LOGGERNAME )->debug ( msg );
    //     }

    int scidb4geo::ErrorManager::getKey(scidb4geo::StatusCode code) {
        return errKeys[code];
    }

    std::string scidb4geo::ErrorManager::getMsg(scidb4geo::StatusCode code) {
        return errMsgs[code];
    }

    std::string scidb4geo::ErrorManager::getName(scidb4geo::StatusCode code) {
        return errNames[code];
    }
}
