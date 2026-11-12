FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S backend -B backend/build
RUN cmake --build backend/build -j2

EXPOSE 8080

WORKDIR /app/backend/build

CMD ["./powerpath"]