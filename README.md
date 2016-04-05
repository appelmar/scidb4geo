# scidb4geo
A SciDB plugin for managing spacetime earth-observation arrays.

## Description
scidb4geo is a plugin for managing spatial and temporal reference information of SciDB arrays. It defines a couple of additional operators and adds actual reference information to SciDB's system catalog. It can be used within AFL or AQL. 



## Getting Started

The following sequence of SciDB queries demonstrate how the plugin can be used to store spatiotemporal reference of arrays. These are the most basic operations though there are more complex 
operators that will be demonstrated in a case study in the near future.

1. **Create a random spatiotemporal array covering the whole world**

  ```
  set no fetch; # do not print results of store
  store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0,t=0:364,32,0],double(random(),world); 
  eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
  eo_settrs(world,'t','2001-01-01', 'P1D');
  set fetch; # we do want to see output of the following queries.
  ```
  
2. **Get spatial / temporal reference information**
  ```
  eo_getsrs(world); 
  eo_gettrs(world);
  eo_extent(world);
  
  rename(world,earth); # renaming does not change spatial reference
  
  eo_getsrs(earth); 
  eo_gettrs(earth);
  eo_extent(earth);
  ```
  
3. **Clean up**
  ```
  remove(earth);
  ```




## Operators
The following operators are currently implemented:

- `eo_arrays()` - returns a one-dimensional array containing all referenced arrays and their setting, i.e. whether its reference is spatial, temporal, vertical, or any combination.
- `eo_getsrs(named_array)` - returns the spatial reference (as a single one-dimensional array tuple) of the given array
- `eo_extent(named_array)` - returns the bounding box in SRS coordinates of a given array 
- `eo_regnewsrs(auth_name, auth_srid, wktext, proj4text)` - registers a custom spatial reference system 
- `eo_cpsrs(named_array, named_array)` - copies spatial reference information from one array to another
- `eo_setsrs(named_array, xdim, ydim, auth_name, auth_srid, affine_transform)` - sets the spatial reference system of a given array, e.g. `AFL% eo_setsrs(a,'j','i','EPSG',4326,'x0=7.123 y0=52.023 a11=0.1 a22=0.1');`
- `eo_gettrs(named_array)` - returns the temporal reference (as a single one-dimensional array tuple) of the given array
- `eo_settrs(named_array, tdim, t0, dt)` - sets the temporal reference of a given array, e.g. `AFL% eo_settrs(a,'t','2015-01-01', 'P1D');`
- `eo_setmd(named_array, [attribute], keys, values)` - sets key value metadata pairs for an array (attribute), keys and values may be comma separated strings to add multiple pairs. 
- `eo_getmd(named_array)` - gets key value metadata pairs for an array 
- `eo_over(named_array, named_array)` - for all cells of the first array, eo_over computes coordinates of the second array that are spatially or temporally overlying



## Build and Installation
The following instructions refer to using SciDB version 15.7 on Ubuntu 14.04. You will need the SciDB development packages which you normally have automatically installed if you built SciDB from sources.
Furthermore, the plugin requires [libcurl](http://curl.haxx.se/).

1. Clone this repository `git clone https://github.com/mappl/scidb4geo`.
2. Compile by running `make` in the base directory. If SciDB libraries and headers are installed at a nonstandard location, you may use `make /path/to/scidb`.
3. Use the `install/setup.sh` script to extend SciDB's system catalog by running `cd install && chmod +x setup.sh && ./setup.sh` as the root user or with sudo rights.
4. Copy `libscidb4geo.so` to the plugins directory of SciDB on **all instances**, i.e. `cp libscidb4geo.so /opt/scidb/15.7/lib/scidb/plugins/` for a default local SciDB installation.
5. Now, you should be able to load the plugin using `AFL% load_library('scidb4geo');`


