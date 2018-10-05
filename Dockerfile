#
# Build Image
#

FROM alpine:3.8 as builder

# Add the git repo
ADD . /git/mapcrafter

# Dependencies needed for building Mapcrafter
# (not sure how many of these are actually needed)
RUN apk add \
        cmake \
        gcc \
        make \
        g++ \
        zlib-dev \
        libpng-dev \
        libjpeg-turbo-dev \
        boost-dev

# Build mapcrafter from source
RUN cd /git/mapcrafter && \
    mkdir build && cd build && \
    cmake .. && \
    make && \
    mkdir /tmp/mapcrafter && \
    make DESTDIR=/tmp/mapcrafter install


#
# Final Image
#

FROM alpine:3.8

# Mapcrafter, built in previous stage
COPY --from=builder /tmp/mapcrafter/ /

# Depedencies needed for running Mapcrafter
RUN apk add \
        libpng \
        libjpeg-turbo \
        boost \
        boost-iostreams \
        boost-system \
        boost-filesystem \
        boost-program_options \
        shadow

# Entrypoint
ADD entrypoint.sh /
ENTRYPOINT ["/entrypoint.sh"]
