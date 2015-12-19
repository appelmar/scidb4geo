##eo_arrays()

Lists geographically referenced arrays.

###Synopsis
```
AFL% eo_arrays();
```



###Result
Returns a one-dimensional table-like two attribute array with rows for individual referenced arrays where the first attribute gives the name of an array.
The second attributes represents the setting of an array as a string, i.e. whether it has spatial ('s'), temporal ('t'), or vertical ('v') references.


###Examples


1. Create a spatial and a spatiotemporal array
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0],double(random()) / double(2147483647)),world_s);  
eo_setsrs(world_s,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');

store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0,t=0:364,32,0],double(random()) / double(2147483647)),world_st); 
eo_cpsrs(world_s,world_st);
eo_settrs(world_st,'t','2001-01-01', 'P1D');
```

2. List all earth observation arrays
```
eo_arrays();
```

3. Listing only spatiotemporally referenced arrays
```
filter(eo_arrays(),setting='st');
```

4. Clean up
```
remove(world_s);
remove(world_st);
```

###Notes
Vertical referencing is currently not implemented.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>