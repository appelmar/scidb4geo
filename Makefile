ifeq ($(SCIDB),) 
  X := $(shell which scidb 2>/dev/null)
  ifneq ($(X),)
    X := $(shell dirname ${X})
    SCIDB := $(shell dirname ${X})
  endif
endif

# Find SciDB at default locations
ifeq ($(SCIDB),) 
  ifneq ($(wildcard /opt/scidb/15.12/.*),)
  SCIDB := /opt/scidb/15.12
  else 
  ifneq ($(wildcard /opt/scidb/15.7/.*),)
  SCIDB := /opt/scidb/15.7
  else 
  ifneq ($(wildcard /opt/scidb/14.12/.*),)
  SCIDB := /opt/scidb/14.12 
  endif 
  endif
  endif
endif

# A way to set the 3rdparty prefix path that is convenient
# for SciDB developers.
ifeq ($(SCIDB_VER),)
  SCIDB_3RDPARTY = $(SCIDB)
else
  SCIDB_3RDPARTY = /opt/scidb/$(SCIDB_VER)
endif

# A better way to set the 3rdparty prefix path that does
# not assume an absolute path. You can still use the above
# method if you prefer.
ifeq ($(SCIDB_THIRDPARTY_PREFIX),)
  SCIDB_THIRDPARTY_PREFIX := $(SCIDB_3RDPARTY)
endif


CFLAGS=-W -Wextra -Wall -Wno-unused-parameter -Wno-variadic-macros -Wno-strict-aliasing -Wno-long-long -Wno-unused -fPIC -D_STDC_FORMAT_MACROS -Wno-system-headers -isystem -O3 -g -DNDEBUG -D_STDC_LIMIT_MACROS
INC=-I. -I./src/extern -DPROJECT_ROOT="\"$(SCIDB)\"" -I"$(SCIDB_THIRDPARTY_PREFIX)/3rdparty/boost/include/" -I"$(SCIDB)/include"
LIBS=-shared -Wl,-soname,libscidb4geo.so -L. -L"$(SCIDB_THIRDPARTY_PREFIX)/3rdparty/boost/lib" -L"$(SCIDB)/lib" -Wl,-rpath,$(SCIDB)/lib:$(RPATH) -lm -lcurl

SRCS=src/plugin.cxx
# Compiler settings for SciDB version >= 15.7
ifneq ("$(wildcard /usr/bin/g++-4.9)","")
  CC := "/usr/bin/gcc-4.9"a
  CXX := "/usr/bin/g++-4.9"
  CFLAGS+=-std=c++11 -DCPP11
else
  ifneq ("$(wildcard /opt/rh/devtoolset-3/root/usr/bin/gcc)","")
   CC := "/opt/rh/devtoolset-3/root/usr/bin/gcc"
   CXX := "/opt/rh/devtoolset-3/root/usr/bin/g++"
   CFLAGS+=-std=c++11 -DCPP11
  endif
endif

#SRCS+= src/eo_all/LogicalAll.cxx src/eo_all/PhysicalAll.cxx
SRCS+= src/eo_arrays/LogicalArrays.cxx src/eo_arrays/PhysicalArrays.cxx 
SRCS+= src/eo_setsrs/LogicalSetSRS.cxx src/eo_setsrs/PhysicalSetSRS.cxx
SRCS+= src/eo_getsrs/LogicalGetSRS.cxx src/eo_getsrs/PhysicalGetSRS.cxx
SRCS+= src/eo_settrs/LogicalSetTRS.cxx src/eo_settrs/PhysicalSetTRS.cxx
SRCS+= src/eo_gettrs/LogicalGetTRS.cxx src/eo_gettrs/PhysicalGetTRS.cxx
SRCS+= src/eo_regnewsrs/LogicalRegNewSRS.cxx src/eo_regnewsrs/PhysicalRegNewSRS.cxx
SRCS+= src/eo_extent/LogicalExtent.cxx src/eo_extent/PhysicalExtent.cxx
SRCS+= src/eo_over/OverArray.cxx src/eo_over/LogicalOver.cxx src/eo_over/PhysicalOver.cxx
SRCS+= src/eo_setmd/LogicalSetMD.cxx src/eo_setmd/PhysicalSetMD.cxx
SRCS+= src/eo_getmd/LogicalGetMD.cxx src/eo_getmd/PhysicalGetMD.cxx
#SRCS+= src/eo_fill/FillArray.cxx  src/eo_fill/LogicalFill.cxx  src/eo_fill/PhysicalFill.cxx 
#SRCS+= src/eo_pad/PadArray.cxx  src/eo_pad/LogicalPad.cxx  src/eo_pad/PhysicalPad.cxx 
#SRCS+= src/eo_cpsrs/LogicalCpSRS.cxx src/eo_cpsrs/PhysicalCpSRS.cxx 
SRCS+= src/eo_version/LogicalVersion.cxx src/eo_version/PhysicalVersion.cxx 
SRCS+= src/eo_coords/CoordsArray.cxx src/eo_coords/LogicalCoords.cxx src/eo_coords/PhysicalCoords.cxx 
#SRCS+= src/eo_extend/ExtendArray.cxx src/eo_extend/LogicalExtend.cxx src/eo_extend/PhysicalExtend.cxx 
SRCS+= src/PostgresWrapper.cxx src/AffineTransform.cxx src/TemporalReference.cxx src/ErrorCodes.cxx




OBJECTS:=$(SRCS:.cxx=.o)

.PHONY: test package clean install

all: $(OBJECTS)
	@if test ! -d "$(SCIDB)"; then echo  "Error. Try:\n\nmake SCIDB=<PATH TO SCIDB INSTALL PATH>"; exit 1; fi
	$(CXX) $(CFLAGS) $(INC) -o libscidb4geo.so $(OBJECTS) $(LIBS)
	@echo "BUILD successful."  
	@echo "To install the scidb4geo plugin, the following steps are still needed:"
	@echo "1. Run the install/setup.sh script as root user to update the system catalog"
	@echo "2. Copy libscidb4geo.so to your SciDB lib/scidb/plugins directory ON ALL NODES IN YOUR CLUSTER and run"
	@echo "iquery -aq \"load_library('scidb4geo')\" # to load the plugin afterwards."
	@echo "3. Re-start SciDB if the plugin was already loaded previously."
	

%.o: %.cxx
	$(CXX) $(CFLAGS) $(INC) -c -o $@ $<

package: all
	@echo "Building .tar.gz installation archive..."
	@if test ! -e "libscidb4geo.so"; then echo  "Error. Cannot find scidb4geo library, run make first."; exit 1; fi
	cp libscidb4geo.so install/
	tar "cfz" "scidb4geo_bin.tar.gz" install/
	@echo "DONE."
test:
	./test/test.sh
clean:
	rm -f *.so $(OBJECTS)
install:
	cd install && chmod +x setup.sh && yes | ./setup.sh && cp  ../libscidb4geo.so "$(SCIDB)/lib/scidb/plugins"
	@echo "DONE. Please remember to restart SciDB."
deploy:
	cp  libscidb4geo.so "$(SCIDB)/lib/scidb/plugins"
	@echo "DONE. Please remember to restart SciDB."

format:
	find src/ -type f -not -path "src/extern/*" -iname *.h  -exec clang-format-3.9 -i -style=file {} \;
	find src/ -type f -not -path "src/extern/*" -iname *.cxx  -exec clang-format-3.9 -i -style=file {} \;