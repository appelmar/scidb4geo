##eo_gettrs()

Gets the temporal reference of existing arrays.

###Synopsis
```
AFL% eo_gettrs([...]);
```

Argument   | Description 
--------   | ------------
`...`      | Optional list of arrays


###Result
A one-dimensional table-like array with rows for all given arrays and attributes
(name,tdim,t0,dt,tMin,tMax). If no array is given as argument, the operator considers all temporally referenced arrays.
Result attributes are described below.

Attribute   | Description 
--------   | ------------
`name`      | Array name
`tdim`      | Name of the array dimension representing time
`t0`      | Datetime at the origin of the temporal dimension as ISO8601 string 
`dt`      | Temporal distance between subsequent cells as ISO8601 date / time period string
`tMin`      | Minimum datetime covered by an array
`tMax`      | Maximum datetime covered by an array





###Examples

1. Create a spatiotemporal array
```
set no fetch; # do not print results of store
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0,t=0:364,32,0],double(random()) / double(2147483647)),world); 
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
eo_settrs(world,'t','2001-01-01', 'P1D');
set fetch;
```

2. Get temporal reference information
```
eo_gettrs(world);
```

3. Clean up
```
remove(world);
```



###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>