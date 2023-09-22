FROM danger89/cmake:latest AS builder

# Set the working directory inside the container
WORKDIR /app

RUN set -ex;                                                                      \
    apt-get update;                                                               \
    apt-get install -y g++ libzmq3-dev; 

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

FROM debian:buster AS runtime

RUN set -ex;                                                                      \
    apt-get update;                                                               \
    apt-get install -y curl; 
RUN ln -s /usr/bin/curl /bin/curl

COPY --from=0 /app/build/Chista_Asabru /bin/Chista_Asabru
COPY --from=0 /app/lib/asabru-handlers/build /asabru-handlers
COPY --from=0 /app/config/config.xml /config.xml
ENV CONFIG_FILE=/config.xml
ENV PLUGINS_FOLDER_PATH=/asabru-handlers
CMD ["/bin/Chista_Asabru"]