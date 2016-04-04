#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


// called by savejpg.c
// ffmpeg init

//time convert ka.bmp ka.jpg

int makeVideo(FILE *f)
{
	MPI_Init(NULL, NULL);
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size > 1)
    {
    	//parallelize
    	//makeVideo();

		int number;
		if (world_rank == 0) {
		    number = -1;
		    //it's really just a lot of chars tho
		    MPI_Send(&number, 1, filename, 1, 0, MPI_COMM_WORLD);
		} else if (world_rank == 1) {
		    MPI_Recv(&number, 1, filename, 0, 0, MPI_COMM_WORLD,
		             MPI_STATUS_IGNORE);
		    //receive name of file
		    printf("Process 1 received number %d from process 0\n",
		           number);

		    FILE * f;
		    int errnum;
		    f = fopen(filename, "rb");
		    //if file isn't there, output error and wait a bit, then try again
		    while (f == NULL)
		    {
		    	errnum = errno;
    			fprintf(stderr, "Value of errno: %d\n", errno);
				perror("Error printed by perror");
				fprintf(stderr, "Error opening file: %s\n", strerror( errnum ));
				//wait
				f = fopen(filename, "rb");
		    }
		    //system("ffmpeg bob.jpg");
		    fclose (f);
		    //delete jpg
		    int ret = remove(filename);
		    if(ret != 0)
	    	{
	    		printf("Error: unable to delete the file");
	    	}
	    	//else {don't worry about it}
		}
		MPI_CHAR bye = ""
		/* after it is done:
		MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
		*/
		MPI_Finalize();
    }
    else
    {
		MPI_Finalize();
    	//compute as we go if not enough cores
    }

}