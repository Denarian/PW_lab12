#include <mpi.h>
#include "pbm.h"

typedef struct 
{
	int width;
	int height;
	int *tab;
}filter;

int applyFilter(int width, uchar * img, filter f)
{
	int s = 0, div = 0;
	img -= width + f.width / 2;
	for(int i = 0; i < f.height; i++)
	{
		for(int j = 0; j < f.width; j++)
		{
			s += img[j] * f.tab[i * f.width + j];
			div += f.tab[i * f.width + j];
		}
		img += width;
	}
	if(s > 255) s = 255;
	else if(s < 0) s = 0;
	return s / div;
}


int main(int argc, char *argv[]) {
    image in, out;
    int npes, myrank;
	double start, end;
  
	filter f = {3,3,malloc(sizeof(int)* 3 * 3)};
	int temp[] = {0,-1,0,
				-1,5,-1,
				0,-1,0};
	for(int i; i < 9;i++) f.tab[i] = temp[i];
    
	readInput(argv[1], &in);

	MPI_Init(&argc, &argv);
	
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if(myrank == 0)
	{
		out.height = in.height;
		out.width = in.width;
		out.maxValue = in.maxValue;
		memcpy(out.type, in.type, TYPE_LEN+1);

		out.pixel = (uchar*) malloc(sizeof(uchar)*out.height*out.width);
	}

	int range = in.height / npes;
	 
	uchar * temp_pixel = (uchar*) malloc(sizeof(uchar) * in.width * range);

	int x = myrank == 0 ? 1 : myrank*range;//x numer aktualnego wiersza(absolutny)pirwszy wiersz obrazu pominiety

	start = MPI_Wtime();
	if(myrank == npes) range--; //ostatni wiersz obrazu pominiety
	for(int i = 0 ; i < range; i++, x++)
	{
		for(int j = 1; j < in.width-1; j++)// pierwsz i ostatnia kolumna obrazu pominieta.
			temp_pixel[i * in.width + j] = applyFilter(in.width, &in.pixel[x * in.width + j], f);

	}
	end = MPI_Wtime();

	MPI_Gather(temp_pixel, in.width * range, MPI_CHAR, out.pixel, in.width*range, MPI_CHAR, 0, MPI_COMM_WORLD );
	
	
	//printf("%d done\n", myrank);

	if (myrank == 0) {
        writeData(argv[2], &out);
		printf("%lf\n", end-start);
	}
	MPI_Finalize();
	return 0;
}
