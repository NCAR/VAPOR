;
; This NCL script generates a global, geo-referenced map image with color
; filled political boundaries. The resulting image can be correctly 
; registered and displayed by vaporgui using the "Image" tab
;
; N.B. as of version 6.0 of NCL support for rendering to a raster
; image is limited. Hence, we have to render to postscript, and then
; rasterize the image. Getting the process right is somewhat tricky,
; and the reason for much of the complexity of this code.

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"

begin

 wks_type = "ps"
 wks_type@wkForegroundColor = "white"


 wks = gsn_open_wks(wks_type,"temp")

;
; Set up some map resources.
;
 mpres = True
;
; Take up as much of viewport as possible.
;
; This is not really necessary since we're going to remove
; the white space later with "convert".
;
 mpres@vpXF = 0.0
 mpres@vpYF = 1.0
 mpres@vpWidthF = 1.0
 mpres@vpHeightF = 1.0

 ;
 ; The perimeter boarder is necessary so that later we can use 
 ; "convert's", -trim option to remove PostScripts page margins
 ;
 mpres@gsnTickMarksOn = False
 mpres@mpPerimDrawOrder = "PostDraw"
 mpres@mpPerimLineColor = "blue"
 mpres@mpPerimOn = True

 ;
 ; Select the map projection. This must match the "+proj" option we
 ; use later when we call tiff2geotif. Note, an NCL CylindricalEquidistant
 ; projection is a "latlong" projection in tiff2geotiff speak
 ;
 mpres@mpProjection = "CylindricalEquidistant"
 mpres@mpLimitMode = "LatLon"

 ;
 ; Select the map database
 ;
 mpres@mpDataBaseVersion = "MediumRes"    ; Use the high-res database
 mpres@mpDataSetName = "Earth..4"    ; Use the high-res database
 mpres@mpDataResolution = "Fine"


 mpres@mpFillOn = False
 mpres@mpOutlineOn = True
 mpres@mpOutlineBoundarySets = "AllBoundaries"   ; turn on country boundaries
 mpres@mpUSStateLineColor = "yellow"
 mpres@mpGeophysicalLineColor = "yellow"

 ;
 ; Specify the map coordinates. The whole world
 ;
 ; N.B. There is a bug in vaporgui that prevents true periodic boundaries. I.e
 ; MaxLon cannot equal MinLat + 360, for example
 ; 
 mpres@mpMinLonF             =  -180.0
 mpres@mpMaxLonF             =  -60.0
 mpres@mpMinLatF             =  15.0
 mpres@mpMaxLatF             =  80.0



 map = gsn_csm_map(wks,mpres)

;---Close workstation (PS file) so we can convert to png
  delete(wks)

 ;
 ; Convert to png with no margins. You might need to increase 
 ; the -density value if you increase the size of the image.
 ;
 cmd = "convert -trim -density 600 -compress lzw temp.ps temp.tif"
 system(cmd)

 ;
 ; Now insert the projection string into the tiff image. Note, the 
 ; coordinates specified here should match those specified above
 ;
 cmd = "tiff2geotiff -4 +proj=latlong -n'-180.0 15.0 -60.0 80.0' temp.tif USOutline.tif"

 system(cmd)

end
