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
#include "PostgresWrapper.h"
#include "plugin.h"

#include <system/SystemCatalog.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <curl/curl.h>

namespace scidb4geo {

    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("scidb4geo.PostgresWrapper"));

    PostgresWrapper *PostgresWrapper::_instance = 0;

    PostgresWrapper *PostgresWrapper::instance() {
        if (!_instance)
            _instance = new PostgresWrapper();
        return _instance;
    }

    PostgresWrapper::PostgresWrapper() {
        // Extract connection information from scidb cluster configuration file that has been copied to ~/.scidb4geo.config.ini before
        string cluster, dbuser, dbpw, pluginsdir;
        stringstream confpath;
        confpath << getenv("HOME") << "/.scidb4geo/config.ini";
        boost::property_tree::ptree pt;
        boost::property_tree::ini_parser::read_ini(confpath.str(), pt);

        // Assuming only one cluster definition in scidb config file
        cluster = pt.front().first;
        dbuser = pt.get<string>("" + cluster + ".db_user", "postgres");
        dbpw = pt.get<string>("" + cluster + ".db_passwd", "postgres");
        pluginsdir = pt.get<string>("" + cluster + ".pluginsdir", pt.get<string>("" + cluster + ".install_root", "/opt/scidb/14.12") + "/lib/scidb/plugins");

        // Build connection string and connect
        stringstream constr;
        constr << "host=localhost port=5432 dbname= " << cluster << " user=" << dbuser << " password=" << dbpw;
        SCIDB4GEO_DEBUG("Connecting to Postgres catalog with connection string '" + constr.str() + "'");

        try {
            _c = new pqxx::connection(constr.str());
        } catch (const std::exception &e) {
            _c = NULL;
            SCIDB4GEO_ERROR("Postgres connection failed", SCIDB4GEO_ERR_POSTGRES_CONNECTION_FAILED);
        }
    }

    PostgresWrapper::~PostgresWrapper() {
        SCIDB4GEO_DEBUG("Disconnecting from Postgres catalog");
        _c->disconnect();
        delete _c;
    }

    void PostgresWrapper::dbSetSpatialRef(const string &arrayname, const string &dim_x, const string &dim_y, const string &auth_name, int auth_srid, AffineTransform &A) {
        // 1. Check whether auth_name, auth_srid exists in spatial_ref_sys table, if so, return internal SRID
        // "select srid from scidb4geo_spatialrefsys where  lower(trim(both ' ' from auth_name)) = lower(trim(both ' ' from ?1)) and auth_srid = ?2;"
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select srid from scidb4geo_spatialrefsys where  lower(auth_name) = lower('" << auth_name << "') and auth_srid = " << auth_srid << ";";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());

        txn.commit();

        // if #results = 0, throw exception unknown SRS
        // else if #nresults > 1 warning, take first
        // store returned ID
        if (r.size() == 0) {
            // Avoid nested transactions

            //Try to lookup at spatialreference.org
            if (!dbRegSRSFromSRORG(auth_name, auth_srid)) {
                stringstream serr;
                serr << "Unknown SRS '" << auth_name << ":" << auth_srid << "'";
                SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNKNOWN_SRS);
                return;
            } else {
                pqxx::work txn2(*_c);
                // Try once more...
                SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
                r = txn2.exec(q.str());
                if (r.size() == 0) {
                    stringstream serr;
                    serr << "Unknown SRS '" << auth_name << ":" << auth_srid << "'";
                    SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNKNOWN_SRS);
                    return;
                }
                txn2.commit();
            }

        } else if (r.size() > 1) {
            stringstream ss;
            ss << "Found multiple entries for SRS" << auth_name << ":" << auth_srid << "; using first entry";
            SCIDB4GEO_WARN(ss.str());
        }

        int srid = r[0][0].as<int>();

        pqxx::work txn3(*_c);

        // 2. make sure that refsys will be changed if already set
        // delete from scidb4geo_array_s where arrayname = ?1;
        q.str("");
        q << "delete from scidb4geo_array_s where arrayname = '" << arrayname << "';";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        txn3.exec(q.str());

        // 3. Add to scidb4geo_array_s table
        // insert into scidb4geo_array_s(arrayname, xdim, ydim,
        q.str("");
        q << setprecision(numeric_limits<double>::digits10) << "insert into scidb4geo_array_s(arrayname, xdim, ydim, srid, x0,y0,a11,a12,a21,a22) values('" << arrayname << "','" << dim_x << "','" << dim_y << "'," << srid << ","
          << A._x0 << "," << A._y0 << "," << A._a11 << "," << A._a12 << "," << A._a21 << "," << A._a22 << ");";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        txn3.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn3.commit();
    }

    vector<SpatialArrayInfo> PostgresWrapper::dbGetSpatialRef(const vector<string> &arraynames) {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select xdim,ydim,x0,y0,a11,a12,a21,a22,auth_name,auth_srid,srtext,proj4text,arrayname from scidb4geo_array_s a inner join scidb4geo_spatialrefsys r on a.srid = r.srid";
        if (arraynames.size() > 0) {
            q << " where"
              << " arrayname='" << arraynames[0] << "'";
            for (size_t i = 1; i < arraynames.size(); ++i) {
                q << " or arrayname='" << arraynames[i] << "'";
            }
        }
        q << ";";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        vector<SpatialArrayInfo> info_list;
        for (size_t i = 0; i < r.size(); ++i) {
            SpatialArrayInfo info;

            info.xdim = r[i][0].as<string>();  // TODO r[0]["xdim"] etc would improve readability
            info.ydim = r[i][1].as<string>();
            AffineTransform A(r[i][2].as<double>(), r[i][3].as<double>(), r[i][4].as<double>(), r[i][7].as<double>(), r[i][5].as<double>(), r[i][6].as<double>());
            info.A = A;
            info.auth_name = r[i][8].as<string>();
            info.auth_srid = r[i][9].as<int>();
            info.srtext = r[i][10].as<string>();
            info.proj4text = r[i][11].as<string>();
            info.arrayname = r[i][12].as<string>();
            info_list.push_back(info);
        }

        return info_list;
    }

    vector<SpatialArrayInfo> PostgresWrapper::dbGetSpatialRef() {
        vector<string> v(0);
        return dbGetSpatialRef(v);
    }

    SpatialArrayInfo PostgresWrapper::dbGetSpatialRef(const string &arrayname) {
        vector<string> in;
        in.push_back(arrayname);
        vector<SpatialArrayInfo> out = PostgresWrapper::instance()->dbGetSpatialRef(in);  // must be called via singleton
        if (out.size() != 1) {
            stringstream serr;
            serr << "Cannot find spatial reference for array '" << arrayname << "'";
            SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNDECLARED_ARRAY);
        }
        return out[0];
    }

    SpatialArrayInfo PostgresWrapper::dbGetSpatialRefOrEmpty(const string &arrayname) {
        vector<string> in;
        in.push_back(arrayname);
        vector<SpatialArrayInfo> out = PostgresWrapper::instance()->dbGetSpatialRef(in);  // must be called via singleton
        if (out.size() != 1) {
            SpatialArrayInfo srs;
            srs.arrayname = "";
            out.push_back(srs);
        }
        return out[0];
    }

    int PostgresWrapper::dbGetSpatialRefCount(const vector<string> &arraynames) {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select count(*) from scidb4geo_array_s a inner join scidb4geo_spatialrefsys r on a.srid = r.srid";
        if (arraynames.size() > 0)
            q << " where "
              << " arrayname='" << arraynames[0] << "'";
        for (size_t i = 1; i < arraynames.size(); ++i) {
            q << " or arrayname='" << arraynames[i] << "'";
        }
        q << ";";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        int count = r[0][0].as<int>();

        return count;
    }

    int PostgresWrapper::dbGetSpatialRefCount() {
        vector<string> v(0);
        return dbGetSpatialRefCount(v);
    }

    void PostgresWrapper::dbRegNewSRS(const SRSInfo &info) {
        pqxx::work txn(*_c);
        stringstream q;  // query string

        // 1. Check whether (auth_name, auth_id exists), maybe optional if constraint unique(auth_name,auth_id)
        q.str("");
        q << "select count(*) from scidb4geo_spatialrefsys where lower(auth_name) = lower('" << info.auth_name << "') and auth_srid = " << info.auth_srid << ";";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        int count = r[0][0].as<int>();
        if (count != 0) {
            stringstream serr;
            serr << " SRS '" + (string)info.auth_name << ":" << info.auth_srid << "' already exists in system catalog!";
            SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_SRS_ALREADY_EXISTING);
            return;
        }

        // 2. insert
        pqxx::work txn2(*_c);
        q.str("");
        q << "insert into scidb4geo_spatialrefsys (auth_name, auth_srid, srtext, proj4text) values ('" << info.auth_name << "'," << info.auth_srid << ",'" << info.srtext << "','" << info.proj4text << "');";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        txn2.exec(q.str());
        txn2.commit();
    }

    static size_t responseToStringCallback(void *ptr, size_t size, size_t count, void *stream) {
        ((string *)stream)->append((char *)ptr, 0, size * count);
        return size * count;
    }

    bool PostgresWrapper::dbRegSRSFromSRORG(const string &auth_name, const int &auth_id) {
        using namespace std;

        // 1. Try to find SRS at spatialreference.org

        stringstream url_proj4, url_wkt;
        string auth_name_lower = auth_name;
        std::transform(auth_name_lower.begin(), auth_name_lower.end(), auth_name_lower.begin(), ::tolower);
        url_proj4 << "http://www.spatialreference.org/ref/" << auth_name_lower.c_str() << "/" << auth_id << "/proj4/";
        url_wkt << "http://www.spatialreference.org/ref/" << auth_name_lower.c_str() << "/" << auth_id << "/ogcwkt/";

        CURLcode res;
        char errbuf[CURL_ERROR_SIZE];
        errbuf[0] = 0;
        string proj4, wkt;

        try {
            stringstream ss;
            ss << "Trying to find SRS definition for '" << auth_name_lower.c_str() << ":" << auth_id << "' from spatialreference.org";
            SCIDB4GEO_DEBUG(ss.str());
            CURL *curl_handle = curl_easy_init();
            curl_easy_setopt(curl_handle, CURLOPT_PORT, 80);
            curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
            curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errbuf);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &responseToStringCallback);

            curl_easy_setopt(curl_handle, CURLOPT_URL, url_proj4.str().c_str());
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &proj4);

            ss.str("");
            ss << "Performing HTTP GET " << url_proj4.str().c_str();
            SCIDB4GEO_DEBUG(ss.str());

            res = curl_easy_perform(curl_handle);
            if (res != CURLE_OK) {
                throw res;
            }

            curl_easy_setopt(curl_handle, CURLOPT_URL, url_wkt.str().c_str());
            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &wkt);

            ss.str("");
            ss << "Performing HTTP GET " << url_wkt.str().c_str();
            SCIDB4GEO_DEBUG(ss.str());

            res = curl_easy_perform(curl_handle);
            if (res != CURLE_OK) {
                throw res;
            }

            curl_easy_cleanup(curl_handle);

            if (proj4.length() == 0 || wkt.length() == 0) {
                throw 0;
            }

        }

        catch (int e) {
            stringstream serr;
            serr << "Error while trying to find spatial reference system '" << auth_name.c_str() << ":" << auth_id << "' at spatialreference.org. Returned " << errbuf << ".";
            SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNKNOWN_SRS);
            return false;
        }

        stringstream ss;
        ss << "Sucessfully found SRS definition for '" << auth_name_lower.c_str() << ":" << auth_id << "' from spatialreference.org:\nPROJ4: " << proj4 << "\nWKT:" << wkt << "\n";
        SCIDB4GEO_DEBUG(ss.str());

        SRSInfo srs;
        srs.auth_name = auth_name;
        srs.auth_srid = auth_id;
        srs.srtext = wkt;
        srs.proj4text = proj4;

        dbRegNewSRS(srs);

        return true;
    }

    void PostgresWrapper::dbSetTemporalRef(const string &arrayName, const string &dim_t, const string &t0, const string &dt) {
        pqxx::work txn(*_c);
        stringstream q;  // query string

        // 1. make sure that refsys will be changed if already set
        // delete from scidb4geo_array_t where arrayname = ?1;
        q.str("");
        q << "delete from scidb4geo_array_t where arrayname = '" << arrayName << "';";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        txn.exec(q.str());

        // 2. Add to scidb4geo_array_t table
        // insert into scidb4geo_array_t(arrayname, tdim, t0, dt)
        q.str("");
        q << setprecision(numeric_limits<double>::digits10) << "insert into scidb4geo_array_t(arrayname, tdim, t0, dt) values('" << arrayName << "','" << dim_t << "','" << t0 << "','" << dt << "');";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        txn.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();
    }

    vector<TemporalArrayInfo> PostgresWrapper::dbGetTemporalRef(const vector<string> &arraynames) {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select arrayname, tdim, t0, dt  from scidb4geo_array_t";
        if (arraynames.size() > 0) {
            q << " where "
              << " arrayname='" << arraynames[0] << "'";
            for (size_t i = 1; i < arraynames.size(); ++i) {
                q << "or arrayname='" << arraynames[i] << "'";
            }
        }
        q << ";";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        vector<TemporalArrayInfo> info_list;
        for (size_t i = 0; i < r.size(); ++i) {
            TemporalArrayInfo info;

            info.arrayname = r[i][0].as<string>();
            info.tdim = r[i][1].as<string>();  // TODO r[0]["tdim"] etc would improve readability
            info.tref = new TReference(r[i][2].as<string>(), r[i][3].as<string>());
            // info.t0_text = r[i][2].as<string>();
            // info.dt_text = r[i][3].as<string>();
            //info.t0_timestamp= r[i][2].as<double>();
            //info.dt_seconds = r[i][3].as<double>();
            info_list.push_back(info);
        }

        SCIDB4GEO_DEBUG("Transaction done");

        return info_list;
    }

    vector<TemporalArrayInfo> PostgresWrapper::dbGetTemporalRef() {
        vector<string> v(0);
        return dbGetTemporalRef(v);
    }

    TemporalArrayInfo PostgresWrapper::dbGetTemporalRef(const string &arrayname) {
        vector<string> in;
        in.push_back(arrayname);
        vector<TemporalArrayInfo> out = PostgresWrapper::instance()->dbGetTemporalRef(in);  // must be called via singleton
        if (out.size() != 1) {
            stringstream serr;
            serr << "Cannot find temporal reference for array '" << arrayname << "'";
            SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNDECLARED_ARRAY);
        }
        return out[0];
    }

    TemporalArrayInfo PostgresWrapper::dbGetTemporalRefOrEmpty(const string &arrayname) {
        vector<string> in;
        in.push_back(arrayname);
        vector<TemporalArrayInfo> out = PostgresWrapper::instance()->dbGetTemporalRef(in);  // must be called via singleton
        if (out.size() != 1) {
            TemporalArrayInfo trs;
            trs.arrayname = "";
            return trs;
        }
        return out[0];
    }

    int PostgresWrapper::dbGetTemporalRefCount(const vector<string> &arraynames) {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select count(*) from scidb4geo_array_t";
        if (arraynames.size() > 0)
            q << " where "
              << " arrayname='" << arraynames[0] << "'";
        for (size_t i = 1; i < arraynames.size(); ++i) {
            q << "or arrayname='" << arraynames[i] << "'";
        }
        q << ";";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        int count = r[0][0].as<int>();

        return count;
    }

    int PostgresWrapper::dbGetTemporalRefCount() {
        vector<string> v(0);
        return dbGetTemporalRefCount(v);
    }

    vector<EOArrayInfo> PostgresWrapper::dbGetArrays() {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "SELECT "
             "CASE WHEN  S.arrayname IS NULL AND  T.arrayname IS NULL THEN V.arrayname WHEN S.arrayname IS NULL THEN T.arrayname ELSE S.arrayname END AS arrayname, "
             "CASE WHEN  S.arrayname IS NULL THEN '' ELSE 's' END || CASE WHEN  T.arrayname IS NULL THEN '' ELSE 't' END || CASE WHEN  V.arrayname IS NULL THEN '' ELSE 'v' END as setting "
             "FROM scidb4geo_array_s AS S FULL OUTER JOIN scidb4geo_array_t AS T ON S.arrayname = T.arrayname FULL OUTER JOIN scidb4geo_array_v AS V ON S.arrayname=V.arrayname ORDER BY arrayname;";

        // NOT COMPATIBLE WITH PG 8.4 (string_agg missing)
        //         q <<
        //           "SELECT arrayname, string_agg(setting,'') AS setting FROM "
        //           "(SELECT arrayname, varchar(4) 's' AS setting FROM scidb4geo_array_s UNION "
        //           "SELECT arrayname, varchar(4) 't' AS setting  FROM scidb4geo_array_t UNION "
        //           "SELECT arrayname, varchar(4) 'v' AS setting  FROM scidb4geo_array_v) "
        //           "AS A GROUP BY arrayname ORDER BY arrayname;";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        vector<EOArrayInfo> info_list;
        for (size_t i = 0; i < r.size(); ++i) {
            EOArrayInfo info;
            info.arrayname = r[i][0].as<string>();
            info.setting = r[i][1].as<string>();
            info_list.push_back(info);
        }

        return info_list;
    }

    int PostgresWrapper::dbGetArrayCount() {
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select COUNT (DISTINCT arrayname) FROM "
             "(select arrayname from scidb4geo_array_s union "
             "select  arrayname from scidb4geo_array_t union "
             "select  arrayname from scidb4geo_array_v) as A;";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();
        int count = r[0][0].as<int>();

        return count;
    }

    void PostgresWrapper::dbSetArrayMD(const string &arrayname, map<string, string> &kv, const string &domain) {
        // 1. Check whether there is a metadata row for the array, if not create
        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select count(*) from scidb4geo_array_md where arrayname='" << arrayname << "' and domainname='" << domain << "';";
        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());
        int count = r[0][0].as<int>();  // Does this work without commit?
        if (count == 0) {
            q.str("");
            q << "insert into scidb4geo_array_md(arrayname, domainname, kv) values('" << arrayname << "','" << domain << "','');";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
        }

        // 2. Iterate over all key value pairs in input map and add it
        stringstream key_array_str;
        stringstream val_array_str;
        val_array_str << "array[";
        key_array_str << "array[";

        uint32_t i = 0;
        for (std::map<string, string>::iterator it = kv.begin(); it != kv.end(); ++it) {
            key_array_str << "'" << it->first << "'";
            val_array_str << "'" << it->second << "'";
            if (i++ < kv.size() - 1) {
                val_array_str << ",";
                key_array_str << ",";
            }
        }
        val_array_str << "]";
        key_array_str << "]";

        // Delete existing keys first
        {
            //             q.str ( "" );
            //             q << "UPDATE scidb4geo_array_md SET kv = delete(kv, " << key_array_str.str() << ") where arrayname='" << arrayname << "' and domainname='" << domain << "';";
            //             SCIDB4GEO_DEBUG ( "Performing SQL query '" + q.str() + "'" );
            //             pqxx::result r = txn.exec ( q.str() );
        }

        {
            q.str("");
            q << "UPDATE scidb4geo_array_md SET kv = kv || hstore(" << key_array_str.str() << "," << val_array_str.str() << ")  where arrayname='" << arrayname << "' and domainname='" << domain << "';";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
        }

        SCIDB4GEO_DEBUG("Committing SQL transaction");

        txn.commit();
    }

    map<string, string> PostgresWrapper::dbGetArrayMD(const string &arrayname, const string &domain) {
        map<string, string> result;

        pqxx::work txn(*_c);
        stringstream q;  // query string
        q.str("");

        q << "select (each(kv)).key, (each(kv)).value from scidb4geo_array_md where arrayname='" << arrayname << "' and domainname='" << domain << "';";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();
        if (r.size() == 0) {
            SCIDB4GEO_WARN("Cannot find metadata entries for array '" + arrayname + "' and domain + '" + domain + "'");
        }

        for (size_t i = 0; i < r.size(); ++i) {
            result.insert(std::pair<string, string>(r[i][0].as<string>(), r[i][1].as<string>()));
        }

        return result;
    }

    void PostgresWrapper::dbSetAttributeMD(const string &arrayname, const string &attrname, map<string, string> &kv, const string &domain) {
        // 1. Find array id
        pqxx::work txn(*_c);
        stringstream q;  // query string

        int arrayid;
        {
            q.str("");
            q << "select id from \"array\" where name='" << arrayname << "';";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
            if (r.size() == 0) {
                SCIDB4GEO_ERROR("Cannot find array", SCIDB4GEO_ERR_UNDECLARED_ARRAY);
            }
            arrayid = r[0][0].as<int>();  // Does this work without commit?
        }

        {
            q.str("");
            q << "select count(*) from \"array_attribute\" where name='" << attrname << "' and array_id = " << arrayid << ";";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
            int count = r[0][0].as<int>();
            if (count == 0) {
                SCIDB4GEO_ERROR("Cannot find attribute", SCIDB4GEO_ERR_UNDECLARED_ATTRIBUTE);
            }
        }

        // 2. If needed create an entry in attribute metadata table
        {
            q.str("");
            q << "select count(*) from scidb4geo_attribute_md where arrayid='" << arrayid << "' and attrname='" << attrname << "' and domainname='" << domain << "';";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
            int count = r[0][0].as<int>();  // Does this work without commit?
            if (count == 0) {
                q.str("");
                q << "insert into scidb4geo_attribute_md(arrayid, attrname, domainname, kv) values('" << arrayid << "','" << attrname << "','" << domain << "','');";
                SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
                pqxx::result r = txn.exec(q.str());
            }
        }

        // 3. Iterate over all key value pairs in input map and add it
        stringstream key_array_str;
        stringstream val_array_str;
        val_array_str << "array[";
        key_array_str << "array[";

        uint32_t i = 0;
        map<string, string>::iterator it;
        for (it = kv.begin(); it != kv.end(); ++it) {
            key_array_str << "'" << it->first << "'";
            val_array_str << "'" << it->second << "'";
            if (i++ < kv.size() - 1) {
                val_array_str << ",";
                key_array_str << ",";
            }
        }
        val_array_str << "]";
        key_array_str << "]";

        // Delete existing keys first
        {
            //             q.str ( "" );
            //             q << "UPDATE scidb4geo_attribute_md SET kv = delete(kv, " << key_array_str.str() << ") where arrayid='" << arrayid << "' and attrname='" << attrname << "' and domainname='" << domain << "';";
            //             SCIDB4GEO_DEBUG ( "Performing SQL query '" + q.str() + "'" );
            //             pqxx::result r = txn.exec ( q.str() );
        }

        {
            q.str("");
            q << "UPDATE scidb4geo_attribute_md SET kv = kv || hstore(" << key_array_str.str() << "," << val_array_str.str() << ")  where arrayid='" << arrayid << "' and attrname='" << attrname << "' and domainname='" << domain << "';";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
        }
        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();
    }

    map<string, string> PostgresWrapper::dbGetAttributeMD(const string &arrayname, const string &attrname, const string &domain) {
        map<string, string> result;

        pqxx::work txn(*_c);
        stringstream q;  // query string

        int arrayid;
        {
            q.str("");
            q << "select id from \"array\" where name='" << arrayname << "';";
            SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
            pqxx::result r = txn.exec(q.str());
            if (r.size() == 0) {
                SCIDB4GEO_ERROR("Cannot find array", SCIDB4GEO_ERR_UNDECLARED_ARRAY);
            }
            arrayid = r[0][0].as<int>();  // Does this work without commit?
        }

        q.str("");
        q << "select (each(kv)).key, (each(kv)).value from scidb4geo_attribute_md where arrayid='" << arrayid << "' and attrname='" << attrname << "' and domainname='" << domain << "';";

        SCIDB4GEO_DEBUG("Performing SQL query '" + q.str() + "'");
        pqxx::result r = txn.exec(q.str());

        SCIDB4GEO_DEBUG("Committing SQL transaction");
        txn.commit();

        if (r.size() == 0) {
            SCIDB4GEO_WARN("Cannot find metadata entries for array '" + arrayname + "' and attrname='" + attrname + "' and domain '" + domain + "'");
        }

        for (size_t i = 0; i < r.size(); ++i) {
            result.insert(std::pair<string, string>(r[i][0].as<string>(), r[i][1].as<string>()));
        }

        return result;
    }
}
