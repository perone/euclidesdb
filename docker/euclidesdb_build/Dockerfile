FROM ubuntu:16.04

ARG GITHUB_TOKEN
ENV GITHUB_OAUTH_TOKEN=$GITHUB_TOKEN

WORKDIR /opt

# Fetch will automatically use the GITHUB_OAUTH_TOKEN env variable
RUN apt-get update && apt-get install -y \
    wget curl libopenblas-dev \
    ca-certificates tar bzip2 && apt-get autoremove -y && apt-get clean -y && \
    rm -rf /var/lib/apt/lists/* && \
    \
    wget https://github.com/gruntwork-io/fetch/releases/download/v0.3.2/fetch_linux_amd64 -P ~ && \
    \
    chmod +x ~/fetch_linux_amd64 && \
    ~/fetch_linux_amd64 --repo="https://github.com/perone/euclidesdb" \
                        --tag="v0.2.0" \
                        --release-asset="euclidesdb-0.2.0-Linux.tar.gz" /opt && \
    tar zxvf euclidesdb-0.2.0-Linux.tar.gz && rm -rf euclidesdb-0.2.0-Linux.tar.gz

WORKDIR /opt/euclidesdb-0.2.0-Linux/euclidesdb

# PyTorch
ARG PYTHON_VERSION=3.6
RUN curl -o ~/miniconda.sh -O  https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh  && \
     chmod +x ~/miniconda.sh && \
     ~/miniconda.sh -b -p /opt/conda && \
     rm ~/miniconda.sh && \
     /opt/conda/bin/conda install -y python=$PYTHON_VERSION numpy pyyaml scipy && \
     /opt/conda/bin/conda install -y pytorch-cpu==1.0.1 -c pytorch && \
     /opt/conda/bin/conda clean -ya && \
     /opt/conda/bin/pip install grpcio-tools torchvision_nightly && \
     cd models/resnet18 && /opt/conda/bin/python resnet_trace.py && \
     rm -rf ../vgg16 && \
     rm -rf ../resnet101 && \
     rm -rf /opt/conda && \
     rm -rf ~/.torch && \
     rm -rf ~/.cache

COPY euclidesdb.conf /opt/euclidesdb-0.2.0-Linux/euclidesdb
COPY bootstrap_euclidesdb.sh /opt/euclidesdb-0.2.0-Linux/euclidesdb

ENTRYPOINT ["/opt/euclidesdb-0.2.0-Linux/euclidesdb/bootstrap_euclidesdb.sh"]
EXPOSE 50000
