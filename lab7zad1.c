#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define RESERVE 500
#define DEPOT 1
#define START 2
#define ROUTE 3
#define END_ROUTE 4
#define CRASH 5
#define REFUEL 5000

int fuel = 5000;
int STAY = 1, D_STAY = 0;
int processNumber;
int processNr;
int numberOfBus;
int numberOfWays = 4;
int numberOfWaysTaken = 0;
int tag = 1;
int send[2];
int take[2];
MPI_Status mpi_status;

void Send(int busNumber, int state)
{
    send[0] = busNumber;
    send[1] = state;
    MPI_Send(&send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Depot(int processNumber)
{
    int busNumber, status;
    numberOfBus = processNumber - 1;

    printf("Have a nice trip \n");
    printf("We have %d routes\n", numberOfWays);
    sleep(2);

    while(numberOfWays <= numberOfBus)
        {
            MPI_Recv(&take, 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);
            busNumber = take[0];
            status = take[1];
            if(status == 1)
            {
                printf("The bus %d is at the depot\n", busNumber);
            }
            if(status == 2)
            {
                printf("The bus %d goes on route number %d\n", busNumber, numberOfWaysTaken);
                numberOfWaysTaken--;
            }
            if(status == 3)
            {
                printf("Bus %d on the road\n", busNumber);
            }
            if(status == 4)
            {
                if(numberOfWaysTaken < numberOfWays)
                {
                    numberOfWaysTaken++;
                    MPI_Send(&STAY, 1, MPI_INT, busNumber, tag, MPI_COMM_WORLD);
                }
                else
                {
                    MPI_Send(&D_STAY, 1, MPI_INT, busNumber, tag, MPI_COMM_WORLD);
                }
            }
            if(status == 5)
            {
                numberOfBus--;
                printf("Number of buses %d\n", numberOfBus);
            }
        }
    printf("End of Watch \n");
}

void Bus()
{
    int state, sum, i;
    state = ROUTE;
    while(1)
    {
        if(state == 1)
        {
            if(rand() % 2 == 1)
            {
                state = START;
                fuel = REFUEL;
                printf("Asking for permission to go on tour, bus %d", processNr);
                printf("\n");
                Send(processNr, state);
            }
            else
            {
                Send(processNr, state);
            }
        }
        else if(state == 2)
        {
            printf("Going on tour, bus %d\n", processNr);
            state = ROUTE;
            Send(processNr,state);
        }
        else if(state == 3)
        {
            fuel -= rand() % 500;
            if(fuel <= RESERVE)
            {
                state = END_ROUTE;
                printf("Asking for permission to leave the depot\n");
                Send(processNr,state);
            }

            else
            {
                for(i = 0; rand() % 10000; i++);
            }
        }
        else if(state == 4)
        {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp == STAY)
            {
                state = DEPOT;
                printf("Tour finished, bus %d\n", processNr);
            }
            else
            {
                fuel -= rand() % 500;
                if(fuel > 0)
                {
                    Send(processNr, state);
                }
                else
                {
                    state = CRASH;
                    printf("Bus crash\n");
                    Send(processNr, state);
                return;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processNr);
    MPI_Comm_size(MPI_COMM_WORLD, &processNumber);
    srand(time(NULL));
    if(processNr == 0)
    Depot(processNumber);
    else
    Bus();
    MPI_Finalize();
    return 0;
}
