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
1. Listing all earth observation arrays
```
eo_arrays();
```

2. Listing only spatiotemporally referenced arrays
```
filter(eo_arrays(),setting='st');
```

###Notes
Vertical referencing is currently not implemented.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>