export BUILDDIR = ${TARGET_PREFIX}

SUBDIRS = ${BENCH_CLASS}

LDFLAGS  =
LIBOBJS =

ifneq ($(strip $(TARGET_SYSTEM_KSP_OS)),)
	LIBOBJS += $(BUILDDIR)/$(TARGET_SYSTEM_KSP_OS)/*.o
endif

ifneq ($(strip $(TARGET_SYSTEM_KSP_TASK)),)
	LIBOBJS += $(BUILDDIR)/$(TARGET_SYSTEM_KSP_TASK)/*.o
endif

ifneq ($(strip $(TARGET_SYSTEM_ASP_COM)),)
	LIBOBJS += $(BUILDDIR)/$(TARGET_SYSTEM_ASP_COM)/*.o
endif

ifneq ($(strip $(TARGET_SYSTEM_SSP_CAL)),)
	LIBOBJS += $(BUILDDIR)/$(TARGET_SYSTEM_SSP_CAL)/*.o
endif

ifneq ($(strip $(TARGET_LDSCRIPT)),)
	LDFLAGS += $(TARGET_LDSCRIPT)
endif

NAME = ${TARGET_PREFIX}.x

default: barnes ocean water-spatial water-nsquared

#install_check apes apes_submake binary

#install_check:
#	@if [ -z $(APES_HOME) ] ; then \
#		echo '[ERROR  ] The APES environement has not been installed';\
		false;\
	fi;

apes: 
	make -C $(APES_HOME)
	echo '[BUILD...] $(NAME)'

barnes:
	echo "Compiling Barnes"
	. apps/barnes/install.sh && make -C apps/barnes
   
ocean:
	echo "Compiling Ocean"
	. apps/ocean/contiguous_partitions/install.sh && make -C apps/ocean/contiguous_partitions

water-spatial:
	echo "Compiling Water-Spatial"
	. apps/water-spatial/install.sh &&	make -C apps/water-spatial

water-nsquared:
	echo "Compiling Water-NSquared"
	. apps/water-squared/install.sh &&	make -C apps/water-squared

clean: install_check apes_clean 
	make -C $(APES_HOME) clean
	make -C $(SUBDIRS) app_clean
	echo '[CLEAN...] $(NAME)'
	rm -rf ${NAME}

app_clean:$ install_check
	make -C $(SUBDIRS) app_clean
	rm -f $(BUILDDIR)/$(APP_NAME)/$(APP_NAME_EXT)/*
	echo '[CLEAN...] $(APP_NAME)/$(APP_NAME_EXT) [o]'
	
binary : 
	$(TARGET_LD) -o $(NAME) $(LIBOBJS) $(BUILDDIR)/$(APP_NAME)/$(APP_NAME_EXT)/*.o $(TARGET_LDFLAGS) $(LDFLAGS) 

ifneq ($(strip $(APES_HOME)),)
include $(APES_HOME)/support/Makefile.rules
endif


