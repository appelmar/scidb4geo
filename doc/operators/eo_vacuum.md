##eo_vacuum()

Removes dead geographic reference entries from the system catalog and cleans up a few other things.

**THIS METHOD IS NOT YET AVAILABLE**

###Synopsis
```
AFL% eo_vacuum();
```



###Result
This is a maintanance operator and won't create any results.

###Details
From time to time, geographic arrays might be removed, spatial / temporal / vertical dimensions might be renamed or other actions can lead to an inconsistent state regarding georeference metadata. Although this is usually detected and resolved automatically, it might be sometimes needed to clean the database manually using this operator. This is in particular true for older versions of SciDB using outdated Postgres installations without support for some triggers.


###Examples

```
eo_vacuum()
```


###Notes

**THIS METHOD IS NOT YET AVAILABLE**

###Author(s)
Marius Appel - <marius.appel@uni-muenster.de>