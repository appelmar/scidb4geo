##eo_create_stv()

Creates a four-dimensional (2x horizontal **s**pace, 1x **v**ertical space, 1x **t**ime) geographically referenced SciDB array.

**THIS METHOD IS NOT YET AVAILABLE**

###Synopsis
```
AFL% eo_create_stv(name, attr, s_ref, s_dom, s_A, t_dom, dt, v_ref, v_dom, dv);
```

Argument   | Description 
--------   | ------------
`name`     | Unique identifying array name 
`attr`     | Attribute definition as a string (see SciDB operator `CREATE ARRAY`)
`s_ref`    | String definition of the spatial reference system e.g. "EPSG:4326"
`s_dom`    | Spatial domain or coverage of the array specified as cordinates of the lower left and upper right corner e.g. "xmin=7.123 xmax=15.01541 ymin=49.1 ymax=52.62"
`s_A`      | String representation of an affine transformation that maps image coordinates (i.e. integers starting with 0) to world coordinates in the given reference system
`t_dom`    | Tempral extent of the array as a string containing minimum and maximum dates, e.g. "tmin=2014-01-01 02:55:01.92 +01:00 tmax=2014-12-01 02:55:01.92 +01:00"
`dt`       | Time difference between subsequent pixels as a string according to ISO 8601, e.g. "1d" or "24h"
`v_ref`    | String definition of the **vertical** spatial reference system e.g. "EPSG:5215"
`v_dom`    | Spatial domain or coverage of the array specified as cordinates of the lower left and upper right corner e.g. "vmin=0 vmax=10000"
`v_A`      | String representation of an affine transformation that maps image coordinates (i.e. integers starting with 0) to world coordinates in the given reference system. **Usually, you need to specify only the vertical pixel size dv**, which means image coordinate zero coresponds to the lower boundary of the vertical domain as specified in v_dom. 


###Result
This is a data definition operator and won't create any results.

###Details




###Examples
1. TODO 
```
TODO 
```


###Notes
This function currently does not support custom chunk and overlap values. This might be added as an optional argument in the future.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>