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

#ifndef SCIDB4GEO_ERRORCODES_H
#define SCIDB4GEO_ERRORCODES_H

#include <log4cxx/logger.h>
#include <system/Constants.h>
#include <system/ErrorCodes.h>
#include <system/ErrorsLibrary.h>
#include <system/Exceptions.h>
#include <map>

#define SCIDB4GEO_LOGGERNAME "scidb4geo"
#define SCIDB4GEO_ERRORNAMESPACE "scidb4geo"

// Error macro need to be used to print correct function names and line numbers in exceptions
#define SCIDB4GEO_ERROR(msg, code)                                \
    log4cxx::Logger::getLogger(SCIDB4GEO_LOGGERNAME)->error(msg); \
    throw scidb::UserException(REL_FILE, __FUNCTION__, __LINE__, SCIDB4GEO_ERRORNAMESPACE, scidb::SCIDB_SE_EXECUTION, ErrorManager::instance()->getKey(code), "SCIDB_SE_EXECUTION", ErrorManager::instance()->getName(code).c_str()) << msg

#define SCIDB4GEO_WARN(msg) \
    log4cxx::Logger::getLogger(SCIDB4GEO_LOGGERNAME)->warn(msg)

#define SCIDB4GEO_DEBUG(msg) \
    log4cxx::Logger::getLogger(SCIDB4GEO_LOGGERNAME)->debug(msg);

namespace scidb4geo {

    using namespace std;

    enum StatusCode {
#define X(name, code, msg) name = code,
#include "Errors.def"
#undef X
        DUMMY = SCIDB_USER_ERROR_CODE_START + 9999999
    };

    class ErrorManager {
       public:
        static ErrorManager *instance();

        string getMsg(StatusCode code);
        string getName(StatusCode code);
        int getKey(StatusCode code);

       private:
        ErrorManager();

        ErrorManager(const ErrorManager &);
        ~ErrorManager();

        static ErrorManager *_instance;

        map<StatusCode, string> errNames;
        map<StatusCode, string> errMsgs;
        map<StatusCode, int> errKeys;
    };
}

#endif
