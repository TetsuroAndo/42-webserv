FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y wget lsb-release software-properties-common gnupg && \
    wget https://apt.llvm.org/llvm.sh && chmod +x llvm.sh && ./llvm.sh 21 all && \
    apt-get install -y clang-format-21 && \
    ln -sf /usr/bin/clang-format-21 /usr/bin/clang-format && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /workspace
