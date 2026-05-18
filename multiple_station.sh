mpirun -machinefile ./machinefile --mca orte_keep_fqdn_hostnames t --mca btl_tcp_if_exclude docker0,docker_gwbridge,lo -np 3 ./openmp+mpi 1000000 L7 4
