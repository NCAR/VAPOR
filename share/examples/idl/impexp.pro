; $Id$
;
; This example shows how to importion a region of data from vaporgui,
; perform an operation on that region, rename and export the region back
; to vaporgui
;

;
; Import a region of data previously exported by vaporgui
;
region = impregion(STATEINFO=stateinfo)

;
; Perform an operation on the imported data - in this example we 
; apply a simple algebraic operator
;
region = region * region

;
; Finally, export the region to a new VDC, named 'impexp.vdf'. The new
; variable we have created will be named 'varsqr'. The new VDC maybe
; either import into a running vaporgui session from whence the original
; data came, or may simply be loaded as a new data set
;
tmpvdf = 'impexp.vdf'
varname = 'varsqr'
expregion, tmpvdf, stateinfo, varname, region


end
