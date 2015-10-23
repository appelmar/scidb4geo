##eo_create_st()

Creates a three-dimensional (2x **s**pace, 1x **t**ime) geographically referenced SciDB array.

**THIS METHOD IS NOT YET AVAILABLE**

###Synopsis
```
AFL% eo_create_st(name, attr, s_ref, s_dom, s_A, t_dom, dt);
```

Argument   | Description 
--------   | ------------
`name`     | Unique identifying array name 
`attr`     | Attribute definition as a string (see SciDB operator `CREATE ARRAY`)
`s_ref`    | String definition of the spatial reference system e.g. "EPSG:4326"
`s_dom`    | Spatial domain or coverage of the array specified as cordinates of the lower left and upper right corner e.g. "xmin=7.123 xmax=15.01541 ymin=49.1 ymax=52.62"
`s_A`      | String representation of an affine transformation that maps image coordinates (i.e. integers starting with 0) to world coordinates in the given reference system
`t_dom`    | Tempral extent of the array as a string containing minimum and maximum dates, e.g. "tmin=2014-01-01 02:55:01.92 +01:00 tmax=2014-12-01 02:55:01.92 +01:00"
`dt`       | Time difference between subsequent pixels as a string according to ISO 8601, e.g. "1d"



###Result
This is a data definition operator and won't create any results.

###Details




###Examples
1. Creating an array of hourly precipitation values covering some parts of western Germany for January 2014. 
```
eo_create_s('Precipitation', '<mm : double>', 'EPSG:4326', 'xmin=7.0 xmax=7.99 ymin=51.0 ymax=51.99', 'x0=7.0 y0=51.0 a11=0.01 a22=-0.01', 'tmin=2014-01-01 tmax=2014-01-30', '1h'))  
```


###Notes
This function currently does not support custom chunk and overlap values. This might be added as an optional argument in the future.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>