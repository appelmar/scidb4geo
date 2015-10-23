# scidb4geo
A SciDB Plugin for Managing Spatial and Temporal Reference Information of Arrays

## Description
This is a preliminary version of a plugin for managing spatial and temporal reference information of SciDB arrays. It defines a couple of UDOs and adds actual reference information to the system catalog. It can be used within AFL or AQL. 

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


## Getting Started
Currently, installing the plugin requires building it from source along with SciDB (see below for details). We are working on a deployment without compiling SciDB.
Once built and installed, the `ex01.afl` file shows some very basic examples how to use the plugin within AFL.


## Build Instructions
The following instructions refer to using SciDB version 14.12 on Ubuntu 14.04.

<!--### The fast way: Linking with prebuilt SciDB dev packages 
1. Install prebuilt binary packages from Paradigm4 and some additional dependencies (see install_dependencies.sh)
2. Clone the scidb4geo source `git clone https://github.com/mappl/scidb4geo`
3. Run `cmake . && XXXXX`
4. The resulting install/libscidb4geo.so library can now be used to install it as a SciDB plugin (see Installation) XXXX -->

### Compiling with SciDB
We assume that you have a running SciDB build environment, i.e. you are able to compile SciDB from source (see SciDB_Build_Instructions_14_12.pdf document of Paradigm4).

1. If not yet done, set the environment variable `SCIDB_INSTALL_PATH` to your scidb installation path and to the SciDB source code root directory. Let `SCIDB_TRUNK` denote the directory of the scsidb source code.
2. Change directory to `cd $SCIDB_TRUNK/examples/`
3. Clone the scidb4geo source `git clone https://github.com/mappl/scidb4geo`
4. Add entries in `$SCIDB_TRUNK/examples/CMakeLists.txt` and `$SCIDB_TRUNK/install.cmake`
5. Setup, make, install, and run SciDB  (i.e. subsequently run `$SCIDB_TRUNK/run.py setup | make | install | run`)
7. Run the provided `$SCIDB_TRUNK/examples/scidb4geo/install/setup.sh` script to create neccessary tables in the system catalog
8. Now, you can load the plugin using `AFL% load_library('scidb4geo');`




