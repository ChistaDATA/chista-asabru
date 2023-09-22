# Use an official CMake image as the base image
FROM danger89/cmake
# RUN apt-get update && apt-get -y install cmake protobuf-compiler

# Set the working directory inside the container
WORKDIR /app

# Copy the CMake project files into the container
COPY . /app

# Build the dependencies
RUN chmod +x ./build_libs.sh && ./build_libs.sh

# Build the CMake project
RUN rm -rf build 
RUN mkdir build 
RUN cd build && cmake .. && make

# # --------------------------------------------------

FROM scratch
COPY --from=0 /app/build/Chista_Asabru /bin/Chista_Asabru
COPY --from=0 /app/lib/asabru-handlers/build /asabru-handlers
COPY --from=0 /app/config/config.xml /config.xml
ENV CONFIG_FILE=/config.xml
ENV PLUGINS_FOLDER_PATH=/asabru-handlers
CMD ["/bin/Chista_Asabru"]