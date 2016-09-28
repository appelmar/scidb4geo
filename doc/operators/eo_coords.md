##eo_coords()

Derives referenced coordinates of array cells as additional attributes.

###Synopsis
```
AFL% eo_coords(A);
```



###Result
The result has the same dimensionality as A but additional attributes `eo_x`, `eo_y`, and `eo_t`, with referenced coordinates for all array cells. The cell's integer index and the affine transformation is used to convert array coordinates to referenced coordinates. 



###Examples


1. Create a spatiotemporal array, compute cell coordinates, and derive the physical extent
```
store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0,t=0:364,32,0],double(random()) / double(2147483647)),A); 
eo_setsrs(A,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');
eo_settrs(A,'t','2001-01-01', 'P1D');
aggregate(eo_coords(A), min(eo_t), max(eo_t), min(eo_x), max(eo_x), min(eo_y), max(eo_y));
```

2. Clean up
```
remove(A);
```

###Notes
The input array must be persistent and referenced, i.e. using nested AFL queries as input is not supported due to missing reference.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>