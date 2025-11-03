FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gdb \
    clang clang-tidy \
    pkg-config \
    git \
    python3 \
    libssl-dev \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Build and install cmake 4.0.3
RUN wget https://cmake.org/files/v4.0/cmake-4.0.3.tar.gz \
    && tar -xzvf cmake-4.0.3.tar.gz \
    && cd cmake-4.0.3 \
    && ./bootstrap \
    && make -j$(nproc) \
    && make install \
    && cd .. \
    && rm -rf cmake-4.0.3* \
    && rm -f cmake-4.0.3.tar.gz

# Build command:
#   docker build --load -t webserv-devenv:latest .
#
# Run command:
#   docker run -it --rm -v $(pwd):/workspace -w /workspace webserv-devenv:latest bash
