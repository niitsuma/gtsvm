#====  files  =================================================================

LIBRARY := \
	libgtsvm.a

HEADERS := \
	gtsvm.h \
	svm.hpp \
	cuda.hpp \
	cuda_sparse_kernel.hpp \
	cuda_reduce.hpp \
	cuda_find_largest.hpp \
	cuda_partial_sum.hpp \
	cuda_array.hpp \
	cuda_exception.hpp \
	cuda_helpers.hpp \
	helpers.hpp

SOURCES := \
	gtsvm.cpp \
	svm.cpp \
	cuda_sparse_kernel.cu \
	cuda_reduce.cu \
	cuda_find_largest.cu \
	cuda_partial_sum.cu \
	cuda_array.cu \
	cuda_exception.cpp

PRECOMPILED_HEADER_SOURCE := \
	headers.hpp


#====  derived variables  =====================================================

PRECOMPILED_HEADER := ${PRECOMPILED_HEADER_SOURCE:%.hpp=%.hpp.gch}

OBJECTS := \
	${patsubst %.cpp,%.o,${filter %.cpp,$(SOURCES)}} \
	${patsubst %.c,%.o,${filter %.c,$(SOURCES)}} \
	${patsubst %.cu,%.o,${filter %.cu,$(SOURCES)}}


#====  compilation rules  =====================================================

.PHONY : all
all: $(LIBRARY)

$(LIBRARY) : $(OBJECTS)
	@echo "----  Building \"$@\"  ----"
	$(AR) rcs $@ $(OBJECTS)
	@echo

$(PRECOMPILED_HEADER) : $(PRECOMPILED_HEADER_SOURCE) $(HEADERS)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CXX) $(CXXFLAGS) $<
	@echo

%.o : %.c $(PRECOMPILED_HEADER)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CC) $(CFLAGS) -c $< -o $@
	@echo

%.o : %.cpp $(PRECOMPILED_HEADER)
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo

%.o : %.cu
	@echo "----  Building \"$@\" from \"$<\"  ----"
	$(NVCC) $(NVCCFLAGS) -c $< -o $@
	@echo

.PHONY : clean
clean :
	@echo "----  Cleaning  ----"
	rm -f $(LIBRARY) $(OBJECTS) $(PRECOMPILED_HEADER)
	@echo
