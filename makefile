FLAGS=-acc -fast -ta=tesla,cc35 -Minfo -Minform=inform -O3
CFLAGS  = $(FLAGS)
CXXFLAGS= $(FLAGS)
CC=pgCC
CXX=pgCC


PROGRAM_NAME=mandelbox

$(PROGRAM_NAME): main.o print.o timing.o savebmp.o getparams.o getcolor.o \
	mandelboxde.o raymarching.o renderer.o 3d.o distance_est.o init3D.o
	$(CC) -o $@ $? $(CFLAGS) $(LDFLAGS)

omp:
	clean
	make -f makefile_omp
serial:
	clean
	make -f makefile_serial

test:
	./mandelbox params2.dat
bench:
	python -m timeit -n 5 -r 2 "__import__('os').system('./mandelbox params2.dat')"

clean:
	rm *.o $(PROGRAM_NAME) $(EXEEXT) *~
