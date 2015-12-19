##eo_all()

Collects complete metadata of arrays

###Synopsis
```
AFL% eo_all(...);
```

Argument   | Description 
--------   | ------------
`...`      | Optional list of arrays


###Result
A one-dimensional table-like array with rows for all given arrays and attributes
(name, dimensions, attributes, srs, trs, extent). These are string representations of `dimensions(name)`, `attributes(name)`, `eo_getsrs(name)`, `eo_gettrs(name)`, and `eo_extent(name)` respectively. As such, dimensions is the output of `dimensions(name)` where single dimensions are enclosed in square brackets and
its properties like chunk sizes or boundaries are separated by three semicolons. Similarly, attributes contains the result of `attributes(name)` but uses angle brackets.

###Details
This operator is used to fetch metadata of arrays (dimensions, attributes, geographic reference) with only one AFL query. 
If no input array is provided, metadata of all spatially or temporally referenced arrays will be returned. Otherwise only provided arrays are considered.


###Examples
1. Create a spatial array and return complete metadata at once.
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
eo_all(world); 
remove(world);
```


###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>