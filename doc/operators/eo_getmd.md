##eo_getmd()

Retrieve additional key value metadata of a given array.

###Synopsis
```
AFL% eo_getmd(array);
```

Argument      | Description 
--------      | ------------
`array`       | Existing array


###Result
This is a data definition operator and won't return any results.
A one-dimensional table-like array with rows for all key value metadata pairs and attributes
(arrayname,attribute,domain,key,value) where attribute is an empty string for general array (non-attribute) metadata. All attributes are strings.


###Details
This operator returns all key value metadata of an array. This includes both general array metadat like additional data generation information as well as attribute specific information like no data values, error code definitions, or units.
Domain strings can be used to organize metadata items in groups. This is in line with the GDAL metadata model.


###Examples
1. Create an array that covers the whole world with 0.1 degrees pixel size and one day temporal resolution starting at January 1st, 2015.
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
eo_settrs(world,'t','2015-01-01', 'P1D');
eo_setmd(world, 'AREA_OR_POINT,Description','AREA,A random array');
eo_setmd(world, 'val', 'ColorInterp,NODATA_VALUE', 'GCI_GrayIndex,-1');
eo_getmd(world);
remove(world);
```


###Notes
See the [GDAL metadata model](http://www.gdal.org/gdal_datamodel.html) for additional information.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>