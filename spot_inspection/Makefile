##############################################
# OpenWrt Makefile for spot_inspection program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
# Name and release number of this package
PKG_NAME:=spot_inspection
PKG_RELEASE:=1.0.3


# This specifies the directory where we're going to build the program. 
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk
 
# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/spot_inspection
	SECTION:=utils
	CATEGORY:=Utilities
#	DEPENDS:=+libpthread
	TITLE:=spot_inspection -- for system use
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/spot_inspection/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef

define Package/spot_inspection/conffiles
	/etc/config/spot_inspection
endef

 
# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the spot_inspection executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/spot_inspection/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/spot_inspection $(1)/bin/

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./etc/* $(1)/etc/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./init.d/* $(1)/etc/init.d/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./config/* $(1)/etc/config/

	$(INSTALL_DIR) $(1)/root
	$(INSTALL_BIN) ./sh/*.sh $(1)/root/

	#$(INSTALL_DIR) $(1)/etc/rc.d
	#$(INSTALL_BIN) ./rc.d/* $(1)/etc/rc.d

endef


define Package/$(PKG_NAME)/postinst
#!/bin/sh 
[ -n "${IPKG_INSTROOT}" ] || {
	/etc/init.d/spot_inspection enable
	/etc/init.d/spot_inspection start
	touch /root/success
	echo starting the app ...
	exit 0
}
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,spot_inspection))