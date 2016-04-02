#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int video()
{
	//called by savejpg.c
    //ffmpeg init
    while()
	{
		MPI_Recv(/*image*/);
		//ffmpeg add image to video
	}
	//ffmpeg finish up, compress, etc.
    MPI_Send(/*video file*/);
}