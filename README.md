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
The table below lists novel AFL operators. Detailed descriptions of arguments, return values, and examples can be found in the `gh-pages` branch at [http://appelmar.github.io/scidb4geo/operators](http://appelmar.github.io/scidb4geo/operators).

| **Operator** | **Description** |
| -----------  | --------------------------------------------------------  |
| `eo_arrays()`    | Lists geographically referenced arrays |
| `eo_setsrs()`    | Sets the spatial reference of existing arrays |
| `eo_getsrs()`    | Gets the spatial reference of existing arrays |
| `eo_regnewsrs()` | Registers custom spatial reference systems  |
| `eo_extent()`    | Computes the geographic extent of referenced arrays |
| `eo_settrs()`    | Sets the temporal reference of arrays |
| `eo_gettrs()`    | Gets the temporal reference of arrays |
| `eo_setmd()`     | Sets key value metadata of arrays and array attributes |
| `eo_getmd()`     | Gets key value metadata of arrays and array attributes |
| `eo_over()`      | Overlays two geographically referenced arrays  |
| `eo_coords()`    | Converts array cell indexes to coordinates as additional array attributes  |
| `eo_version()`   | Prints the plugin version and build dates  |


## Build and Installation
The following instructions refer to using SciDB version 15.7 on Ubuntu 14.04. You will need the SciDB development packages which you normally have automatically installed if you built SciDB from sources.
Furthermore, the plugin requires [libcurl](http://curl.haxx.se/).

1. Clone this repository `git clone https://github.com/appelmar/scidb4geo`.
2. Compile by running `make` in the base directory. If SciDB libraries and headers are installed at a nonstandard location, you may use `make /path/to/scidb`.
3. Use the `install/setup.sh` script to extend SciDB's system catalog by running `cd install && chmod +x setup.sh && ./setup.sh` as the root user or with sudo rights.
4. Copy `libscidb4geo.so` to the plugins directory of SciDB on **all instances**, i.e. `cp libscidb4geo.so /opt/scidb/15.7/lib/scidb/plugins/` for a default local SciDB installation.
5. Now, you should be able to load the plugin using `AFL% load_library('scidb4geo');`


