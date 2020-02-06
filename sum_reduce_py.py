from mpi4py import MPI
import numpy as np
import sys
import time

comm = MPI.COMM_WORLD

# get number of processes
size = comm.Get_size()

# get rank of current process
rank = comm.Get_rank()

if rank == 0:
    start_time = time.time()

min = 1
# must be smaller than 9223372036854775807 (sys.maxsize)
max = 100000000

if rank == 0:
    print('\nThis program will add every integer between ' + str(min) + ' and ' + str(max)+ ' using ' + str(size) + ' processes\n')

totalSum = 0

totalRange = max - min + 1
procRange = totalRange / size


procMin = int((procRange * rank) + min)
procMax = int(procMin + procRange - 1)

# initalize numpy array to store total sum
totalSum = np.array(0.0,'d')

if rank == (size-1):
    sum = 0
    # loop from values from procMin to max
    for x in range(procMin, max+1):
        sum = sum + x
    value_sum = np.array(sum,'d')
else:
    sum = 0
    for x in range (procMin, procMax+1):
        sum = sum + x
    value_sum = np.array(sum,'d')

print('Process ' + str(rank) + ': My sum contribution is ' + str(sum) + '.\n')

comm.Reduce(value_sum, totalSum, op=MPI.SUM, root=0)
#comm.Reduce([sum, MPI.DOUBLE], [totalSum, MPI.DOUBLE], op=MPI.SUM, root=0)

if rank == 0:
    print('Master process\n')
    print('Total sum: ' + str(totalSum) + '\n')
    print("--- %s seconds for program execution ---" % (time.time() - start_time))
