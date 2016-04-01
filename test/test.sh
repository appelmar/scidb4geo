#!/bin/bash
TESTARRAY=scidb4geo_test_$( date +%Y%m%d_%H%M%S )
iquery -anq "remove(${TESTARRAY});" > /dev/null 2>&1
iquery -anq "store(build(<val:double>[lat=0:179,256,0,lon=0:359,256,0, t=0:31,16,0],1),${TESTARRAY});" > /dev/null 2>&1

rm test.out > /dev/null 2>&1
rm test.expected > /dev/null 2>&1

echo "Running tests for scidb4geo:"

# 1. eo_setsrs
iquery -o csv:l -aq "eo_setsrs(${TESTARRAY},'lon','lat','EPSG',4326,'x0=-180 y0=90 a11=1 a22=-1');" > test.out
echo "Query was executed successfully" > test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_setsrs()... successful"; else echo "eo_setsrs()... failed"; fi

# 2. eo_setrs
iquery -o csv:l -aq "eo_settrs(${TESTARRAY},'t','2016-01-01', 'P1D');" > test.out
echo "Query was executed successfully" > test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_settrs()... successful"; else echo "eo_settrs()... failed"; fi

# 3. eo_getsrs
iquery -o csv:l -aq "set no timer;eo_getsrs(${TESTARRAY});" > test.out
echo "name,xdim,ydim,auth_name,auth_srid,srtext,proj4text,A" > test.expected
echo "'${TESTARRAY}','lon','lat','EPSG',4326,'GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]','+proj=longlat +datum=WGS84 +no_defs ','x0=-180 y0=90 a11=1 a22=-1 a12=0 a21=0'" >> test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_getsrs()... successful"; else echo "eo_getsrs()... failed"; fi

# 4. eo_gettrs
iquery -o csv:l -aq "set no timer;eo_gettrs(${TESTARRAY});" > test.out
echo "name,tdim,t0,dt,tMin,tMax" > test.expected
echo "'${TESTARRAY}','t','2016-01-01T00:00:00','P1D','2016-01-01T00:00:00','2016-02-01T00:00:00'" >> test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_gettrs()... successful"; else echo "eo_gettrs()... failed"; fi

# 5. eo_extent
iquery -o csv:l -aq "set no timer;eo_extent(${TESTARRAY});" > test.out
echo "arrayname,setting,xmin,xmax,ymin,ymax,tmin,tmax,vmin,vmax" > test.expected
echo "'${TESTARRAY}','st',-180,179,-89,90,'2016-01-01T00:00:00','2016-02-01T00:00:00',null,null" >> test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_extent()... successful"; else echo "eo_extent()... failed"; fi

# 6. eo_arrays
iquery -o csv:l -aq "set no timer;filter(eo_arrays(),name='${TESTARRAY}');" > test.out
echo "name,setting" > test.expected
echo "'${TESTARRAY}','st'" >> test.expected
if diff test.out test.expected > /dev/null 2>&1; then echo "eo_arrays()... successful"; else echo "eo_arrays()... failed"; fi

rm test.out > /dev/null 2>&1
rm test.expected > /dev/null 2>&1

echo "DONE."