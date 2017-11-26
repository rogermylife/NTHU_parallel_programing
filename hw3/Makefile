CC = mpicc
CXX = mpicxx
LDFLAGS = -lpng
CFLAGS = -O3 -std=gnu99
CXXFLAGS = -O3 -std=gnu++11
TARGETS = APSP_MPI_sync APSP_MPI_async

.PHONY: all
all: $(TARGETS)

.PHONY: clean
clean:
	rm -f $(TARGETS) $(TARGETS:=.o)
