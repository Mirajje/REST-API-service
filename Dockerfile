FROM ghcr.io/userver-framework/docker-userver-build-base:v1a AS builder

RUN apt-get install -y g++
RUN apt-get install -y cmake
RUN apt-get install -y pkg-config
RUN apt-get install -y libpqxx-dev
RUN apt-get install -y git
		
RUN git clone https://github.com/microsoft/vcpkg
RUN apt-get install -y curl zip
RUN vcpkg/bootstrap-vcpkg.sh

RUN /vcpkg/vcpkg install crow

COPY /src CMakeLists.txt /app

WORKDIR /app

EXPOSE 8080

RUN cmake . && make

CMD ["./app"]