##eo_cpsrs()

Copies the spatial reference of a source array to a target array.

**DEPRECATED** (use `eo_setsrs(target, source)` instead)

###Synopsis
```
AFL% eo_cpsrs(source, target);
```

Argument      | Description 
--------      | ------------
`source`      | Source array
`target`      | Target array

###Result
This is a data definition operator and won't return any results. 


###Details
This operator sets the spatial reference of the target array to the same system as the source array. Dimension names of spatial axes must match.
This is mostly useful to call after the source array has been processed and results are stored as a new array.


###Examples
1. Create a spatially referenced array, perform some analysis, and copy the reference to the result array.
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
store(apply(filter(world,val > 0.5), newval, 1), world_new);
eo_cpsrs(world,world_new);
remove(world);
remove(world_new);
```


###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>