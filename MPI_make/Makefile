
test.exe : main.o MPI_dijkstra.o
	mpiicpx main.o MPI_dijkstra.o -o test.exe


MPI_dijkstra.o : MPI_dijkstra.cpp MPI_dijkstra.h
	mpiicpx -c MPI_dijkstra.cpp

main.o : main.cpp MPI_dijkstra.h
	mpiicpx -c main.cpp

clean:
	rm *.o *.exe