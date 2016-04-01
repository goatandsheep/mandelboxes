FLAGS=-acc -fast -ta=tesla,cc35 -Minfo=accel -Minform=inform -O3
CFLAGS  = $(FLAGS)
CXXFLAGS= $(FLAGS)
CC=pgCC
CXX=pgCC


PROGRAM_NAME=mandelbox

$(PROGRAM_NAME): main.o print.o timing.o savebmp.o getparams.o 3d.o getcolor.o distance_est.o \
	mandelboxde.o raymarching.o renderer.o init3D.o
	$(CC) -o $@ $? $(CFLAGS) $(LDFLAGS)


clean:
	rm *.o $(PROGRAM_NAME) $(EXEEXT) *~
