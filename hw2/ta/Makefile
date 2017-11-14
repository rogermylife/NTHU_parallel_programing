CC = mpicc
CXX = mpicxx
LDFLAGS = -lpng
CFLAGS = -O3 -std=gnu99
CXXFLAGS = -O3 -std=gnu++11
TARGETS = ms_seq ms_mpi_static ms_mpi_dynamic ms_omp ms_hybrid

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
