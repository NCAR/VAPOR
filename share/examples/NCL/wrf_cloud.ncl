;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adapted from the script "wrf_cloud.ncl" at:
;; http://www.mmm.ucar.edu/wrf/OnLineTutorial/Graphics/NCL/Examples/LEVELS_MODEL/wrf_Cloud.htm
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 

;   Example script to produce plots for a WRF real-data run,
;   with the ARW coordinate dynamics option.

load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/wrf/WRFUserARW.ncl"
;load "./WRFUserARW.ncl"
load "$VAPOR_HOME/share/examples/NCL/wrf2geotiff.ncl"


begin
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Add your own data here....
;
;
; The WRF ARW input file.  
; This needs to have a ".nc" appended, so just do it.
  ;;;;a = addfile("./wrfout_d03_2008-09-14_18:00:00.nc","r")            
  wrffiles = systemfunc("ls ./HurricaneIKE/wrfout_d03_2008*")
  numFiles = dimsizes(wrffiles)
  do i=0,numFiles-1
    wrffiles(i) = wrffiles(i) + ".nc"
  end do
  inpFiles = addfiles(wrffiles,"r")

  ; Do we want the Geotiffs cropped?
  if (.not.isdefined("cropPlot")) then
      cropPlot = True
  end if
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Output type must be postscript...
  type = "ps"    
  wks = gsn_open_wks(type,"plt_Cloud")
  gsn_define_colormap(wks,"WhBlGrYeRe")   ; overwrite the .hluresfile color map

  ; initialize our tiff-capture process...
  wrf2gtiff = wrf2geotiff_open(wks)

; Set some basic resources
  res = True
  res@MainTitle = "REAL-TIME WRF"

  mpres  = True  ; Map resources
  pltres = True ; Plot resources

  pltres@FramePlot = False   ; Extremely important for geotiff output process

; Loop over files...
do ifile = 0, numFiles-1
  a = inpFiles[ifile]
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; What times and how many time steps are in the data set?
  times  = wrf_user_list_times(a)  ; get times in the file
  ntimes = dimsizes(times)         ; number of times in the file

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  do it = ntimes-1,ntimes-1        ; TIME LOOP

    print("Working on time: " + times(it) )
    res@TimeLabel = times(it)   ; Set Valid time to use on plots

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; First get the variables we will need        

    ;; original script generated several different plots per timestep

    if(isfilevar(a,"QVAPOR"))
      qv = wrf_user_getvar(a,"QVAPOR",it) 
      qv = qv*1000.
      qv@units = "g/kg"   
    end if

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;; original script looped over levels
    ;;;;do level = 0,24,5      ; LOOP OVER LEVELS
      level = 4

      display_level = level + 1
      opts = res
      opts@cnFillOn         = True
      opts@gsnSpreadColors  = False
      opts@PlotLevelID      = "Eta Level  " + display_level

      if (isvar("qv"))
        contour = wrf_contour(a,wks,qv(level,:,:),opts)
        plot = wrf_map_overlays(a,wks,(/contour/),pltres,mpres)
        wrf2geotiff_write(wrf2gtiff, a, times(it), wks, plot, cropPlot)
        frame(wks)  ; Now that we are done drawing, draw the frame
        delete(contour)
      end if

      delete(opts)

    ;;;;end do      ; END OF LEVEL LOOP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

  end do        ; END OF TIME LOOP     
end do          ; END OF FILE LOOP

wrf2geotiff_close(wrf2gtiff, wks)
end
