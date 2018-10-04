#
# Build Image
#

FROM ubuntu:18.04 as builder

# Add the git repo
ADD . /git/mapcrafter

# Dependencies needed for building Mapcrafter
RUN apt-get update && \
    apt-get -y install \
        cmake \
        build-essential \
        libpng-dev \
        libjpeg-dev \
        libboost-iostreams-dev \
        libboost-system-dev \
        libboost-filesystem-dev \
        libboost-program-options-dev \
        libboost-test-dev \
        wget \
        python \
        imagemagick


# Textures
RUN cd /git/mapcrafter && \
    wget https://launcher.mojang.com/v1/objects/8de235e5ec3a7fce168056ea395d21cbdec18d7c/client.jar && \
    ./src/tools/mapcrafter_textures.py client.jar src/data/textures/

# Build mapcrafter from source
RUN cd /git/mapcrafter && \
    mkdir build && cd build && \
    cmake .. && \
    make && \
    mkdir /tmp/mapcrafter && \
    make DESTDIR=/tmp/mapcrafter install && \
    ldconfig


#
# Final Image
#

FROM ubuntu:18.04

# Mapcrafter, built in previous stage
COPY --from=builder /tmp/mapcrafter/ /

# Depedencies needed for running Mapcrafter
RUN apt-get update && \
    apt-get -y install \
        libpng16-16 \
        libjpeg-turbo-progs \
        libboost-iostreams1.65.1 \
        libboost-system1.65.1 \
        libboost-filesystem1.65.1 \
        libboost-program-options1.65.1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    ldconfig

ENTRYPOINT ["mapcrafter"]
