;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adapted from the script "wrf_EtaLevels.ncl" at:
;; http://www.mmm.ucar.edu/wrf/OnLineTutorial/Graphics/NCL/Examples/LEVELS_MODEL/wrf_EtaLevels.htm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
;   Example script to produce plots for a WRF real-data run,
;   with the ARW coordinate dynamics option.

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/wrf/WRFUserARW.ncl"
load "$VAPOR_HOME/share/examples/NCL/wrf2geotiff.ncl"

begin
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Add your own data here....
;
; The WRF ARW input file.  
; This needs to have a ".nc" appended, so just do it.
  ;;;a = addfile("./wrfout_d01_2000-01-24_12:00:00.nc","r")
  wrffiles = systemfunc("ls ./HurricaneIKE/wrfout_d03_2008*")
  numFiles = dimsizes(wrffiles)
  do i=0,numFiles-1
    wrffiles(i) = wrffiles(i) + ".nc"
  end do
  inpFiles = addfiles(wrffiles,"r")

; Output type must be postscript...
  type = "ps"    
  wks = gsn_open_wks(type,"plt_EtaLevels")

  ; Do we want the Geotiffs cropped?
  if (.not.isdefined("cropPlot")) then
      cropPlot = True
  end if

  ; initialize our tiff-capture process...
  wrf2gtiff = wrf2geotiff_open(wks)

; Set some Basic Plot options
  res = True
  res@MainTitle = "REAL-TIME WRF"

  pltres = True
  pltres@FramePlot = False
  mpres = True
  mpres0 = True
  mpres0@mpGeophysicalLineColor = "Black"
  mpres0@mpNationalLineColor    = "Black"
  mpres0@mpUSStateLineColor     = "Black"


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; loop over files...
do ifile=0, numFiles-1
  a = inpFiles[ifile]
  ; What times and how many time steps are in the data set?
  times  = wrf_user_list_times(a)  ; get times in the file
  ntimes = dimsizes(times)         ; number of times in the file
  
  do it = 0,ntimes-1,2             ; TIME LOOP

    print("Working on time: " + times(it) )
    res@TimeLabel = times(it)   ; Set Valid time to use on plots

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; First get the variables we will need        

    th  = wrf_user_getvar(a,"theta",it)   ; theta
    qv  = wrf_user_getvar(a,"QVAPOR",it)  ; Qv
      qv = qv*1000.
      qv@units = "g/kg"

    u   = wrf_user_getvar(a,"ua",it)      ; u averaged to mass points
    v   = wrf_user_getvar(a,"va",it)      ; v averaged to mass points
      spd = (u*u + v*v)^(0.5)             ; speed in m/sec
      spd@description = "Wind Speed"
      spd@units = "m/s"
      u = u*1.94386                       ; winds now in kts
      v = v*1.94386                       ; winds now in kts
      u@units = "kts"
      v@units = "kts"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    dimsv = dimsizes(th)          ; Get levels
    ;; original script looped over levels...
    ;;;do level =0,dimsv(0)-1,5      ; LOOP OVER LEVELS
       level = 9

       display_level = level + 1
       res@PlotLevelID = "Eta Level  " + display_level


     ; Theta
       ;opts = res
       ;opts@cnLineColor         = "Red"
       ;opts@cnInfoLabelOn       = False
       ;opts@lbLabelsOn          = False
       ;opts@ContourParameters   = (/ 5.0 /)
       ;opts@gsnContourLineThicknessesScale = 2.0
       ;contour = wrf_contour(a,wks,th(level,:,:),opts)
       ;plot = wrf_map_overlays(a,wks,(/contour/),pltres,mpres0)
       ;delete(opts)
   
     ; Qv
       ;opts = res
       ;opts@cnLineColor         = "Blue"
       ;opts@cnFillOn            = True
       ;opts@lbLabelsOn          = False
       ;contour = wrf_contour(a,wks,qv(level,:,:),opts)
       ;plot = wrf_map_overlays(a,wks,(/contour/),pltres,mpres)
       ;delete(opts)
   
     ; Wind Vectors and Speed
       opts = res
       opts@ContourParameters = (/ 15., 60., 5. /)
       opts@cnFillOn          = True
       contour = wrf_contour(a,wks,spd(level,:,:),opts)
       delete(opts)
   
       opts = res
       opts@FieldTitle        = "Wind"       ; Overwrite Field Title
       opts@NumVectors        = 47           ; wind barb density
       vector =  wrf_vector(a,wks,u(level,:,:),v(level,:,:),opts)
       delete(opts)

       plot = wrf_map_overlays(a,wks,(/contour, vector/),pltres,mpres)
       wrf2geotiff_write(wrf2gtiff, a, times(it), wks, plot, cropPlot)
       frame(wks)  ; Now that we are done drawing, draw the frame


    ;;;;end do      ; END OF LEVEL LOOP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  end do        ; END OF TIME LOOP
end do          ; END OF FILES LOOP

wrf2geotiff_close(wrf2gtiff, wks)   
end
