FROM ubuntu:18.04
MAINTAINER muebau

ENV DEBIAN_FRONTEND noninteractive
ENV HOME /

VOLUME ["/config"]
VOLUME ["/output"]
VOLUME ["/world"]

RUN apt-get update && \
    apt-get -y install git cmake build-essential libpng-dev libjpeg-dev libboost-iostreams-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-test-dev wget python imagemagick && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ADD . /git/mapcrafter

# Textures
RUN cd /git/mapcrafter && \
    wget https://launcher.mojang.com/v1/objects/8de235e5ec3a7fce168056ea395d21cbdec18d7c/client.jar && \
    ./src/tools/mapcrafter_textures.py client.jar src/data/textures/ && \
    rm client.jar

# Build mapcrafter from source
RUN cd /git/mapcrafter && \
    mkdir build && cd build && \
    cmake .. && \
    make ; make && \
    make ; make install && \
    ldconfig

# Render.sh, as the container entrypoint
ADD docker/render.sh /render
RUN chmod 0777 /render


# Default render.conf, if user does not specify one
# Run with -v $PWD/config:/config to override, where $PWD/config contains a render.conf
ADD docker/render.conf /render.conf

ENTRYPOINT ["/render"]

