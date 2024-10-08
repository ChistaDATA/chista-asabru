FROM ubuntu:latest AS builder
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update -y \
  && apt-get upgrade -y \
  && apt-get install -y \
    git \
    build-essential \
    libtool \
    libssl-dev \
    automake \
    dpkg \
    cmake \
    libuv1-dev \
    ccache \
    python3 \
    python3-dev \
    sqlite3 \
    libsqlite3-dev \
    bash \
    curl

RUN apt-get install -y \
    intltool \
    autoconf \
    m4 \
    libssl-dev \
    libreadline8 \
    libreadline-dev

RUN curl -sL https://deb.nodesource.com/setup_18.x | bash
RUN apt-get install -y nodejs

# Set the working directory inside the container
WORKDIR /app

# Copy the CMake project files into the container
COPY . /app

# Build the dependencies
RUN chmod +x ./build_libs.sh 
RUN ./build_libs.sh

# Build the CMake project
RUN rm -rf build 
RUN mkdir build 
RUN cd build && cmake -DASABRU_COMMONS_BUILD=LOCAL_DIR -DASABRU_ENGINE_BUILD=LOCAL_DIR -DASABRU_PARSERS_BUILD=LOCAL_DIR -DGTEST_ENABLED:BOOL=ON .. && make

# The second stage will install the runtime dependencies only and copy
# the compiled executables

FROM ubuntu:latest AS runner

RUN apt-get update -y \
  && apt-get upgrade -y \
  && apt-get install -y \
    build-essential \
    python3 \
    bash

RUN apt-get install -y \
    intltool \
    autoconf \
    m4 \
    libreadline8 \
    libreadline-dev

RUN mkdir -p /opt/bin
RUN ln -s /usr/bin/curl /opt/bin/curl

COPY --from=0 /app/build/src /bin
COPY --from=0 /app/build/public /bin/public
COPY --from=0 /app/build/config.xml /bin/config.xml
COPY --from=0 /app/lib/asabru-handlers/build /asabru-handlers
ENV PLUGINS_FOLDER_PATH=/asabru-handlers
ENV PUBLIC_FOLDER_PATH=/bin/public
CMD ["/bin/Chista_Asabru"]