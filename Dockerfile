FROM arm64v8/alpine:latest AS builder

RUN apk update \
  && apk upgrade \
  && apk add --no-cache \
    openssl \
    openssl-dev \
    clang \
    clang-dev \
    alpine-sdk \
    dpkg \
    cmake \
    ccache \
    python3 \
    bash

RUN ln -sf /usr/bin/clang /usr/bin/cc \
  && ln -sf /usr/bin/clang++ /usr/bin/c++ \
  && update-alternatives --install /usr/bin/cc cc /usr/bin/clang 10\
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 10\
  && update-alternatives --auto cc \
  && update-alternatives --auto c++ \
  && update-alternatives --display cc \
  && update-alternatives --display c++ \
  && ls -l /usr/bin/cc /usr/bin/c++ \
  && cc --version \
  && c++ --version

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
RUN cd build && cmake .. && make

# The second stage will install the runtime dependencies only and copy
# the compiled executables

FROM arm64v8/alpine:latest AS runtime

RUN apk update \
  && apk upgrade \
  && apk add --no-cache \
    openssl \
    openssl-dev \
    clang \
    clang-dev \
    alpine-sdk \
    dpkg \
    cmake \
    ccache \
    python3 \
    bash

RUN ln -sf /usr/bin/clang /usr/bin/cc \
  && ln -sf /usr/bin/clang++ /usr/bin/c++ \
  && update-alternatives --install /usr/bin/cc cc /usr/bin/clang 10\
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 10\
  && update-alternatives --auto cc \
  && update-alternatives --auto c++ \
  && update-alternatives --display cc \
  && update-alternatives --display c++ \
  && ls -l /usr/bin/cc /usr/bin/c++ \
  && cc --version \
  && c++ --version

RUN mkdir -p /opt/bin
RUN ln -s /usr/bin/curl /opt/bin/curl

COPY --from=0 /app/build/Chista_Asabru /bin/Chista_Asabru
COPY --from=0 /app/lib/asabru-handlers/build /asabru-handlers
ENV PLUGINS_FOLDER_PATH=/asabru-handlers
CMD ["/bin/Chista_Asabru"]