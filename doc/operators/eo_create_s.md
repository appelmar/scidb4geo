##eo_create_s()

Creates a two-dimensional (2x **s**pace) geographically referenced SciDB array.

**THIS METHOD IS NOT YET AVAILABLE**

###Synopsis
```
AFL% eo_create_s(name, attr, s_ref, s_dom, s_A);
```

Argument   | Description 
--------   | ------------
`name`     | Unique identifying array name 
`attr`     | Attribute definition as a string (see SciDB operator `CREATE ARRAY`)
`s_ref`    | String definition of the spatial reference system e.g. "EPSG:4326"
`s_dom`    | Spatial domain or coverage of the array specified as cordinates of the lower left and upper right corner e.g. "xmin=7.123 xmax=15.01541 ymin=49.1 ymax=52.62"
`s_A`      | String representation of an affine transformation that maps image coordinates (i.e. integers starting with 0) to world coordinates in the given reference system



###Result
This is a data definition operator and won't create any results.

###Details




###Examples
1. Creating an array of MODIS reflectance values covering some parts of western Germany. The pixel of the result array will be 0.01 in both dimensions leading to 100x100 pixels in total.
```
eo_create_s('MOD09Q1', '<nir : uint16, red : uint16, quality : uint16>', 'EPSG:4326', 'xmin=7.0 xmax=7.99 ymin=51.0 ymax=51.99', 'x0=7.0 y0=51.0 a11=0.01 a22=-0.01')  
```

<!--
2. TODO
```
TODO
```
-->

###Notes
This function currently does not support custom chunk and overlap values. This might be added as an optional argument in the future.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>