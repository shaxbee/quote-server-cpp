FROM gcc:10-buster AS build
RUN apt-get update && apt-get install -y \
    python3-pip \
    cmake \
 && rm -rf /var/lib/apt/lists/*
RUN pip3 install conan
RUN conan profile new default --detect && \
    conan profile update settings.compiler.libcxx=libstdc++11 default
WORKDIR /opt/quote-server-cpp
COPY conanfile.txt .
RUN conan install --build=missing -if build/ .
COPY . .
RUN cmake -B build/ && \
    cmake --build build/

CMD build/bin/server