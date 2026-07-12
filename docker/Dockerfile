FROM registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest

WORKDIR /app

RUN apt update && apt install -y \
    build-essential \
    g++ \
    cmake \
    ninja-build \
    git \
    zlib1g \
    zlib1g-dev \
    libssl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    pkg-config \
    libcurl4-openssl-dev \
    libmaxminddb-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

RUN git config --system --add safe.directory '*'

WORKDIR /app/source

CMD ["/bin/bash"]