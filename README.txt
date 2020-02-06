The shared drive
====
Anything you put here will be shared across all nodes, that means when you compile files on pc0 the executables will appear on all of the
nodes in the cluster so you don't have to use ClusterSSH, upload the source to each one, and compile on each one. You only have to do things
on pc0 and make sure you're in this shared folder.

mpirun -n 4 -mca btl_tcp_if_include eth0 -host pc1:1,pc2:1,pc3:1,pc4:1 sum_mpi
