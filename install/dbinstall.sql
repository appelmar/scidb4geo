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

-- Instalation script for creating schemas, functions, and trigger for storing spatial reference metadata in SciDB's system catalog

-- This table is adopted from PostGIS
create table scidb4geo_spatialrefsys (
	srid serial not null,
	auth_name varchar(256) ,
	auth_srid integer not null,
	srtext varchar(2048) ,
	proj4text varchar(2048) ,
	primary key (srid), unique(auth_name,auth_srid));


create index scidb4geo_idx_spatialrefsys_auth ON scidb4geo_spatialrefsys (auth_name, auth_srid);
	
-- each row in the following table represents spatial reference for one array including affine transformation parameters to convert array coordinates to world coordinates
create table scidb4geo_array_s (
	arrayname varchar(64) not null,
	xdim varchar(64) not null,
	ydim varchar(64) not null,
	srid integer not null,
	x0 numeric not null default 0.0,
	y0 numeric not null default 0.0,
	a11 numeric not null default 1.0,
	a12 numeric not null default 0.0,
	a21 numeric not null default 0.0,
	a22 numeric not null default 1.0,
	primary key (arrayname),
	foreign key (srid) references scidb4geo_spatialrefsys(srid) on delete set null on update cascade);


create table scidb4geo_array_t (
	arrayname varchar(64) not null,
	tdim varchar(64) not null,
	t0 varchar(64)  not null,
	dt varchar(64) not null,
	primary key (arrayname));
	
	
create table scidb4geo_array_v (
	arrayname varchar(64) not null,
	vdim varchar(64) not null,
	srid integer not null,
	v0 numeric not null default 0.0,
	dv numeric not null default 1.0,
	primary key (arrayname),
	foreign key (srid) references scidb4geo_spatialrefsys(srid) on delete set null on update cascade);
	

create table scidb4geo_array_md (
        arrayname varchar(64) ,
        domainname varchar(64) default '', 
	kv hstore,
	primary key (arrayname));	
	
	
create table scidb4geo_attribute_md (
	arrayid integer not null,
	attrname varchar(64) not null,
	domainname varchar(64) default '', 
	kv hstore,
	primary key (arrayid,attrname));
	
	

-- Trigger array rename operations	
create or replace function scidb4geo_proc_array_rename() returns trigger as
$BODY$
  begin
    update scidb4geo_array_s      set arrayname = NEW.name where arrayname = OLD.name;
    update scidb4geo_array_v      set arrayname = NEW.name where arrayname = OLD.name;
    update scidb4geo_array_t      set arrayname = NEW.name where arrayname = OLD.name;
    update scidb4geo_array_md     set arrayname = NEW.name where arrayname = OLD.name;
  return NULL;
  end;
$BODY$ LANGUAGE plpgsql;
	

create trigger scidb4geo_trig_array_rename
  after update on "array"
  for each row
  when (OLD.name is distinct from NEW.name)
  execute procedure scidb4geo_proc_array_rename();
  
  
  
	
-- Trigger array remove operations	
create or replace function scidb4geo_proc_array_remove() returns trigger as
$BODY$
  begin
  delete from scidb4geo_array_s       where arrayname = OLD.name;
  delete from scidb4geo_array_v       where arrayname = OLD.name;
  delete from scidb4geo_array_t       where arrayname = OLD.name;
  delete from scidb4geo_array_md      where arrayname = OLD.name;
  delete from scidb4geo_attribute_md  where arrayid   = OLD.id;
  return NULL;
  end;
$BODY$ LANGUAGE plpgsql;
	

create trigger scidb4geo_trig_array_remove
  after delete on "array"
  for each row
  execute procedure scidb4geo_proc_array_remove();
  
  
 
  
-- Trigger array dimension rename operations	  
create or replace function scidb4geo_proc_dimension_rename() returns trigger as
$BODY$
  begin
    update scidb4geo_array_s set xdim = NEW.name where xdim = OLD.name and arrayname = (select name from "array" where id = OLD.array_id);
    update scidb4geo_array_s set ydim = NEW.name where ydim = OLD.name and arrayname = (select name from "array" where id = OLD.array_id);
    update scidb4geo_array_v set vdim = NEW.name where vdim = OLD.name and arrayname = (select name from "array" where id = OLD.array_id);
    update scidb4geo_array_t set tdim = NEW.name where tdim = OLD.name and arrayname = (select name from "array" where id = OLD.array_id);
  return NULL;
  end;
$BODY$ LANGUAGE plpgsql;
	

create trigger scidb4geo_trig_dimension_rename
  after update on "array_dimension"
  for each row
  when (OLD.name is distinct from NEW.name)
  execute procedure scidb4geo_proc_dimension_rename();
  
  
 

-- Trigger array dimension remove operations	  
create or replace function scidb4geo_proc_dimension_remove() returns trigger as
$BODY$
  begin
    -- delete srs if either xdim or ydim was removed from array for whatever reason
    delete from scidb4geo_array_s where (xdim = OLD.name or ydim = OLD.name) and arrayname = (select name from "array" where id = OLD.array_id);  
	delete from scidb4geo_array_v where (vdim = OLD.name) and arrayname = (select name from "array" where id = OLD.array_id);  
	delete from scidb4geo_array_t where (tdim = OLD.name) and arrayname = (select name from "array" where id = OLD.array_id);  
  return NULL;
  end;
$BODY$ LANGUAGE plpgsql;
	

create trigger scidb4geo_trig_dimension_remove
  after delete on "array_dimension"
  for each row
  execute procedure scidb4geo_proc_dimension_remove();
    
 
 
 
-- Trigger attribute rename operations	
create or replace function scidb4geo_proc_attribute_rename() returns trigger as
$BODY$
  begin
    update scidb4geo_attribute_md set attrname = NEW.name where attrname = OLD.name and arrayid = OLD.array_id;
  return NULL;
  end;
$BODY$ LANGUAGE plpgsql;	

create trigger scidb4geo_trig_attribute_rename
  after update on "array_attribute"
  for each row
  when (OLD.name is distinct from NEW.name)
  execute procedure scidb4geo_proc_attribute_rename();
  
	


-- Trigger array attribute remove operations	  
create or replace function scidb4geo_proc_attribute_remove() returns trigger as
$BODY$
  begin
    delete from scidb4geo_attribute_md where (attrname = OLD.name and arrayid = OLD.array_id);
    return NULL;
  end;
$BODY$ LANGUAGE plpgsql;
	

create trigger scidb4geo_trig_attribute_remove
  after delete on "array_attribute"
  for each row
  execute procedure scidb4geo_proc_attribute_remove();
  
  

 
  