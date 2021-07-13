# quote-server-cpp

Quote Server is providing stream of Orderbook and Trade updates from Coinbase API.

## Run

```sh
docker run -p 8080:8080 shaxbee/quote-server-cpp
```

## Build

### Docker

```sh
docker build -t shaxbee/quote-server-cpp
```

### Local

Install cmake and conan.

Create build directory:
```sh
mkdir -p build/
```

Install dependencies:
```sh
conan install -if build/ .
```

Configure and build:
```sh
cmake -B build/
cmake --build build/
```