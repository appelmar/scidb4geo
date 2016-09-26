##eo_setsrs()

Sets the spatial reference of an existing array.

###Synopsis
```
AFL% eo_setsrs(array_target, xdim, ydim, auth_name, auth_srid, A  );
AFL% eo_setsrs(array_target, array_source );
```

Argument        | Description 
--------        | ------------
`array_target`  | Existing array whose spatial reference will be set
`array_source`  | Existing array from which the spatial reference will be copied to the target array
`xdim`          | Name of the longitude / easting dimension as a string
`ydim`          | Name of the latitude / northing dimension as a string
`auth_name`     | String identifier of SRS authority, e.g. 'EPSG'
`auth_srid`     | Integer ID of the reference system according to an authority, e.g. 4326
`A`             | Affine transformation parameters as a string (see details below), e.g. 'x0=-180 y0=90 a11=0.1 a22=-0.1'

###Result
This is a data definition operator and won't return any results.

###Details
The affine transformation A is used to relate image coordinates to referenced coordinates. Its string definition takes a space separated list of parameters x0,y0,a11,a12,a21,a22 
with default values representing the identity transformation (0,0,1,0,0,1). Notice that the affine transformation is in line with the [GDAL data model](http://www.gdal.org/gdal_datamodel.html).



###Examples
1. Create an array that covers the whole world with 0.1 degrees pixel size
```
store(build(<val:double>[lat=0:1799,256,0,lon=0:3599,256,0],1),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=0.1 a22=-0.1');
```


###Notes
The plugin comes with standard EPSG definitions. If the combination of authority name and id cannot be found internally, the plugin checks [spatialreference.org](http://spatialreference.org) and automatically adds the system to the catalog if available.
Otherwise, you may add proj4 and wkt definitions manually using `eo_regnewsrs()`.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>