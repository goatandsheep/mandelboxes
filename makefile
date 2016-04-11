
PROGRAM_NAME=mandelbox

mandelbox:
	# will run on all cores, for MPI compile from mpihost01 and "make mpi"
	make clean
	make -f makefile_ompserv
omp49: # uses gcc4.9 - for local OS X running
	make clean
	make -f makefile_omp
omp: # compiles on mcmaster servers
	make clean
	make -f makefile_ompserv
mpi: # compiles on mpihost01 and distributes to sockets, also uses openMP
	make clean
	make -f makefile_ompservmpi
serial: # uses gcc4.9 - for local OS X running
	make clean
	make -f makefile_serial
mandelbulb: # compiles on tesla
	make clean
	make -f makefile_mandelbulb


test:
	rm -rf _images
	time ./mandelbox params2.dat
testmpi:
	# --map-by socket:PE=1:none  (use for openACC)
	rm -rf _images
	time mpirun --map-by socket:PE=4:none -hostfile host_file ./mandelbox para

bench:
	python -m timeit -n 3 -r 1 "__import__('os').system('./mandelbox params2.dat')"
benchmpi:
	python -m timeit -n 3 -r 1 "__import__('os').system('mpirun --map-by socket:PE=4 -hostfile host_file ./mandelbox para')"

erik: # for my own use
	make ompservmpi
	make testmpi
	make video
	open "_videos/output.mp4"


video:
	ffmpeg -y -r 30 -i "_images/image%010d.bmp"  -c:v libx264 -crf 0 -preset ultrafast "_videos/output.mp4"
	open "_videos/output.mp4"
video_hq:
	ffmpeg -y -r 30 -i "_images/image%010d.bmp"  -c:v libx264 -preset slow -tune animation -crf 18 -c:a copy "_videos/output.mp4"
	open images/output.mp4


clean:
	rm -f *.o $(PROGRAM_NAME) $(EXEEXT) *~
