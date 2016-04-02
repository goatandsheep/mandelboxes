/*
   This file is part of the Mandelbox program developed for the course
    CS/SE  Distributed Computer Systems taught by N. Nedialkov in the
    Winter of 2015-2016 at McMaster University.

    Copyright (C) 2015-2016 T. Gwosdz and N. Nedialkov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <mpi.h>

void saveJPG(const char* filename, const unsigned char* result, int w, int h){

	MPI_Init(NULL, NULL);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	FILE *f;
	unsigned char *img = NULL;
	int filesize = 54 + 3*w*h;  //w is your image width, h is image height, both int

	unsigned char jpgfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char jpginfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char jpgpad[3] = {0,0,0};

	jpgfileheader[ 2] = (unsigned char)(filesize    );
	jpgfileheader[ 3] = (unsigned char)(filesize>> 8);
	jpgfileheader[ 4] = (unsigned char)(filesize>>16);
	jpgfileheader[ 5] = (unsigned char)(filesize>>24);

	jpginfoheader[ 4] = (unsigned char)(       w    );
	jpginfoheader[ 5] = (unsigned char)(       w>> 8);
	jpginfoheader[ 6] = (unsigned char)(       w>>16);
	jpginfoheader[ 7] = (unsigned char)(       w>>24);
	jpginfoheader[ 8] = (unsigned char)(       h    );
	jpginfoheader[ 9] = (unsigned char)(       h>> 8);
	jpginfoheader[10] = (unsigned char)(       h>>16);
	jpginfoheader[11] = (unsigned char)(       h>>24);

	f = fopen(filename,"wb");
	fwrite(jpgfileheader,1,14,f);
	fwrite(jpginfoheader,1,40,f);

	
	img = (unsigned char *)malloc(3*w);
	assert(img);

	int i,j;
	for(j=0; j<h; j++)
	{
	    for(i=0; i<w; i++)
		{
            img[i*3+0] = result[(j*w+i)*3+0];
            img[i*3+1] = result[(j*w+i)*3+1];
            img[i*3+2] = result[(j*w+i)*3+2];
		}
		fwrite(img,3,w,f);
	    fwrite(jpgpad,1,(4-(w*3)%4)%4,f);
	}
	fclose(f);
	MPI_Finalize();
}
