
#CUDA_INSTALL_PATH ?= /usr/local/cuda
CUDA_INSTALL_PATH ?= /opt/cuda ##gentoo  
#CUDA_SDK_PATH ?= $(HOME)/cuda
CUDA_SDK_PATH ?= $(CUDA_INSTALL_PATH)/sdk  ##gentoo
ARCH := $(shell getconf LONG_BIT)
#LIBDIR     := $(CUDA_INSTALL_PATH)/lib$(ARCH)
ifeq ($(ARCH), 32)
LIBDIR = $(CUDA_INSTALL_PATH)/lib/
else
LIBDIR = $(CUDA_INSTALL_PATH)/lib64/
endif


#====  files  =================================================================

TARGET_SOURCES := \
	gtsvm_initialize.cpp \
	gtsvm_shrink.cpp \
	gtsvm_restart.cpp \
	gtsvm_recalculate.cpp \
	gtsvm_optimize.cpp \
	gtsvm_classify.cpp

HEADERS := \
	auto_context.hpp \
	../lib/gtsvm.h

SOURCES :=

LIBRARIES := \
	../lib/libgtsvm.a

PRECOMPILED_HEADER_SOURCE := \
	headers.hpp

LIBRARY_FLAGS := \
	-lgtsvm \
	-lcudart \
	-lboost_regex \
	-lboost_program_options \
	-L $(LIBDIR)


#====  derived variables  =====================================================

PRECOMPILED_HEADER := ${PRECOMPILED_HEADER_SOURCE:%.hpp=%.hpp.gch}

OBJECTS := \
	${patsubst %.cpp,%.o,${filter %.cpp,$(SOURCES)}} \
	${patsubst %.c,%.o,${filter %.c,$(SOURCES)}}

TARGET_OBJECTS := \
	${patsubst %.cpp,%.o,${filter %.cpp,$(TARGET_SOURCES)}} \
	${patsubst %.c,%.o,${filter %.c,$(TARGET_SOURCES)}}

TARGETS := \
	${patsubst %.cpp,%,${filter %.cpp,$(TARGET_SOURCES)}} \
	${patsubst %.c,%,${filter %.c,$(TARGET_SOURCES)}}


#====  compilation rules  =====================================================

# disable default rules
.SUFFIXES:

.PHONY : all
all: $(TARGETS)

$(PRECOMPILED_HEADER) : $(PRECOMPILED_HEADER_SOURCE) $(HEADERS)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CXX) $(CXXFLAGS) -I../lib $<
	@echo

%.o : %.c $(PRECOMPILED_HEADER)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CC) $(CFLAGS) -I../lib -c $< -o $@
	@echo

%.o : %.cpp $(PRECOMPILED_HEADER)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CXX) $(CXXFLAGS) -I../lib -c $< -o $@
	@echo

% : %.o $(OBJECTS) $(LIBRARIES)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(LD) $(LDFLAGS) -L../lib $< $(OBJECTS) -o $@ $(LIBRARY_FLAGS)
	@echo

.PHONY : clean
clean :
	@echo "----  Cleaning  ----"
	rm -f $(TARGETS) $(TARGET_OBJECTS) $(OBJECTS) $(PRECOMPILED_HEADER)
	@echo
