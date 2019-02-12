FROM ubuntu:16.04

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get install -y git wget curl build-essential unzip ca-certificates libjpeg-dev libpng-dev \
                       autoconf libtool pkg-config libopenblas-dev
WORKDIR /opt

# Get rid of old cmake
RUN apt-get remove -y cmake && apt-get purge -y --auto-remove cmake
RUN wget https://cmake.org/files/v3.12/cmake-3.12.4.tar.gz && tar -xzvf cmake-3.12.4.tar.gz
RUN cd cmake-3.12.4 && ./bootstrap && make -j $(nproc) && make install && cmake --version

# Install gRPC
RUN git clone -b v1.15.0 https://github.com/grpc/grpc
RUN cd grpc && git submodule update --init
RUN cd grpc && make -j $(nproc) REQUIRE_CUSTOM_LIBRARIES_opt=1 static
RUN cd grpc && make install
RUN cd grpc/third_party/protobuf && make install

# Install LevelDB
RUN git clone https://github.com/google/leveldb.git
RUN cd leveldb && git reset --hard 1cb384088184be9840bd59b4040503a9fa9aee66
RUN cd leveldb && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cd leveldb && cd build && make -j $(nproc)
RUN cd leveldb && cd build && make install

# PyTorch
ARG PYTHON_VERSION=3.6
RUN curl -o ~/miniconda.sh -O  https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh  && \
     chmod +x ~/miniconda.sh && \
     ~/miniconda.sh -b -p /opt/conda && \
     rm ~/miniconda.sh && \
     /opt/conda/bin/conda install -y python=$PYTHON_VERSION numpy pyyaml scipy ipython mkl mkl-include cython typing && \
     /opt/conda/bin/conda clean -ya
ENV PATH /opt/conda/bin:$PATH
RUN pip install grpcio-tools
WORKDIR /opt/pytorch
RUN git clone https://github.com/pytorch/pytorch.git
WORKDIR /opt/pytorch/pytorch

RUN git checkout v1.0.1 && git submodule update --init && mkdir build

RUN cd build && python ../tools/build_libtorch.py

RUN cd / && git clone https://github.com/perone/euclidesdb.git && \
    cd /euclidesdb && git submodule update --init

RUN ln -s /opt/pytorch/pytorch/torch/lib/tmp_install /euclidesdb/libtorch
RUN mv /euclidesdb/libtorch/include/google /euclidesdb/libtorch/include/g

RUN mkdir -p /euclidesdb/build
RUN cd /euclidesdb/build && cmake -DCMAKE_BUILD_TYPE=Release ..
RUN cd /euclidesdb/build && make -j $(nproc) VERBOSE=1
RUN cd /euclidesdb/build && make package

WORKDIR /euclidesdb/build
