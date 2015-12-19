##eo_setmd()

Sets additional key value metadata for arrays and array attributes

###Synopsis
```
AFL% eo_setmd(array, [attribute], keys, values);
```

Argument      | Description 
--------      | ------------
`array`       | Existing array
`attribute`   | Optional attribute name of provided array as string
`keys`        | Comma-separated list of metadata keys
`values`      | Comma-separated list of metadata values

###Result
This is a data definition operator and won't return any results.


###Details
This operator can be used to set additional key value array and attribute metadata depending on whether three or four arguments are given. 
Length of `keys` and `values` arguments must match which sometimes leads to problems if data have metadata strings with commas.  


###Examples
1. Create an array that covers the whole world with 1 degrees pixel size and one day temporal resolution starting at January 1st, 2015.
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
eo_settrs(world,'t','2015-01-01', 'P1D');

eo_setmd(world,'AREA_OR_POINT','AREA');
eo_setmd(world,'val','NODATA,SCALE', '-1,10000');
eo_getmd(world);
remove(world);
```


###Notes
See the [GDAL metadata model](http://www.gdal.org/gdal_datamodel.html) for additional information.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>