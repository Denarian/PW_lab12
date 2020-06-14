#include <mpi.h>
#include "pbm.h"

int main(int argc, char *argv[]) {
    image in;
    int npes, myrank;
	double start, end;

	int f[9] = {0,-1,0,//f  - filtr
				-1,5,-1,
				0,-1,0};
/*
    int f[9] = {0,0,0,//f  - filtr
				0,1,0,
				0,0,0};*/
	MPI_Init(&argc, &argv);

	MPI_Status status;
	
	MPI_Comm_size(MPI_COMM_WORLD, &npes);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if(myrank == 0)
		readInput(argv[1], &in);

	MPI_Bcast( &in.width, 2, MPI_INT,0, MPI_COMM_WORLD );

	int range = in.height / npes; // ilosc wierszy dla pojedynczego procesu poza ostatnim
	int range_last = in.height - (range * (npes - 1));
	int el_cnt[npes]; // ilosc elementow dla poszczegolnych procesów
	int disp[npes];// odstepy do wysylania
	
	for(int i = 0; i < npes - 1; i++)// wypelnienie tablicy zawierajacej ilosci elementow dla poszczegolnych procesow 
		el_cnt[i] = range * in.width;
	el_cnt[npes - 1] = range_last * in.width;

	if(myrank == 0){
		for(int i = 0; i < npes; i++)// wypelnienie tablicy offsetow(od ktrego miejsc ma wyslac do poszczegolnych procesow)
				disp[i] = i * range * in.width;
	}

 	uchar * out_pixel = (uchar*) malloc(sizeof(uchar) * el_cnt[myrank]);
	uchar * in_pixel = (uchar*) malloc(sizeof(uchar) * el_cnt[myrank]);
  
	MPI_Scatterv(in.pixel,el_cnt, disp ,MPI_CHAR, in_pixel, el_cnt[myrank], MPI_CHAR, 0, MPI_COMM_WORLD);
	
	uchar * ex_pixels = (uchar*) malloc(sizeof(uchar) * in.width * 2); // dodatkwe wiersze

	if(myrank == 0)
	{
		memcpy(ex_pixels + in.width,in.pixel + range * in.width,in.width);// uchar ma 1 bajt więc szerokość obrazu jest zarazem ilością bajtów w lini 
		int i;
		for(i = 1; i < npes - 1; i++)
		{
			MPI_Send(in.pixel + i * range * in.width - in.width , in.width, MPI_CHAR, i, 10, MPI_COMM_WORLD);
			MPI_Send(in.pixel + (i+1) * range * in.width, in.width, MPI_CHAR, i, 11, MPI_COMM_WORLD);
		}
		MPI_Send(in.pixel + i * range * in.width - in.width , in.width, MPI_CHAR, i, 10, MPI_COMM_WORLD);

	}
	else
	{
		MPI_Recv(ex_pixels, in.width, MPI_CHAR, 0, 10, MPI_COMM_WORLD, &status);
		if(myrank < (npes - 1))
		 	MPI_Recv(ex_pixels + in.width, in.width, MPI_CHAR, 0, 11, MPI_COMM_WORLD, &status);
	}

	int x  = myrank * range;//x numer aktualnego wiersza(absolutny)
	if(myrank == (npes - 1)) range = range_last;	
	start = MPI_Wtime();
	for(int i = 0 ; i < range; i++,x++)
	{
		if((myrank == npes - 1 &&  x == in.height - 1) || (x == 0 && myrank == 0)) // pierwszy i ostatni wiersz obrazu pominięty 
			continue;
	
		for(int j = 1; j < in.width-1; j++)// pierwsza i ostatnia kolumna obrazu pominieta.
		{
			int s = 0, div = 0;
			uchar * img;
			for(int k = 0; k < 3; k++)//3 wysokosc filtra
			{
				if(i == 0)//procedura dla górnej krawędzi obszaru
				{
					switch(k)
					{	
						case 0:
							img = &ex_pixels[j-1];// pierwszy dodatkowy wiersz
							break;
						case 1:
							img = &in_pixel[i * in.width + j];//pozycja aktualna
							img -= 1;
							break;
						default:
							img += in.width;
							break;		
					}
				}
				else if(i == (range - 1))//procedura dla dolnej krawędzi obszaru
				{
					switch(k)
					{	
						case 0:
							img = &in_pixel[i * in.width + j];//pozycja aktualna
							img -= in.width + 1;// przesuniecie aby rozpoczac od lewgo górnego rogu filtra
							break;
						case 2:
							img = &ex_pixels[in.width+j-1];// drugi dodatkowy wiersz
							break;	
						default:
							img += in.width;
							break;
															
					}
				}
				else// procedura dla wierszy wewnatrz obszaru	
				{
					switch(k)
					{	
						case 0:
							img = &in_pixel[i * in.width + j];//pozycja aktualna
							img -= in.width + 1;// przesuniecie aby rozpoczac od lewgo górnego rogu filtra
							break;
						default:
							img += in.width;
							break;
					}
				}
				for(int l = 0; l < 3; l++)//3 szerokosc filtra
				{
					s += img[l] * f[k * 3 + l];
					div += f[k * 3 + l];
				}
			}
			if(s > 255) s = 255;
			else if(s < 0) s = 0;
			out_pixel[i * in.width + j] = s / div;
			
		
		}
	}
	
	MPI_Gatherv(out_pixel ,el_cnt[myrank], MPI_CHAR, in.pixel, el_cnt, disp, MPI_CHAR, 0, MPI_COMM_WORLD);
	end = MPI_Wtime();

	printf("%d done\n", myrank);

	if (myrank == 0) {
        writeData(argv[2], &in);
		printf("%lf\n", end-start);
	}
	MPI_Finalize();
	return 0;
}
