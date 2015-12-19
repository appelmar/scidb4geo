##eo_settrs()

Sets the temporal reference of an existing array.

###Synopsis
```
AFL% eo_settrs(array, tdim, t0, dt  );
```

Argument      | Description 
--------      | ------------
`array`       | Existing array
`tdim`        | Name of the temporal dimension as a string
`t0`          | Datetime of the cell at t=0, i.e. the starting date given as a string according to ISO8601 e.g. '2015-02-05T12:04:11'
`dt`          | Datetime interval of temprally neighbouring cells as a string according to ISO8601 for period, e.g. 'P16D or 'PT3H'

###Result
This is a data definition operator and won't return any results.




###Examples
1. Create an array that covers the whole world with 1 degree pixel size and one day temporal resolution starting at January 1st, 2015.
```
store(build(<val:double>[lat=0:1799,256,0,lon=0:3599,256,0, t=0:364,16,0],1),world);  
eo_setsrs(world,'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=0.1 a22=-0.1');
eo_settrs(world,'t','2015-01-01', 'P1D');
```


###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>