##eo_getsrs()

Gets the spatial reference of existing arrays.

###Synopsis
```
AFL% eo_getsrs([...]);
```

Argument   | Description 
--------   | ------------
`...`      | Optional list of arrays


###Result
A one-dimensional table-like array with rows for all given arrays and attributes
(name, xdim, ydim, auth_name, auth_srid, srtext, proj4text). If no array is given as argument, the operator considers all spatially referenced arrays.
Result attributes are described below.

Attribute   | Description 
--------   | ------------
`name`      | Array name
`xdim`      | Name of the array dimension representing easting or longitude
`ydim`      | Name of the array dimension representing northing or latitude
`auth_name`      | Authority name of the spatial reference system
`auth_srid`      | Unique identifier of the spatial reference system
`srtext`      | WKT string of the reference system
`proj4text`      | Proj4 string of the reference system
`A`      | String definintion for an affine transformation that maps array coordinates to world coordinates

###Details
The affine transformation A is used to relate image coordinates to referenced coordinates. Its string definition takes a space separated list of parameters x0,y0,a11,a12,a21,a22 
with default values representing the identity transformation (0,0,1,0,0,1). Notice that the affine transformation is in line with the [GDAL data model](http://www.gdal.org/gdal_datamodel.html).



###Examples
1. Create an array that covers the whole world with 1 degree pixel size and one day temporal resolution starting at January 1st, 2015.
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
```

2. Get spatial reference information
```
eo_getsrs(world);
```

3. Get partial reference information
```
project(eo_getsrs(world),arrayname, auth_name, auth_id) ;
```

4. Clean up
```
remove(world);
```

###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>