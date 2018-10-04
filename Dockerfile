#
# Build Image
#

FROM ubuntu:18.04 as builder

ENV DEBIAN_FRONTEND noninteractive
ENV HOME /

# Add the git repo
ADD . /git/mapcrafter


# Build dependencies
RUN apt-get update && \
    apt-get -y install cmake build-essential libpng-dev libjpeg-dev libboost-iostreams-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-test-dev wget python imagemagick && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


# Textures
RUN cd /git/mapcrafter && \ 
    wget https://launcher.mojang.com/v1/objects/8de235e5ec3a7fce168056ea395d21cbdec18d7c/client.jar && \
    ./src/tools/mapcrafter_textures.py client.jar src/data/textures/ && \
    rm client.jar

# Build mapcrafter from source
RUN cd /git/mapcrafter && \
    mkdir build && cd build && \
    cmake .. && \
    make && \
    mkdir /tmp/mapcrafter && \
    make DESTDIR=/tmp/mapcrafter install && \
    ldconfig

#ENTRYPOINT ["mapcrafter"]


#
# Final Image
#

FROM ubuntu:18.04

# Mapcrafter, built in previous stage
COPY --from=builder /tmp/mapcrafter/ /

# Run Depedencies
RUN apt-get update && \
    apt-get -y install libpng-dev libjpeg-dev libboost-iostreams-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    ldconfig

ENTRYPOINT ["mapcrafter"]
