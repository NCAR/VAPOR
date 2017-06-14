TOP = ../..

include $(TOP)/make/config/prebase.mk


INSTALL_SHADERS_DIR = $(INSTALL_SHAREDIR)/shaders
INSTALL_SHADER_INCLUDES_DIR = $(INSTALL_SHADERS_DIR)/includes
INSTALL_SHADER_MAINS_DIR = $(INSTALL_SHADERS_DIR)/main

EFFECTS = $(wildcard *.efc)
SHADER_INCLUDES = $(wildcard includes/*.hgl)
SHADER_MAINS = $(wildcard main/*.vgl)
SHADER_MAINS += $(wildcard main/*.fgl)

include $(TOP)/make/config/base.mk

install:: 
	@$(ECHO) "Installing shader effect files $(EFFECTS) in $(INSTALL_SHADERS_DIR)."
	@if test ! -d $(INSTALL_SHADERS_DIR); then $(MKDIR) $(INSTALL_SHADERS_DIR); fi
	@for i in $(EFFECTS); do $(INSTALL_NONEXEC) $$i $(INSTALL_SHADERS_DIR); done
	@$(ECHO) "Installing shader include files $(SHADER_INCLUDES) in $(INSTALL_SHADER_INCLUDES_DIR)."
	@if test ! -d $(INSTALL_SHADER_INCLUDES_DIR); then $(MKDIR) $(INSTALL_SHADER_INCLUDES_DIR); fi
	@for i in $(SHADER_INCLUDES); do $(INSTALL_NONEXEC) $$i $(INSTALL_SHADER_INCLUDES_DIR); done
	@$(ECHO) "Installing shader main files $(SHADER_MAINS) in $(INSTALL_SHADER_MAINS_DIR)."
	@if test ! -d $(INSTALL_SHADER_MAINS_DIR); then $(MKDIR) $(INSTALL_SHADER_MAINS_DIR); fi
	@for i in $(SHADER_MAINS); do $(INSTALL_NONEXEC) $$i $(INSTALL_SHADER_MAINS_DIR); done
