FLAGS=-acc -fast -ta=tesla,cc35 -Minfo=accel -Minform=inform -O3
CFLAGS  = $(FLAGS)
CXXFLAGS= $(FLAGS)
CC=pgCC
CXX=pgCC


PROGRAM_NAME=mandelbox

$(PROGRAM_NAME): main.o print.o timing.o savebmp.o getparams.o 3d.o getcolor.o distance_est.o \
	mandelboxde.o raymarching.o renderer.o init3D.o
	make clean
	$(CC) -o $@ $? $(CFLAGS) $(LDFLAGS)

omp:
	make clean
	make -f makefile_omp
serial:
	make clean
	make -f makefile_serial

test:
	# rm -f image2.bmp
	./mandelbox params2.dat
	# open image0000000002.bmp
	# sleep 1
	# rm -f image0000000002.bmp
bench:
	python -m timeit -n 3 -r 1 "__import__('os').system('./mandelbox params2.dat')"

video:
	make test
	ffmpeg -r 20 -i images/image%010d.bmp  -c:v libx264 -preset slow -crf 22 -c:a copy output.mp4
	open images/output.mp4


clean:
	rm -f *.o $(PROGRAM_NAME) $(EXEEXT) *~
