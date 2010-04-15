# these values filled in by    yorick -batch make.i
Y_MAKEDIR=/home/frigaut/yorick-2.1
Y_EXE=/home/frigaut/yorick-2.1/bin/yorick
Y_EXE_PKGS=
Y_EXE_HOME=/home/frigaut/yorick-2.1
Y_EXE_SITE=/home/frigaut/yorick-2.1

# ----------------------------------------------------- optimization flags

# options for make command line, e.g.-   make COPT=-g TGT=exe
COPT=$(COPT_DEFAULT)
TGT=$(DEFAULT_TGT)

# ------------------------------------------------ macros for this package

PKG_NAME=ml4
PKG_I=ml4.i

OBJS=ml4.o

# change to give the executable a name other than yorick
PKG_EXENAME=yorick

# PKG_DEPLIBS=-Lsomedir -lsomelib   for dependencies of this package
PKG_DEPLIBS=
# set compiler (or rarely loader) flags specific to this package
PKG_CFLAGS=
PKG_LDFLAGS=

# list of additional package names you want in PKG_EXENAME
# (typically Y_EXE_PKGS should be first here)
EXTRA_PKGS=$(Y_EXE_PKGS)

# list of additional files for clean
PKG_CLEAN=

# autoload file for this package, if any
PKG_I_START=ml4_start.i
# non-pkg.i include files for this package, if any
PKG_I_EXTRA=

# -------------------------------- standard targets and rules (in Makepkg)

# set macros Makepkg uses in target and dependency names
# DLL_TARGETS, LIB_TARGETS, EXE_TARGETS
# are any additional targets (defined below) prerequisite to
# the plugin library, archive library, and executable, respectively
PKG_I_DEPS=$(PKG_I)
Y_DISTMAKE=distmake

include $(Y_MAKEDIR)/Make.cfg
include $(Y_MAKEDIR)/Makepkg
include $(Y_MAKEDIR)/Make$(TGT)

# override macros Makepkg sets for rules and other macros
# Y_HOME and Y_SITE in Make.cfg may not be correct (e.g.- relocatable)
Y_HOME=$(Y_EXE_HOME)
Y_SITE=$(Y_EXE_SITE)

# reduce chance of yorick-1.5 corrupting this Makefile
MAKE_TEMPLATE = protect-against-1.5

# ------------------------------------- targets and rules for this package

# simple example:
#myfunc.o: myapi.h
# more complex example (also consider using PKG_CFLAGS above):
#myfunc.o: myapi.h myfunc.c
#	$(CC) $(CPPFLAGS) $(CFLAGS) -DMY_SWITCH -o $@ -c myfunc.c

ml4.o: ml4.h

clean::
	-rm -rf binaries

# -------------------------------------------------------- end of Makefile


# for the binary package production (add full path to lib*.a below):
PKG_DEPLIBS_STATIC=-lm 
PKG_ARCH = $(OSTYPE)-$(MACHTYPE)
PKG_VERSION = $(shell (awk '{if ($$1=="Version:") print $$2}' $(PKG_NAME).info))
# .info might not exist, in which case he line above will exit in error.

# packages or devel_pkgs:
PKG_DEST_URL = packages

package:
	$(MAKE)
	$(LD_DLL) -o $(PKG_NAME).so $(OBJS) ywrap.o $(PKG_DEPLIBS_STATIC) $(DLL_DEF)
	mkdir -p binaries/$(PKG_NAME)/dist/y_home/lib
	mkdir -p binaries/$(PKG_NAME)/dist/y_home/i-start
	mkdir -p binaries/$(PKG_NAME)/dist/y_site/i0
	cp -p $(PKG_I) binaries/$(PKG_NAME)/dist/y_site/i0/
	cp -p $(PKG_NAME).so binaries/$(PKG_NAME)/dist/y_home/lib/
	if test -f "check.i"; then cp -p check.i binaries/$(PKG_NAME)/.; fi
	if test -n "$(PKG_I_START)"; then cp -p $(PKG_I_START) \
	  binaries/$(PKG_NAME)/dist/y_home/i-start/; fi
	cat $(PKG_NAME).info | sed -e 's/OS:/OS: $(PKG_ARCH)/' > tmp.info
	mv tmp.info binaries/$(PKG_NAME)/$(PKG_NAME).info
	cd binaries; tar zcvf $(PKG_NAME)-$(PKG_VERSION)-$(PKG_ARCH).tgz $(PKG_NAME)

distbin: package
	if test -f "binaries/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_ARCH).tgz" ; then \
	  ncftpput -f $(HOME)/.ncftp/maumae www/yorick/$(PKG_DEST_URL)/$(PKG_ARCH)/tarballs/ \
	  binaries/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_ARCH).tgz; fi
	if test -f "binaries/$(PKG_NAME)/$(PKG_NAME).info" ; then \
	  ncftpput -f $(HOME)/.ncftp/maumae www/yorick/$(PKG_DEST_URL)/$(PKG_ARCH)/info/ \
	  binaries/$(PKG_NAME)/$(PKG_NAME).info; fi

distsrc: clean
	cd ..; tar --exclude binaries --exclude .svn --exclude CVS --exclude *.spec -zcvf \
	   $(PKG_NAME)-$(PKG_VERSION)-src.tgz yorick-$(PKG_NAME)-$(PKG_VERSION);\
	ncftpput -f $(HOME)/.ncftp/maumae www/yorick/$(PKG_DEST_URL)/src/ \
	   $(PKG_NAME)-$(PKG_VERSION)-src.tgz
	ncftpput -f $(HOME)/.ncftp/maumae www/yorick/contrib/ \
	   ../$(PKG_NAME)-$(PKG_VERSION)-src.tgz


# -------------------------------------------------------- end of Makefile
