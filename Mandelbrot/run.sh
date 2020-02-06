mpicc mandel_mpi-io.c
mpirun -mca btl_tcp_if_include eth0 -n 24 -host pc1:6,pc2:6,pc3:6,pc4:6 ./a.out
