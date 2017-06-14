
; Example script to produce plots for a WRF real-data run,
; with the ARW coordinate dynamics option.
; Interpolating to specified height levels

; From the "Using NCL with VAPOR to Visualize WRF-ARW data"
; tutorial.

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/wrf/WRFUserARW.ncl"

; Load the wrf2geotiff library.
; The wrf2geotiff library is installed with VAPOR:
load "$VAPOR_HOME/share/examples/NCL/wrf2geotiff.ncl"
begin
;
; The WRF ARW input file.
; This needs to have a ".nc" appended, so just do it.

; Create a list of all the WRF output files we are using
; from the typhoon Jangmi simulation:
  wrffiles = systemfunc("ls wrfout_d02_2008-09-28*")
  numFiles = dimsizes(wrffiles)
  do i = 0, numFiles-1
    wrffiles(i) = wrffiles(i) + ".nc"
  end do
  inpFiles = addfiles(wrffiles,"r")

; We generate plots, but what kind do we prefer?

; When using wrf2geotiff, type must be ps
; type = "x11"
; type = "pdf"
  type = "ps"
; type = "ncgm"
  wks = gsn_open_wks(type,"plt_HeightLevel")
; Create the wrf2gtiff opaque pointer, which will
; be referenced in all geotiff changes
  wrf2gtiff = wrf2geotiff_open(wks)

; Set some basic resources
  res = True
  res@MainTitle = "REAL-TIME WRF"
  res@Footer = False

; For wrf2geotiff we need to control frame advance:
  pltres = True
  pltres@gsnFrame = False
 
  pltres = True
  mpres = True
  mpres@mpGeophysicalLineColor = "Black"
  mpres@mpNationalLineColor = "Black"
  mpres@mpUSStateLineColor = "Black"
  mpres@mpGridLineColor = "Black"
  mpres@mpLimbLineColor = "Black"
  mpres@mpPerimLineColor = "Black"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; What times and how many time steps are in the data set?
  do ifile = 0, numFiles-1 ; LOOP OVER FILES
    a = inpFiles[ifile]
    times = wrf_user_list_times(a) ; get times in the file
    ntimes = dimsizes(times) ; number of times in the file

; The specific height levels that we want the data interpolated to.

; We use height of 5000m in the plot for VAPOR
; That height is near the top of the typhoon
   height_levels = (/ 250., 5000./) ; height levels to plot - in meter
   nlevels = dimsizes(height_levels) ; number of height levels

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Modify the time loop to include all time steps, not just every odd one
    do it = 0,ntimes-1 ; TIME LOOP

      print("Working on time: " + times(it) )
      res@TimeLabel = times(it) ; Set Valid time to use on plots

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; First get the variables we will need

     tc = wrf_user_getvar(a,"tc",it) ; T in C
     u = wrf_user_getvar(a,"ua",it) ; u averaged to mass points
     v = wrf_user_getvar(a,"va",it) ; v averaged to mass points
     p = wrf_user_getvar(a, "pressure",it) ; pressure is our vertical coordinate
     z = wrf_user_getvar(a, "z",it) ; grid point height
     rh = wrf_user_getvar(a,"rh",it) ; relative humidity

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Modify loop over level to just do one level = 1
     level = 1 ; just one level for the georeferenced plot

      height = height_levels(level)

      p_plane = wrf_user_intrp3d( p,z,"h",height,0.,False)
      tc_plane = wrf_user_intrp3d(tc,z,"h",height,0.,False)
      rh_plane = wrf_user_intrp3d(rh,z,"h",height,0.,False)
      u_plane = wrf_user_intrp3d( u,z,"h",height,0.,False)
      v_plane = wrf_user_intrp3d( v,z,"h",height,0.,False)

      u_plane = u_plane*1.94386 ; kts
      v_plane = v_plane*1.94386 ; kts
      u_plane@units = "kts"
      v_plane@units = "kts"

; Plotting options for T
      opts = res
      opts@cnLineColor = "Red"
      opts@ContourParameters = (/ 5.0 /)
      opts@cnInfoLabelOrthogonalPosF = 0.07 ; offset second label info
      opts@gsnContourLineThicknessesScale = 2.0
      contour_tc = wrf_contour(a,wks,tc_plane,opts)
      delete(opts)

; Plotting options for Pressure
      opts = res
      opts@cnLineColor = "Blue"
      opts@gsnContourLineThicknessesScale = 3.0
      contour_p = wrf_contour(a,wks,p_plane,opts)
      delete(opts)

; Plotting options for RH
      opts = res
      opts@cnFillOn = True
      opts@ContourParameters = (/ 10., 90., 10./)
      opts@cnFillColors = (/"White","White","White", \
		"White","Chartreuse","Green",\
		"Green3","Green4", \
		"ForestGreen","darkorchid3"/)
      contour_rh = wrf_contour(a,wks,rh_plane,opts)
      delete(opts)

; Plotting options for Wind Vectors
      opts = res
      opts@FieldTitle = "Wind" ; overwrite Field Title
      opts@NumVectors = 47 ; wind barb density
      vector = wrf_vector(a,wks,u_plane,v_plane,opts)
      delete(opts)

; MAKE PLOTS
      plot = wrf_map_overlays(a,wks,(/contour_rh,contour_tc,contour_p, \
            vector/),pltres,mpres)

; capture plot georeferencing information
; then change the frame
; Don’t crop (leave annotation intact)
      wrf2geotiff_write(wrf2gtiff, a, times(it), wks, plot, False)
      frame(wks)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    end do ; END OF TIME LOOP
  end do ; END OF LEVEL LOOP

; Call wrf2geotiff_close at the end
  wrf2geotiff_close(wrf2gtiff, wks)

end
