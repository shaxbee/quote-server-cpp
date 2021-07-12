FROM debian:bullseye-slim AS build
RUN apt-get update && \
    apt-get install -y python3-pip gcc-10 cmake && \
    rm -rf /var/lib/apt/lists/*
RUN pip3 install conan
RUN conan profile new default --detect && \
    conan profile update settings.compiler.libcxx=libstdc++11 default
WORKDIR /opt/quote-server-cpp
COPY conanfile.txt .
RUN conan install --build=missing -if build/ .
COPY . .
RUN cmake -B build/ && \
    cmake --build build/ --config RelMinSize

FROM debian:bullseye-slim
RUN apt-get update && \
    apt-get install -y libatomic1 libstdc++6 && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /opt/quote-server-cpp
COPY --from=build /opt/quote-server-cpp/build/bin/server /opt/quote-server-cpp/server
CMD ["/opt/quote-server-cpp/server"]