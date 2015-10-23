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
1. Insert tiles of the SRTM elevation model where srtm is the previously created result array with enough space to cover specific tiles: 
```
eo_insert(srtm1504, srtm);
eo_insert(srtm1505, srtm);
eo_insert(srtm1506, srtm);
```




###Notes

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>