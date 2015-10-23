##eo_insert()

Inserts a source array to an equal or higher dimensional target array based on its geographic coverages.

**THIS METHOD IS NOT YET AVAILABLE**

###Synopsis
```
AFL% eo_insert(source, target, rm);
```

Argument   | Description 
--------   | ------------
`source`   | Source array that  
`target`   | Target array
`rm`       | Optional boolean argument, if true, source array will be removed after sucessful insertion. Default value is false.


###Result
This is a data definition operator and won't create any results.

###Details
This operator is primarily used to add tile-based remote sensing data to existing arrays. Spatial reference systems of both arrays must be equal. 
Positioning the source array into the target array is done automatically, existing values will be overridden.

The source array must have less or equal dimensions than the target array. Typical use cases include:

- Inserting spatial tiles to a two-dimensional larger array, e.g. a single MODIS tile to an earth-scale spatial array
- Inserting temporal data to a three-dimensional spatiotemporal array (image stack) slice-wise 
- A combination of the above


More complex scenarios with vertical space include:

- Sequentially inserting vertical layers 
- Adding three-dimensional spatial xyz cubes to four dimensional xyzt arrays by time


###Examples
1. TODO 
```
TODO 
```


###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>