TOP = ../../..

include ${TOP}/make/config/prebase.mk

IDL_EXAMPLES = AddMagVDF.pro AddCurlVDF.pro PrintMetaVDF.pro \
	curl_findiff.pro deriv_findiff.pro pencil2vapor.pro WRFVortMagEx.pro\
	wrf_curl_findiff.pro AddWRFCurl.pro AddWRFETH.pro elev_deriv.pro \
	MakeLinCmb.pro MakeCmbo.pro ReadRegionVDF.pro ReadVDF.pro \
	WriteTimeVaryVDF.pro WriteVDF.pro expregion.pro impexp.pro \
	impregion.pro marschner_lobb.pro QuickStartEx1.pro myderiv.pro \
	AddDivVDF.pro AddWRFDiv.pro wrf_div_findiff.pro div_findiff.pro


INSTALL_IDL_EX_DIR = $(INSTALL_SHAREDIR)/examples/idl

include ${TOP}/make/config/base.mk

install:: 
	@$(ECHO) "Installing IDL example files $(IDL_EXAMPLES) in $(INSTALL_IDL_EX_DIR)."
	@if test ! -d $(INSTALL_IDL_EX_DIR); then $(MKDIR) $(INSTALL_IDL_EX_DIR); fi
	@for i in $(IDL_EXAMPLES); do $(INSTALL_NONEXEC) $$i $(INSTALL_IDL_EX_DIR); done
