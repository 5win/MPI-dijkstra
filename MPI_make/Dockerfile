FROM ubuntu:20.04

RUN apt update  \
    && apt install -y wget gnupg2 ca-certificates

RUN wget https://apt.repos.intel.com/intel-gpg-keys/GPG-PUB-KEY-INTEL-SW-PRODUCTS.PUB -O key.PUB \
    && apt-key add key.PUB \
    && rm key.PUB \
    && echo "deb https://apt.repos.intel.com/oneapi all main" | tee /etc/apt/sources.list.d/oneAPI.list \
    && apt update
    
RUN apt install -y intel-oneapi-compiler-dpcpp-cpp \
        intel-oneapi-mpi-devel \
        openssh-server \
        vim gcc g++ make \
        curl \
    && service ssh start

RUN useradd -m mpiuser
WORKDIR /home/mpiuser

COPY ./* .