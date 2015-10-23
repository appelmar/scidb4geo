##eo_extent()

Outputs the geographical extent of an array.


###Synopsis
```
AFL% eo_extent(...);
```

Argument   | Description 
--------   | ------------
`...`      | Optional list of arrays

###Result
A one-dimensional table-like array with rows for all given arrays and the following attributes:
(arrayname,setting,xmin,xmax,ymin,ymax,tmin,tmax,vmin,vmax) representing the geographical extent.


### Details
If no array is given as parameters, eo_extent considers all existing earth observation arrays (as in `eo_arrays()`).



###Examples
1. Extent for all arrays 
```
eo_extent();
```

2. Extent for two specific arrays
```
eo_extent(modis,trmm);
```

###Notes
Vertical referencing is currently not implemented. 

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>