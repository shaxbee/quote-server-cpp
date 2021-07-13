# quote-server-cpp

Quote Server is providing stream of Orderbook and Trade updates from Coinbase API.

## Run

Start server:
```sh
docker run -p 8080:8080 shaxbee/quote-server-cpp
```


## API

Orderbook and trade updates are streamed over [GRPC](https://grpc.io) with API schema defined in `api/quote.proto`.
Server can be interacted with using tools like grpcurl, grpc_cli, evans or programatically by generating bindings in prefered language.

### Subscribe to orderbook

```
grpc_cli call localhost:8080 quote.Quote.SubscribeOrderbook 'product_id: "BTC-USD"'
```

### Subscribe to trades

```
grpc_cli call localhost:8080 quote.Quote.SubscribeTrade 'product_id: "BTC-USD"'
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

## Design

### Layers

* Coinbase Client (`server/coinbase`) - uses REST/Websocket Coinbase API to [fetch orderbook](https://docs.pro.coinbase.com/#get-product-order-book) and [subscribe to full channel](https://docs.pro.coinbase.com/#the-full-channel).
* Coinbase Source (`server/source.h`) - manages orderbooks and maps full channel updates to orderbook updates and trades.
* Server (`server/quote_service.h`) - retrieves current orderbook snapshots and pushes orderbook updates and trades to clients.

### Synchronization

To distribute messages across subscribers the Dispatcher is provided that pushes messages to buffered Subscribers.

Ring buffer is used for message storage in Subscribers as well as buffering full channel.

### End-to-end dataflow

[![](https://mermaid.ink/img/eyJjb2RlIjoic2VxdWVuY2VEaWFncmFtXG4gICAgcGFydGljaXBhbnQgQ2xpZW50XG4gICAgcGFydGljaXBhbnQgU2VydmVyXG4gICAgcGFydGljaXBhbnQgU291cmNlXG4gICAgcGFydGljaXBhbnQgQ29pbmJhc2VcblxuICAgIFNvdXJjZS0-PkNvaW5iYXNlOiBTdWJzY3JpYmUgdG8gZnVsbCBjaGFubmVsXG4gICAgQ29pbmJhc2UtPj5Tb3VyY2U6IFN1YnNjcmliZWRcbiAgICBwYXIgbWVzc2FnZSBoYW5kbGVyXG4gICAgICAgIGxvb3BcbiAgICAgICAgICAgIENvaW5iYXNlLS0-PlNvdXJjZTogRnVsbCB1cGRhdGVcbiAgICAgICAgZW5kXG4gICAgYW5kIG9yZGVyYm9vayBzdGF0ZVxuICAgICAgICBTb3VyY2UtPj5Db2luYmFzZTogR2V0IG9yZGVyYm9va3NcbiAgICAgICAgQ29pbmJhc2UtPj5Tb3VyY2U6IE9yZGVyYm9va3NcblxuICAgICAgICBsb29wIHVwZGF0ZVxuICAgICAgICAgICAgQ29pbmJhc2UtLT4-U291cmNlOiBPcmRlcmJvb2sgdXBkYXRlXG4gICAgICAgIGVuZFxuICAgIGFuZCBjbGllbnQgZmxvd1xuICAgICAgICBDbGllbnQtPj5TZXJ2ZXI6IFN1YnNjcmliZSBvcmRlcmJvb2tcbiAgICAgICAgU2VydmVyLT4-U291cmNlOiBTdWJzY3JpYmUgb3JkZXJib29rXG4gICAgICAgIFNlcnZlci0-PlNvdXJjZTogR2V0IG9yZGVyYm9va1xuICAgICAgICBTb3VyY2UtPj5TZXJ2ZXI6IE9yZGVyYm9va1xuICAgICAgICBTZXJ2ZXItPj5DbGllbnQ6IE9yZGVyYm9vayBzbmFwc2hvdFxuICAgICAgICBsb29wIHVwZGF0ZVxuICAgICAgICAgICAgU291cmNlLS0-PlNlcnZlcjogT3JkZXJib29rIHVwZGF0ZVxuICAgICAgICAgICAgU2VydmVyLS0-PkNsaWVudDogT3JkZXJib29rIHVwZGF0ZVxuICAgICAgICBlbmRcbiAgICBlbmRcbiAgICAgICAgICAgICIsIm1lcm1haWQiOnsidGhlbWUiOiJkZWZhdWx0In0sInVwZGF0ZUVkaXRvciI6ZmFsc2UsImF1dG9TeW5jIjp0cnVlLCJ1cGRhdGVEaWFncmFtIjpmYWxzZX0)](https://mermaid-js.github.io/mermaid-live-editor/edit##eyJjb2RlIjoic2VxdWVuY2VEaWFncmFtXG4gICAgcGFydGljaXBhbnQgQ2xpZW50XG4gICAgcGFydGljaXBhbnQgU2VydmVyXG4gICAgcGFydGljaXBhbnQgU291cmNlXG4gICAgcGFydGljaXBhbnQgQ29pbmJhc2VcblxuICAgIFNvdXJjZS0-PkNvaW5iYXNlOiBTdWJzY3JpYmUgdG8gZnVsbCBjaGFubmVsXG4gICAgQ29pbmJhc2UtPj5Tb3VyY2U6IFN1YnNjcmliZWRcbiAgICBwYXIgbWVzc2FnZSBoYW5kbGVyXG4gICAgICAgIGxvb3BcbiAgICAgICAgICAgIENvaW5iYXNlLS0-PlNvdXJjZTogRnVsbCB1cGRhdGVcbiAgICAgICAgZW5kXG4gICAgYW5kIG9yZGVyYm9vayBzdGF0ZVxuICAgICAgICBTb3VyY2UtPj5Db2luYmFzZTogR2V0IG9yZGVyYm9va3NcbiAgICAgICAgQ29pbmJhc2UtPj5Tb3VyY2U6IE9yZGVyYm9va3NcblxuICAgICAgICBsb29wIHVwZGF0ZVxuICAgICAgICAgICAgQ29pbmJhc2UtLT4-U291cmNlOiBPcmRlcmJvb2sgdXBkYXRlXG4gICAgICAgIGVuZFxuICAgIGFuZCBjbGllbnQgZmxvd1xuICAgICAgICBDbGllbnQtPj5TZXJ2ZXI6IFN1YnNjcmliZSBvcmRlcmJvb2tcbiAgICAgICAgU2VydmVyLT4-U291cmNlOiBTdWJzY3JpYmUgb3JkZXJib29rXG4gICAgICAgIFNlcnZlci0-PlNvdXJjZTogR2V0IG9yZGVyYm9va1xuICAgICAgICBTb3VyY2UtPj5TZXJ2ZXI6IE9yZGVyYm9va1xuICAgICAgICBTZXJ2ZXItPj5DbGllbnQ6IE9yZGVyYm9vayBzbmFwc2hvdFxuICAgICAgICBcbiAgICBlbmRcbiAgICAgICAgICAgICIsIm1lcm1haWQiOiJ7XG4gIFwidGhlbWVcIjogXCJkZWZhdWx0XCJcbn0iLCJ1cGRhdGVFZGl0b3IiOmZhbHNlLCJhdXRvU3luYyI6dHJ1ZSwidXBkYXRlRGlhZ3JhbSI6ZmFsc2V9)