#library('devtools')
#install_github("SciDBR","paradigm4",quick=TRUE)

# Download sample file 
download.file("http://download.osgeo.org/geotiff/samples/spot/chicago/UTM2GTIF.TIF","sample.tif")


library(sp)
library(scidb)
library(rgdal)
library(proj4)

sp2scidb <- function(x, arrayname) {
  g = x@grid
  proj4text = x@proj4string@projargs
  affine = paste("x0=", g@cellcentre.offset[1], " y0=", g@cellcentre.offset[2], " a11=", g@cellsize[1] , " a22=", g@cellsize[2], sep="")
  nband = ncol(x)
  # Upload and reshape array
  
  temp <- as.scidb(X = x@data, name = "temp")
  scidbarray <- rename(reshape(temp,shape=rev(g@cells.dim),dimnames=c("x","y"),eval=T),name=arrayname)
  #scidbremove(temp,force = T)
  
  q <- paste("eo_regnewsrs('", scidbarray@name ,"',", 1, ",'','", proj4text ,"');",  sep="")
  iquery(q)
  q <- paste("eo_setsrs(", scidbarray@name, ",'x','y','", scidbarray@name , "', 1, '" , affine, "');", sep="")
  iquery(q)
  return(scidbarray)
}


scidb2sp <- function(scidbarray) {
  data = scidbarray[]
  nrow<-dim(data)[1]
  ncol<-dim(data)[2]
  #data <- data.frame(as.vector(scidbarray[])
   
  q <- paste("eo_getsrs(", scidbarray@name, ");", sep="")
  srs <- iquery(q,return=T)
  
  # Parse affine transform string
  A <- c(0,0,1,0,0,1)
  pars <- strsplit(srs$A, " ")[[1]]
  for (i in 1:length(pars)) {
    p <- strsplit(pars[i], "=")[[1]]
    if (length(p) == 2) {
      if (p[1] == "x0") A[1] = as.double(p[2])
      if (p[1] == "y0") A[2] = as.double(p[2])
      if (p[1] == "a11") A[3] = as.double(p[2])
      if (p[1] == "a12") A[4] = as.double(p[2])
      if (p[1] == "a21") A[5] = as.double(p[2])
      if (p[1] == "a22") A[6] = as.double(p[2])
    }
  }
  if (A[4] != 0 || A[5] != 0) {
    warning("Array seems to be rotated or sheared, which is not supported in sp objects")
  }
  
  g <- GridTopology(cellcentre.offset = c(A[1],A[2]), cellsize = c(A[3],A[6]), cells.dim = c(ncol,nrow))
  df = data.frame(as.vector(t(data)))
  names(df) <- scidbarray@attributes
  return (SpatialGridDataFrame(grid = g, data=df, proj4string = srs$proj4text))      
}





scidbconnect()

chicago_spot <- readGDAL("sample.tif")
#spplot(chicago_spot,scales=list(draw=T))


chicago_arrayref <- sp2scidb(chicago_spot, "chicago2")
#image(t(chicago_arrayref))
#apply(chicago_arrayref,FUN = function(v) )

#chicago_arrayref = bind(chicago_arrayref, "band2", "band1 / 255 ", eval=T)


chicago_arrayref = scidb("chicagoT")

chicago_local <- scidb2sp(chicago_arrayref)

spplot(chicago_local,scales=list(draw = TRUE) )
