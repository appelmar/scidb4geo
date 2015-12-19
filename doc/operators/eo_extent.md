##eo_extent()

Computes the geographical extent of a given array.


###Synopsis
```
AFL% eo_extent(...);
```

Argument   | Description 
--------   | ------------
`...`      | Optional list of arrays

###Result
A one-dimensional table-like array with rows for all given arrays and attributes
(arrayname,setting,xmin,xmax,ymin,ymax,tmin,tmax,vmin,vmax) representing the geographical extent.


### Details
If no array is given as parameters, eo_extent considers all existing earth observation arrays (as in `eo_arrays()`).



###Examples

1. Create a spatial and a spatiotemporal array
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world_s);  
eo_setsrs(world_s,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');

store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0,t=0:364,32,0],double(random()) / double(2147483647)),world_st); 
eo_cpsrs(world_s,world_st);
eo_settrs(world_st,'t','2001-01-01', 'P1D');
```

2. Extent for all arrays 
```
eo_extent();
```

3. Extent one specific arrays
```
eo_extent(world_st);
```

4. Clean up
remove(world_s);
remove(world_st);

###Notes
Vertical referencing is currently not implemented. 

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>