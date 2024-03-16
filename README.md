# MPI-dijkstra

## Environments
- C++ & Makefile
- Intel(R) Xeon(R) Gold 6330 CPU @ 2.00GHz (28-cores)
- Ubuntu 20.04.5 LTS
- Intel MPI
  - mpiicpx: 2024.0.2 (2024.0.2.20231213)
  - mpirun: Version 2021.11 Build 20231005
- Dataset
  - DIMACS
  - USA-road-d.NY
  - 264,346 vetices, 733,846 edges
- Docker + Kubernetes

## Purpose
대용량 그래프 데이터를 사용하여 dijkstra's algorithm들을 벤치마킹하여 비교하고자 함.<br>
다음과 같은 환경과 알고리즘에서 각각 벤치마킹하였음.
- List-based Dijkstra's Algorithm &rarr; $O(V^2)$
- Priority Queue-Based Dijkstra's Algorithm &rarr; $O(ElogE)$
- List-Based Distributed Dijkstra's Algorithm Implemented with MPI &rarr; $O(V^2/N)$  *(N: # of Nodes)*
- List-Based Distributed Dijkstra's Algorithm Implemented with MPI on Kubernetes

  

## Usage
### Settings
Multi-Nodes Cluster에서 수행하기 위해선 `hostfile`설정과 `ssh` 공개키 배포 작업을 먼저 수행해야 하며, <br>
K8s 환경에서는 아직 Single-Node MPI만을 지원함.

### Compilation
``` shell
source {mpi_installed_dir}/setvars.sh
cd ./MPI_make
make
```
### MPI Run
``` shell
mpirun -n {_#_of_processes} {.exe_file} {vertex_size} {input_file}

```
### MPI + K8s Run
``` shell
sudo kubectl create namespace {namespace}
sudo kubectl apply 5win/mpi_dijkstra:v0.4 -n {namespace}
sudo kubectl exec -it mpi_dijkstra /bin/bash

source {mpi_installed_dir}/setvars.sh
cd ./MPI_make
make

mpirun -n {_#_of_processes} {.exe_file} {vertex_size} {input_file}
```
## Experiment Results
Dijkstra's Algorithm에 대한 벤치마킹 비교는 아래와 같이 수행하였음. (ms 단위)
- MPI 분산 알고리즘에 대한 Single-Node에서의 성능 비교.
- MPI 분산 알고리즘에 대한 Two-Nodes에서의 성능 비교.
- Single-Node에서의 non-K8s와 K8s 환경의 성능 비교.
- Single-Node에서의 각 알고리즘별 성능 비교.
### Single Node
<img width="710" alt="image" src="https://github.com/5win/MPI-dijkstra/assets/94297900/b01c40c4-ab02-4291-a645-3c80a37fcb78">

### Two Nodes
<img width="710" alt="image" src="https://github.com/5win/MPI-dijkstra/assets/94297900/2eceae14-64da-4bc8-9c36-0e466ad41c03">

### On K8s
<img width="709" alt="image" src="https://github.com/5win/MPI-dijkstra/assets/94297900/ebe692e2-3e44-461a-8142-2c5e8262be72">

### Comparison of Algorithms
<img width="710" alt="image" src="https://github.com/5win/MPI-dijkstra/assets/94297900/c61e3a9d-319f-413f-84d1-4aabe30d7dae">

## Conclusion
Single-Node 환경에서는 할당 core 수의 증가에 따라 시간복잡도 $O(V^2/N)$에 맞게 수행 시간이 감소하였음.  
그러나, Two-Nodes 환경에서는 네트워크 통신 오버헤드의 발생으로 같은 core 수에 대해 Single-Node 보다 수행 시간이 증가하였음.  
<br>
다음으로 K8s환경에서 수행 시, 일반 로컬 환경 대비 약 2배정도의 성능 향상이 있었음.  
리스트 기반 알고리즘의 특성상 우선순위 큐 기반의 알고리즘보다 성능이 낮았지만, 기존 리스트 기반 알고리즘 대비 성능 향상이 있음을 확인하였음.
