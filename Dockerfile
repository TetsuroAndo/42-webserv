FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    clang clang-tidy \
    pkg-config git python3 \
 && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y build-essential libssl-dev wget \
 && wget https://cmake.org/files/v4.0/cmake-4.0.3.tar.gz \
 && tar -xzvf cmake-4.0.3.tar.gz \
 && cd cmake-4.0.3 \
 && ./bootstrap \
 && make -j$(nproc) \
 && make install \
 && cd .. && rm -rf cmake-4.0.3* \
 && rm -rf /var/lib/apt/lists/*


# $docker build -t my/ubuntu-dev:22.04 .
# $docker run -it --rm -v $(pwd):/workspace -w /workspace my/ubuntu-dev:22.04 bash