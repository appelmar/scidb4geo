-- scidb4geo - A SciDB plugin for managing spatially referenced arrays
-- Copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU Affero General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU Affero General Public License for more details.
--
-- You should have received a copy of the GNU Affero General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.
-- -----------------------------------------------------------------------------

-- Uninstallation script to safely remove schemas, functions, and triggers of spatial reference metadata in SciDB's system catalog


DROP TRIGGER IF EXISTS scidb4geo_trig_dimension_rename  ON "array_dimension";
DROP TRIGGER IF EXISTS scidb4geo_trig_dimension_remove ON "array_dimension";
DROP TRIGGER IF EXISTS scidb4geo_trig_array_rename ON "array";
DROP TRIGGER IF EXISTS scidb4geo_trig_array_remove ON "array";
DROP TRIGGER IF EXISTS scidb4geo_trig_attribute_rename ON "array_attribute";
DROP TRIGGER IF EXISTS scidb4geo_trig_attribute_remove ON "array_attribute";
DROP TRIGGER IF EXISTS scidb4geo_trig_namespace_rename ON "namespaces";
DROP TRIGGER IF EXISTS scidb4geo_trig_namespace_remove ON "namespaces";

DROP FUNCTION IF EXISTS scidb4geo_proc_array_rename();
DROP FUNCTION IF EXISTS scidb4geo_proc_array_remove();
DROP FUNCTION IF EXISTS scidb4geo_proc_dimension_rename();
DROP FUNCTION IF EXISTS scidb4geo_proc_dimension_remove();
DROP FUNCTION IF EXISTS scidb4geo_proc_attribute_rename();
DROP FUNCTION IF EXISTS scidb4geo_proc_attribute_remove();
DROP FUNCTION IF EXISTS scidb4geo_proc_namespace_remove();
DROP FUNCTION IF EXISTS scidb4geo_proc_namespace_rename();

DROP TABLE IF EXISTS scidb4geo_array_v;
DROP TABLE IF EXISTS scidb4geo_array_t;
DROP TABLE IF EXISTS scidb4geo_array_s;
DROP TABLE IF EXISTS scidb4geo_array_md;
DROP TABLE IF EXISTS scidb4geo_attribute_md;

DROP INDEX IF EXISTS scidb4geo_idx_spatialrefsys_auth;
DROP TABLE IF EXISTS scidb4geo_spatialrefsys;
