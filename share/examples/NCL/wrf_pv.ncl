;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adapted from the script "wrf_pv.ncl" at:
;; http://www.mmm.ucar.edu/wrf/OnLineTutorial/Graphics/NCL/Examples/DIAGNOSTICS/wrf_pv.htm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 
;   Example script to produce Vorticity plots from WRF ARW model data
;   November 2008

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/wrf/WRFUserARW.ncl"
load "$VAPOR_HOME/share/examples/NCL/wrf2geotiff.ncl"

begin
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Add your own data here....
;
; The WRF ARW input file.  
; This needs to have a ".nc" appended, so just do it.
  ;;;a = addfile("./wrfout_d01_2005-08-28_00.nc","r")
  wrffiles = systemfunc("ls ./HurricaneIKE/wrfout_d03_2008*")
  numFiles = dimsizes(wrffiles)
  do i=0,numFiles-1
    wrffiles(i) = wrffiles(i) + ".nc"
  end do
  inpFiles = addfiles(wrffiles,"r")
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Output type must be postscript...
  type = "ps"    
  wks = gsn_open_wks(type,"plt_pv")

  ; Do we want the Geotiffs cropped?
  if (.not.isdefined("cropPlot")) then
      cropPlot = True
  end if

  ; initialize our tiff-capture process...
  wrf2gtiff = wrf2geotiff_open(wks)

; Set some basic resources
  res = True
  res@MainTitle                   = "REAL-TIME WRF"

  mpres = True
  pltres = True
  pltres@FramePlot = False   ; Extremely important for geotiff output process


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; loop over files...
do ifile = 0, numFiles-1
  a = inpFiles[ifile]

; What times and how many time steps are in the data set?
  times  = wrf_user_list_times(a)  ; get times in the file
  ntimes = dimsizes(times)         ; number of times in the file

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  do it = 0,ntimes-1,2             ; TIME LOOP

    print("Working on time: " + times(it) )
    res@TimeLabel = times(it)   ; Set Valid time to use on plots


  ; Get the data
    pvo   = wrf_user_getvar(a,"pvo",it)
    avo   = wrf_user_getvar(a,"avo",it)
    p     = wrf_user_getvar(a,"pressure",it)


  ; Interpolate to pressure
    pv_plane = wrf_user_intrp3d(pvo,p,"h",300.,0,False)
    av_plane = wrf_user_intrp3d(avo,p,"h",500.,0,False)


    ; Plotting options 
      opts = res                         
      opts@cnFillOn = True  
      opts@gsnSpreadColorEnd = -3  ; End third from the last color in color map
      opts@ContourParameters = (/ 0., 100., 10./)
      ;; Original script generated two distinct plots per timestep...
      ;;contour_a = wrf_contour(a,wks,av_plane,opts)

      opts@ContourParameters = (/ 0., 5., 1./)
      contour = wrf_contour(a,wks,pv_plane,opts)
      delete(opts)

    ; MAKE PLOTS                                       
      ;; Original script generated two distinct plots per timestep...
      ;;plot = wrf_map_overlays(a,wks,(/contour_a/),pltres,mpres)

      plot = wrf_map_overlays(a,wks,(/contour/),pltres,mpres)
      wrf2geotiff_write(wrf2gtiff, a, times(it), wks, plot, cropPlot)
      frame(wks)  ; Now that we are done drawing, draw the frame

  end do        ; END OF TIME LOOP
end do          ; END OF FILES LOOP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

wrf2geotiff_close(wrf2gtiff, wks)

end
