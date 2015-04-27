# options are "optimize" and "debug"
BUILD := optimize

# options are "sse2"
PLATFORM_CPU := sse2

# options are "sm_13"
PLATFORM_GPU := sm_52

# options are "32" and "64"
BITS := 64

AR   := ar
MAKE := make
CC   := gcc
CXX  := g++
LD   := g++
NVCC := nvcc
MEX  := mex

SUBDIRS := \
	lib \
	bin


#====  platform-dependent  ====================================================

DEFINE_FLAGS :=

COMPILER_FLAGS          := -Wall -fPIC -D_GNU_SOURCE -m$(BITS) $(DEFINE_FLAGS)
COMPILER_DEBUG_FLAGS    :=
COMPILER_OPTIMIZE_FLAGS := -funroll-loops -fomit-frame-pointer -ffast-math

# nvcc has a "-use_fast_math" option, which seems to hurt accuracy more than it improves speed
NVCC_FLAGS          := --maxrregcount=32 --ptxas-options=-v -m$(BITS) $(DEFINE_FLAGS)
NVCC_DEBUG_FLAGS    :=
NVCC_OPTIMIZE_FLAGS :=

MEX_FLAGS          :=
MEX_DEBUG_FLAGS    :=
MEX_OPTIMIZE_FLAGS :=

ifeq ($(PLATFORM_CPU),sse2)
COMPILER_FLAGS := $(COMPILER_FLAGS) -msse2 -mfpmath=sse
endif

ifeq ($(PLATFORM_GPU),sm_13)
DEFINE_FLAGS := $(DEFINE_FLAGS) -DCUDA_USE_DOUBLE
NVCC_FLAGS := $(NVCC_FLAGS) -arch sm_13
else ifneq  ($(PLATFORM_GPU),)
NVCC_FLAGS := $(NVCC_FLAGS) -arch $(PLATFORM_GPU)
endif


ifeq ($(BITS),32)
MEX_EXTENSION := mexglx
MEX_FLAGS := $(MEX_FLAGS) -glnx86
endif

ifeq ($(BITS),64)
MEX_EXTENSION := mexa64
MEX_FLAGS := $(MEX_FLAGS) -glnxa64 -largeArrayDims
endif


#====  platform-independent  ==================================================

COMMA := ,
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)

ifeq ($(BUILD),debug)
LINKER_FLAGS :=
COMPILER_FLAGS := \
	-g3 \
	$(COMPILER_DEBUG_FLAGS) \
	$(COMPILER_FLAGS)
NVCC_FLAGS := \
	-g -G \
	--compiler-options ${subst $(SPACE),$(COMMA),${strip -fpermissive $(COMPILER_FLAGS) $(COMPILER_DEBUG_FLAGS)}} \
	$(NVCC_DEBUG_FLAGS) \
	$(NVCC_FLAGS)
MEX_FLAGS := \
	-g \
	$(MEX_DEBUG_FLAGS) \
	$(MEX_FLAGS)
endif

ifeq ($(BUILD),optimize)
LINKER_FLAGS :=
COMPILER_FLAGS := \
	-O2 \
	$(COMPILER_OPTIMIZE_FLAGS) \
	$(COMPILER_FLAGS)
NVCC_FLAGS := \
	-O2 \
	--compiler-options ${subst $(SPACE),$(COMMA),${strip -fpermissive $(COMPILER_FLAGS) $(COMPILER_OPTIMIZE_FLAGS)}} \
	$(NVCC_OPTIMIZE_FLAGS) \
	$(NVCC_FLAGS)
MEX_FLAGS := \
	-O \
	$(MEX_OPTIMIZE_FLAGS) \
	$(MEX_FLAGS)
endif

LDFLAGS := ${strip $(LINKER_FLAGS)}
CFLAGS := ${strip -std=gnu99 $(COMPILER_FLAGS)}
CXXFLAGS := ${strip $(COMPILER_FLAGS)}
NVCCFLAGS := ${strip $(NVCC_FLAGS)}
MEXFLAGS := ${strip $(MEX_FLAGS)}

INNER_MAKEFLAGS := \
	"AR=$(AR)" \
	"MAKE=$(MAKE)" \
	"LD=$(LD)" \
	"CC=$(CC)" \
	"CXX=$(CXX)" \
	"NVCC=$(NVCC)" \
	"MEX=$(MEX)" \
	"LDFLAGS=$(LDFLAGS)" \
	"CFLAGS=$(CFLAGS)" \
	"CXXFLAGS=$(CXXFLAGS)" \
	"NVCCFLAGS=$(NVCCFLAGS)" \
	"MEXFLAGS=$(MEXFLAGS)" \
	"MEX_EXTENSION=$(MEX_EXTENSION)"


#====  compilation rules  =====================================================

.PHONY : all
all:
	@for ii in $(SUBDIRS) ; do \
		( cd $$ii ; $(MAKE) $(MAKEFLAGS) $(INNER_MAKEFLAGS) all ) ; \
	done

.PHONY : clean
clean :
	@for ii in $(SUBDIRS) ; do \
		( cd $$ii ; $(MAKE) $(MAKEFLAGS) $(INNER_MAKEFLAGS) clean ) ; \
	done
