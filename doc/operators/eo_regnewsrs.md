##eo_regnewrs()

Adds a custom spatial reference system to the catalog.

###Synopsis
```
AFL% eo_regnewsrs(auth_name, auth_srid, wkt, proj4);
```

Argument         | Description 
--------         | ------------
`auth_name`      | Authority name as a string
`auth_srid`      | Integer identifier
`wkt`            | Well-known text definition as a string
`proj4`          | Proj4 definition as a string

###Result
This is a data definition operator and won't return any results. 


###Details
This operator adds a new spatial reference system and its WKT and proj4 definitions to the catalog.

###Examples
1. Add the MODIS sinusoidal systems (as on [http://spatialreference.org/ref/sr-org/6842/](http://spatialreference.org/ref/sr-org/6842/))
```
eo_regnewsrs('SR-ORG',6842, 'PROJCS["Sinusoidal",GEOGCS["GCS_Undefined",DATUM["Undefined",SPHEROID["User_Defined_Spheroid",6371007.181,0.0]],PRIMEM["Greenwich",0.0],UNIT["Degree",0.0174532925199433]],PROJECTION["Sinusoidal"],PARAMETER["False_Easting",0.0],PARAMETER["False_Northing",0.0],PARAMETER["Central_Meridian",0.0],UNIT["Meter",1.0]]','+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371007.181 +b=6371007.181 +units=m +no_defs');
```

###Notes
The combination of authority name and ID must be unique and must not exist before.

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>