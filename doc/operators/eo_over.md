##eo_over()

Overlays two geographically referenced arrays and returns array coordinates of the target array which overlay specific cells of the source array.


###Synopsis
```
AFL% eo_over(A, B);
```

Argument   | Description 
--------   | ------------
`A`        | Source array
`B`        | Target array


###Result
The result array has the same dimensionality and the same number of cells as the source array A.
Attribute values of result cells are coordinates of B at which A and B mutually overlay. Depending on the setting of A and B,
The output always contains 4 attributes over_x, over_y, over_t, and over_v. 
Depending on the setting of A and B, i.e. whether the arrays  have spatial, temporal, spatiotemporal, 
or even vertical geographic reference, particular attributes values might be null for all cells.


###Details
This operator can be used to insert earth-observation data by tiles, and spatiotemporally joining datasets for statistical analyses.



###Examples
1. Insert two temporal slices into a three dimensional array.
```
store(build(<val:double>[lat=0:1799,256,0,lon=0:3599,256,0, t=0:364,16,0],1),world_jan1);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=0.1 a22=-0.1');
eo_settrs(world,'t','2015-01-01', 'P1D');

store(build(<val:double>[lat2=0:1799,256,0,lon2=0:3599,256,0, t2=0:364,16,0],2),world_jan2);  
eo_setsrs(world,'lon2','lat2','EPSG',4326,'x0=-180 y0=90 a11=0.1 a22=-0.1');
eo_settrs(world,'t2','2015-01-02', 'P1D');

insert(redimension(attribute_rename(join(world_jan2, eo_over(world_jan2,world_jan1)), over_x, x, over_y, y, over_t, t), world_jan1), world_jan1);
```




###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>