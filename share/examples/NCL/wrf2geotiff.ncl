load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"

; Set this to true to save intermediate files...
_wrf2geotiff_DEBUG = False

; Some symbolic constants (hopefully these don't collide with anything else!)
_wrf2geotiff_LANDSCAPE = 6
_wrf2geotiff_PSMINXY = 0
_wrf2geotiff_PSMAXX = 612   ; 8.5"
_wrf2geotiff_PSMAXY = 792   ; 11"
_wrf2geotiff_GROWSIZE = 10

; ------------------------------------------------------------------------------------
; 
; wrf2geotiff_open()
;
; Called once to prime the plot capture process.
;
;
; NOTE: at present, only postscript workstations are supported. We'd like to support
; either PS or PDF, but are currently lacking a means to split a multi-page plotfile
; into separate pages for PDF (Imagemagick's convert program introduces noise and minor
; translations per-page).
;
undef("wrf2geotiff_open")
function wrf2geotiff_open(wks)
local class
begin
    if (_wrf2geotiff_DEBUG .eq. True) then
        print("wrf2geotiff_open")
    end if
     
    ; make sure we're dealing with postscript...
    class = NhlClassName(wks)
    if (class.ne."psWorkstationClass") then
         print("wrf2geotiff_open(): Workstation type not PS")
         print("wrf2geotiff processes will fail")
         return False
    end if

    ; We create this "opaque" state variable that gets returned to the client code that then uses
    ; it on subsequent calls to this API. All of the per-plot state is kept in array attributes, which
    ; are sized initially and expand dynamically as needed.

    tcVar = True
    tcVar@numPlots = 0
    if (class.eq."pdfWorkstationClass") then
        getvalues wks
            "wkPDFFileName":  filename
        end getvalues
    else
        getvalues wks
            "wkPSFileName":  filename
        end getvalues
    end if
    tcVar@plotFilename = filename
    tcVar@outputScale = 2   ; default scaling factor for tiff images made from postscript versions
    tcVar@disableGeoTags = False

    ; array attributes...
    tcVar@times = new(_wrf2geotiff_GROWSIZE, string)
    tcVar@orient = new(_wrf2geotiff_GROWSIZE, string)
    tcVar@proj4Str = new(_wrf2geotiff_GROWSIZE, string)
    tcVar@pageMinX = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@pageMaxX = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@pageMinY = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@pageMaxY = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@minLon = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@maxLon = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@minLat = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@maxLat = new(_wrf2geotiff_GROWSIZE, float)
    tcVar@cropFrame = new(_wrf2geotiff_GROWSIZE, logical)

    return tcVar
end

; --------------------------------------------------------------------
;
; _wrf2geotiff_growArrays
;
; Intended as a private function;  used to resize the array attributes that keep track of
; per-plot state.
;
undef("_wrf2geotiff_growArrays")
procedure _wrf2geotiff_growArrays(tcVar)
local atts, i, currSize, newSize, tmp
begin
  if (_wrf2geotiff_DEBUG .eq. True) then
      print("_wrf2geotiff_growArrays(): growing arrays sizes...")
  end if 
  atts = getvaratts(tcVar)
  do i=0,dimsizes(atts)-1
    currSize = dimsizes( tcVar@$atts(i)$ )
    ; By design, anything that reports a dimsize of "1" is a scalar attribute,
    ; and we won't attempt to grow it.
    if (currSize .ne. 1) then
        newSize = currSize + _wrf2geotiff_GROWSIZE
        tmp = new(newSize, typeof( tcVar@$atts(i)$ ))
        tmp(0:currSize-1) = tcVar@$atts(i)$
        delete(tcVar@$atts(i)$)
        tcVar@$atts(i)$ = tmp
        delete(tmp)  ; very important to "clear" this for next time through the loop...
    end if
  end do
end

; ------------------------------------------------------------------------------
;
; _wrf2geotiff_getProj4Str()
;
; Intended as a private function;  given a wrf filevariable, extracts the
; geographical projection information and returns it as a PROJ4 string.
;
undef("_wrf2geotiff_getProj4Str")
function _wrf2geotiff_getProj4Str(wrfFile)
local proj, lon0, lat0, lat1, lat2, projStr, northSouth, stdLon, trueLat, poleLat
begin
    proj = wrfFile@MAP_PROJ

    if (proj .eq. 1) then  ; Lambert Conformal
        lon0 = " +lon_0=" + wrfFile@STAND_LON
        lat0 = " +lat_0=" + wrfFile@MOAD_CEN_LAT
        lat1 = " +lat_1=" + wrfFile@TRUELAT1
        lat2 = " +lat_2=" + wrfFile@TRUELAT2
        projStr = " +proj=lcc +ellps=sphere" + lon0 + lat0 + lat1 + lat2
        return projStr
    end if

    if (proj .eq. 3) then  ; Mercator
        latTs = " +lat_ts=" + wrfFile@TRUELAT1
        lon0 = " +lon_0=" + wrfFile@STAND_LON
        projStr = " +proj=merc +ellps=sphere" + lon0 + latTs
        return projStr
    end if

    if (proj .eq. 2) then ; Polar Stereographic
       if (wrfFile@MOAD_CEN_LAT .lt. 0) then
          northSouth = " +lat_0=-90"
       else
          northSouth = " +lat_0=90"
       end if
       stdLon = " +lon_0=" + wrfFile@STAND_LON
       trueLat = " +lat_ts=" + wrfFile@TRUELAT1
       projStr = " +proj=stere +ellps=sphere" + stdLon + northSouth + trueLat
       return projStr
    end if

    if (proj .eq. 6) then  ; rotated Cassini
       ;: as of 4/29/2009, WRF files are not yet wired with these attributes.
       ;;poleLat = " +o_lat_p=" + wrfFile@POLE_LAT
       ;;poleLon = " +o_lon_p=" + wrfFile@POLE_LON

       ; Hardcode values for testing...
       poleLat = " +o_lat_p=50"
       poleLon = " +o_lon_p=180"
       lon0 = " +lon_0=" + wrfFile@STAND_LON
       projStr = " +proj=ob_tran +ellps=sphere +o_proj=cass " +  poleLon + poleLat + lon0
       return projStr
    end if

    print("wrf2geotiff: Unknown map projection: " + proj)
    print("wrf2geotiff: georeferencing will fail ");
    return ""
end

; ----------------------------------------------------------------------------------
;
; _wrf2geotiff_validateProj4String()
;
; Intended as a private function. Perhaps a sanity check on our assumptions by
; looping though the various proj4 strings collected during process, and verifying
; that they are all the same. A warning is printed if they are not, and in any case
; the first such instance is returned as a quoted string.
;
undef("_wrf2geotiff_validateProj4String")
function _wrf2geotiff_validateProj4String(tcVar)
local quote, proj4Str
begin
   quote = inttochar(34)
   if (tcVar@numPlots .eq. 0) then
       return " "
   end if

   proj4Str = tcVar@proj4Str(0)
   do i=1,tcVar@numPlots-1
       if (tcVar@proj4Str(i) .ne. proj4Str) then
           print("WARNING: inconsistent projections across image collection: " + \
               proj4Str + "  --versus--  " + tcVar@proj4Str(i))
       end if
   end do

   return quote + proj4Str + quote
end

; --------------------------------------------------------------------
;
; _wrf2geotiff_getDevVP()
;
; This function was contributed by Dave Brown (originally named get_dev_vp()).
; Gets the location of the viewport in a workstation-independent fashion.
; Intended as a private function.
;
undef("_wrf2geotiff_getDevVP")
function _wrf2geotiff_getDevVP(wks:graphic,plot:graphic)
local class, vp, dc, orient, dvp
begin

  vp = new(4,float)
  getvalues plot
        "vpXF"      : vp(0)
        "vpYF"      : vp(1)
        "vpWidthF"  : vp(2)
        "vpHeightF" : vp(3)
  end getvalues
  dc = new(4,integer)

  orient = 0
  class = NhlClassName(wks)
  if((class.eq."psWorkstationClass").or. \
     (class.eq."pdfWorkstationClass")) then

       getvalues wks
          "wkDeviceLowerX" : dc(0)
          "wkDeviceLowerY" : dc(1)
          "wkDeviceUpperX" : dc(2)
          "wkDeviceUpperY" : dc(3)
          "wkOrientation"  : orient
       end getvalues
  else if class .eq. "ncgmWorkstationClass" then
       dc(0) = 0
       dc(1) = 0
       dc(2) = 1
       dc(3) = 1
  else
       dc(0) = 0
       dc(1) = 0
       getvalues wks
          "wkVSWidthDevUnits" : dc(2)
       end getvalues
      
       dc(3) = dc(2)
  end if
  end if       

  if (_wrf2geotiff_DEBUG .eq. True) then
      print(vp)
      print(dc)
      print(orient)
  end if

  dvp = new(4,float)
  if (orient .eq. 0) then
      dvp(0) = dc(0) + vp(0) * (dc(2) - dc(0))
      dvp(1) = dc(1) + (vp(1) - vp(3)) * (dc(3) - dc(1))
      dvp(2) = dc(0) + (vp(0) + vp(2)) * (dc(2) - dc(0))
      dvp(3) = dc(1) + vp(1) * (dc(3) - dc(1))
  else
      dvp(0) = dc(1) + (1.0 - vp(0)) * (dc(3) - dc(1))
      dvp(1) = dc(0) + (vp(1) - vp(3)) * (dc(2) - dc(0))
      dvp(2) = dc(1) + (1.0 - (vp(0) + vp(2))) * (dc(3) - dc(1))
      dvp(3) = dc(0) + vp(1) * (dc(2) - dc(0))
  end if

  if (_wrf2geotiff_DEBUG .eq. True) then
      print(dvp)
  end if
  dvp@orientation = orient
   
  return (dvp)
end

; ------------------------------------------------------------------------------
;
; wrf2geotiff_write()
;
; Called by client code on a per-plot basis. This method captures information about the
; plot in an opaque state-variable created by a call to "wrf2geotiff_open()".
;
undef("wrf2geotiff_write")
procedure wrf2geotiff_write(tcVar, wrfFile, time, wks:graphic, plot:graphic, crop:logical)
local plotNum, currSizes, vp, xy, yf, width, height, xNDC, yNDC, xWorld, yWorld, slop
begin
   if (_wrf2geotiff_DEBUG .eq. True) then
       print("wrf2geotiff_write")
   end if

   plotNum = tcVar@numPlots  ; dereference this for convenient use below

   currSizes = dimsizes(tcVar@times)
   if (plotNum .ge. currSizes(0)) then
       _wrf2geotiff_growArrays(tcVar)
   end if

   ; get the location of the viewport on the page...
   vp = _wrf2geotiff_getDevVP(wks,plot)

   ; We want the page coords that correspond to LL,LR,UR,UL corners in
   ; world coordinates; this depends on orientation of the plot on the page.
    tcVar@orient(plotNum) = vp@orientation
    if (vp@orientation .eq. _wrf2geotiff_LANDSCAPE) then
       tcVar@pageMinX(plotNum) = vp(1)
       tcVar@pageMaxX(plotNum) = vp(3)
       tcVar@pageMinY(plotNum) = vp(2)
       tcVar@pageMaxY(plotNum) = vp(0)
   else
       tcVar@pageMinX(plotNum) = vp(0)   ;min x
       tcVar@pageMaxX(plotNum) = vp(2)   ;max x
       tcVar@pageMinY(plotNum) = vp(1)   ;min y
       tcVar@pageMaxY(plotNum) = vp(3)   ;max y
   end if 

   ; get the world-coords of the view port. Note that if we use the viewport
   ; extremes exactly, ndctodata() fails. So we add some slop to make sure the 
   ; values passed in are inside the viewport. OBVIOUSLY A STOP-GAP MEASURE!
   getvalues plot
       "vpXF":  xf;
       "vpYF":  yf;
       "vpWidthF": width;
       "vpHeightF":  height;
   end getvalues

   slop = .0000005  ; NOTE: one more decimal-place right is too much
   xNDC = (/ xf+slop, xf+width-slop, xf+width-slop, xf+slop /)
   yNDC = (/ yf-slop, yf-slop, yf-height+slop, yf-height+slop /)
   xWorld = new(dimsizes(xNDC), float)
   yWorld = new(dimsizes(yNDC), float)
   ndctodata(plot, xNDC, yNDC, xWorld, yWorld)

   tcVar@minLon(plotNum) = xWorld(3)
   tcVar@maxLon(plotNum) = xWorld(1)
   tcVar@minLat(plotNum) = yWorld(3)
   tcVar@maxLat(plotNum) = yWorld(1)

   tcVar@proj4Str(plotNum) = _wrf2geotiff_getProj4Str(wrfFile)

   tcVar@times(plotNum) = time
   tcVar@cropFrame(plotNum) = crop

   tcVar@numPlots = plotNum + 1
end

; ------------------------------------------------------------------------------
;
; wrf2geotiff_setOutputScale()
;
; Called by client code to change the default scale-factor for the tiff images.
; The default is 2-times the resolution of the postscript image. 
;
undef("wrf2geotiff_setOutputScale")
procedure wrf2geotiff_setOutputScale(tcVar, scaleFactor:float)
begin
    tcVar@outputScale = scaleFactor 
end

; ------------------------------------------------------------------------------
;
; wrf2geotiff_disableGeoTags()
;
; Called by client code to disable writing of geotags in the tiff image; only timestamps
; are written. This is useful for plots that are vertical cross-sections through the data.
;
undef("wrf2geotiff_disableGeoTags")
procedure wrf2geotiff_disableGeoTags(tcVar)
begin
    tcVar@disableGeoTags = True
end

; --------------------------------------------------------------------
;
; wrf2geotiff_close()
;
; Called by client code after all plots have been captured and/or sent to
; the workstation. Using the information contained in the opaque state variable
; "tcVar", the following tasks are performed:
;   1. splits a potentially multi-page plot file into separate pages
;   2. Converts each postscript page into a tiff file.
;   3. Computes georeferencing info for each tiff file.
;   4. Injects georeferencing and the timestamp into each tiff file
;   5. Copies individual geotiffs into one multi-image geotiff.
;
; NOTE: this function causes the workstation to be closed and the plot output
; to be flushed to the filesystem.
;
undef("wrf2geotiff_close")
procedure wrf2geotiff_close(tcVar, wks:graphic)
local plotFile, outputScale, wksId, tmpfile, tmpfileRoot, cmd, outStrings, quote, numPages, i, rotation, \
      cropStr, w, h, x, y, pageLX, pageRX, pageTY, pageBY, \
      outFilesRoot, suffix, plotFileLen, chr, multiTiff, multiGTiff, dataFile, proj4String
begin
   if (_wrf2geotiff_DEBUG .eq. True) then
       print("wrf2geotiff_close")
       print(tcVar)
    end if 

   plotFile = tcVar@plotFilename
   outputScale = tcVar@outputScale

   ; generate a filename root for a set of tmp files, based on the workstation ID...
   getvalues wks
       "wkGksWorkId": wksId
   end getvalues

   ; We need to ensure the plotfile is closed before attempting to split it up; 
   ; otherwise, the "psplit" command fails.
   destroy(wks)

   ; split the postscript plotfile into multiple pieces...
   tmpfileRoot = "/tmp/tiffcap" + wksId + "_"
   cmd = "psplit " + plotFile + " " + tmpfileRoot
   print(cmd)
   system(cmd)

     ;;; Attempts to use Imagemagick's tools to split PS file, or to perform operations
     ;;; on frames within a multi=page file have generally been unsuccessful (perhaps missing
     ;;; something?)
     ;;; tmpfileRoot = "/tmp/tiffcap" + wksId + "_"
     ;;; cmd = "convert -page letter +adjoin " + plotFile + " ps:" + tmpfileRoot + "%04d.ps"
     ;;; system(cmd)

   ; Container for the strings to be written to the vapor-data file...
   outStrings = new(tcVar@numPlots, string)
   quote = inttochar(34)

   numPages = tcVar@numPlots
   do i=0,numPages-1

      ;
      ; pick up the next postscript page a convert to a tiff, rotating if necessary...
      ;
      tmpfile = tmpfileRoot + sprinti("%0.4i", i+1)
      rotation = 0
      if (tcVar@orient(i) .ne. 0) then
          rotation = -90
      end if

      cropStr = ""
      if (tcVar@cropFrame(i) .eq.True) then
          w = (tcVar@pageMaxX(i) - tcVar@pageMinX(i)) * outputScale
          h = (tcVar@pageMaxY(i) - tcVar@pageMinY(i)) * outputScale
          x = tcVar@pageMinX(i) * outputScale
          y = (_wrf2geotiff_PSMAXY - tcVar@pageMaxY(i)) * outputScale
          cropStr = " -crop " + w + "x" + h + "+" + x + "+" + y + " "
      end if

      ; Note the ordering of the -density and -crop arguments is important in older versions
      ; of Imagemagick (e.g., 6.0.7 04/20/08).  As of vers. 6.2.8 06/10/08, the ordering does not
      ; matter.
      cmd = "convert  -depth 8 -rotate " + rotation + " -density " + (72*outputScale) + " " + cropStr + " " 
      cmd = cmd +  " " + tmpfile + ".ps tiff:" + tmpfile + ".tiff"
      print(cmd)
      system(cmd)

      if (tcVar@cropFrame(i) .eq. True) then
         pageLX = 0.
         pageRX = 1.
         pageBY = 0.
         pageTY = 1.
      else
         ; compute "normalized" page coordinates of the plot corners...
         pageLX = tcVar@pageMinX(i) / _wrf2geotiff_PSMAXX 
         pageRX = tcVar@pageMaxX(i) / _wrf2geotiff_PSMAXX
         pageBY = tcVar@pageMinY(i) / _wrf2geotiff_PSMAXY
         pageTY = tcVar@pageMaxY(i) / _wrf2geotiff_PSMAXY
      end if 

      if (tcVar@disableGeoTags) then
          outStrings(i) = tcVar@times(i)
      else
          outStrings(i) = tcVar@times(i) + " " + tcVar@minLon(i) + " " + tcVar@minLat(i) + " " + \
             tcVar@maxLon(i) + " " + tcVar@maxLat(i) + " " + pageLX + " " + pageBY + " " + \
             pageRX + " " + pageTY
      end if
   end do

   ; need a filename root for naming several files in the following...
   outFilesRoot = plotFile
   suffix = indStrSubset(str_lower(plotFile), ".ps") 
   ; furthermore, make sure our substring is indeed a suffix...
   plotFileLen = strlen(plotFile)
   if (.not.any(ismissing(suffix)) .and. ((plotFileLen-1) .eq. suffix(dimsizes(suffix)-1))) then
       ; a bit messy; is there a better way to do these string ops?
       chr = stringtocharacter(plotFile)
       outFilesRoot = charactertostring( chr(0:suffix(0)-1) )
   end if

   multiTiff = outFilesRoot + "_temp.tiff"
   multiGTiff = outFilesRoot + ".tiff"
   dataFile = outFilesRoot + ".dat"

   ; append all the intermediate tiff files into one multi-image tiff...
   cmd = "convert -adjoin " + tmpfileRoot + "[0-9]*.tiff " + multiTiff
   print(cmd)
   system(cmd)

   ; write the georeferencing info into a temporary file...
   asciiwrite(dataFile, outStrings)
   
   ; call a custom utility to create a geotiff...

   if (tcVar@disableGeoTags) then
       cmd = "tiff2geotiff  -M " + dataFile + " " + multiTiff + " " + multiGTiff
   else
       proj4String = _wrf2geotiff_validateProj4String(tcVar)
       cmd = "tiff2geotiff -4 " + proj4String + " -m " + dataFile + " " + \
           multiTiff + " " + multiGTiff
   end if
   print(cmd)
   system(cmd)

   ; cleanup
   cmd = "rm -f " + tmpfileRoot + "[0-9]*.* " + dataFile + " " + multiTiff
   print(cmd)
   if (_wrf2geotiff_DEBUG .ne. True) then  
       system(cmd)
   end if

end
