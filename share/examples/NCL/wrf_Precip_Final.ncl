
;   Example script to produce plots for a WRF real-data run,
;   with the ARW coordinate dynamics option.

; From the "Using NCL with VAPOR to Visualize WRF-ARW data"
; tutorial.

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/wrf/WRFUserARW.ncl"

; load the wrf2geotiff library from the VAPOR distribution:
load "$VAPOR_HOME/share/examples/NCL/wrf2geotiff.ncl"

begin
;
; The WRF ARW input file.  
; This needs to have a ".nc" appended, so just do it.

; Instead of selecting one file, make a variable that has
; all the names of the WRF output files in it:
; a = addfile("wrfout_d02_2008-09-28_00.nc","r")
  wrffiles = systemfunc("ls wrfout_d02_2008-09-28*")
  numFiles = dimsizes(wrffiles)
  do i = 0, numFiles-1
    wrffiles(i) = wrffiles(i)+".nc"
  end do
  inpFiles = addfiles(wrffiles,"r")

; We generate plots, but what kind do we prefer?
; For wrf2geotiff, the type MUST be ps
; type = "x11"
; type = "pdf"
  type = "ps"
; type = "ncgm"
  wks = gsn_open_wks(type,"plt_Precip")

; wrf2geotiff_open() creates the opaque pointer wrf2gtiff:
  wrf2gtiff = wrf2geotiff_open(wks)

; Set some basic resources
  res = True
  res@MainTitle = "REAL-TIME WRF"

; Following are needed for control of frame advance:
  pltres = True
  pltres@gsnFrame = False

  pltres = True
  mpres = True
  mpres@mpGeophysicalLineColor = "Black"
  mpres@mpNationalLineColor    = "Black"
  mpres@mpUSStateLineColor     = "Black"
  mpres@mpGridLineColor        = "Black"
  mpres@mpLimbLineColor        = "Black"
  mpres@mpPerimLineColor       = "Black"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; What times and how many time steps are in the data set?
  FirstTime = True

; Make a loop over all the files:
  do ifile = 0,numFiles -1
    a = inpFiles[ifile]

    times  = wrf_user_list_times(a)  ; get times in the file
    ntimes = dimsizes(times)         ; number of times in the file

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Loop over all the times, not just the odd times:
; do it = 0,ntimes-1,2 ; TIME LOOP
    do it = 0, ntimes-1 ; Modified to do every time step

      print("Working on time: " + times(it) )
      if (FirstTime) then            ; Save some times for tracking tendencies
        times_sav = times(it)
      end if
      res@TimeLabel = times(it)   ; Set Valid time to use on plots

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; First get the variables we will need        

    slp = wrf_user_getvar(a,"slp",it)  ; slp
      wrf_smooth_2d( slp, 3 )            ; smooth slp

  ; Get non-convective, convective and total precipitation
  ; Calculate tendency values                               
      rain_exp = wrf_user_getvar(a,"RAINNC",it)
      rain_con = wrf_user_getvar(a,"RAINC",it)
      rain_tot = rain_exp + rain_con
      rain_tot@description = "Total Precipitation"

      if( FirstTime ) then
        if ( it .eq. 0 ) then
          rain_exp_save = rain_exp
          rain_con_save = rain_con
          rain_tot_save = rain_tot
        else
          rain_exp_save = wrf_user_getvar(a,"RAINNC",it-1)
          rain_con_save = wrf_user_getvar(a,"RAINC",it-1)
          rain_tot_save = rain_exp_save + rain_con_save
          FirstTime = False
          times_sav = times(it-1)
        end if
      end if

      rain_exp_tend = rain_exp - rain_exp_save
      rain_con_tend = rain_con - rain_con_save
      rain_tot_tend = rain_tot - rain_tot_save
      rain_exp_tend@description = "Explicit Precipitation Tendency"
      rain_con_tend@description = "Param  Precipitation Tendency"
      rain_tot_tend@description = "Precipitation Tendency"

  ; Bookkeeping, just to allow the tendency at the next time step
      rain_exp_save = rain_exp
      rain_con_save = rain_con
      rain_tot_save = rain_tot

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Don't skip the first time: we need an image at each time stamp
; This will result in a warning message on the first plot
;     if( .not. FirstTime ) then ; We don’t skip the first time

      ; Plotting options for Sea Level Pressure
        opts_psl = res          
        opts_psl@ContourParameters = (/ 900., 1100., 2. /)
        opts_psl@cnLineColor       = "Blue"
        opts_psl@cnInfoLabelOn     = False
        opts_psl@cnLineLabelFontHeightF = 0.01
        opts_psl@cnLineLabelPerimOn = False
        opts_psl@gsnContourLineThicknessesScale = 1.5
        contour_psl = wrf_contour(a,wks,slp,opts_psl)
        delete(opts_psl)
    

      ; Plotting options for Precipitation
        opts_r = res                        
        opts_r@UnitLabel            = "mm"
        opts_r@cnLevelSelectionMode = "ExplicitLevels"
        opts_r@cnLevels             = (/ .1, .2, .4, .8, 1.6, 3.2, 6.4, \
                                        12.8, 25.6, 51.2, 102.4/)
        opts_r@cnFillColors         = (/"White","White","DarkOliveGreen1", \
                                        "DarkOliveGreen3","Chartreuse", \
                                        "Chartreuse3","Green","ForestGreen", \
                                        "Yellow","Orange","Red","Violet"/)
        opts_r@cnInfoLabelOn        = False
        opts_r@cnConstFLabelOn      = False
        opts_r@cnFillOn             = True
    

      ; Total Precipitation (color fill)
        contour_tot = wrf_contour(a,wks, rain_tot, opts_r)
    
      ; Precipitation Tendencies 
        opts_r@SubFieldTitle = "from " + times_sav + " to " + times(it)
    
        contour_tend = wrf_contour(a,wks, rain_tot_tend,opts_r) ; total (color)
        contour_res = wrf_contour(a,wks,rain_exp_tend,opts_r)   ; exp (color)
        opts_r@cnFillOn = False
        opts_r@cnLineColor = "Red4"
        contour_prm = wrf_contour(a,wks,rain_con_tend,opts_r)   ; con (red lines)
        delete(opts_r)



      ; MAKE PLOTS                                       

        ; Total Precipitation 
        ; Don’t do this plot for geotiff:
        ;  plot = wrf_map_overlays(a,wks,contour_tot,pltres,mpres)

        ; Total Precipitation Tendency + SLP
        ; Do just this one plot:
          plot = wrf_map_overlays(a,wks,(/contour_tend,contour_psl/),pltres,mpres)

        ; Then put it in geotiff
        ; Do crop the image to the domain bounds
        wrf2geotiff_write(wrf2gtiff, a, times(it), wks, plot, True)
        ; The frame is needed between time steps after the wrf2geotiff_write
        frame(wks)

        ; Non-Convective and Convective Precipiation Tendencies
        ; plot = wrf_map_overlays(a,wks,(/contour_res,contour_prm/),pltres,mpres)

;     end if    ; END IF FOR SKIPPING FIRST TIME

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

      times_sav = times(it)
      FirstTime = False
    end do        ; END OF TIME LOOP
  end do ; END OF LOOP OVER FILES

; At the end, close the wrf2geotiff
; This is where the tiff writing takes place:
  wrf2geotiff_close(wrf2gtiff,wks)

end
