# Build the application
FROM ubuntu:22.04 as builder
LABEL Author="Bao Quan Lindgren <guslindgba@student.gu.se>"

# Install dependencies like the compiler
RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get dist-upgrade -y && \
    apt-get install -y --no-install-recommends \
        cmake \
        build-essential

# Copy the source directory
ADD . /opt/sources
WORKDIR /opt/sources

# Build the application and move the application
# to a recognisable path
RUN bash build.sh && \
    cd build && \
    mv mem3 /


# Package the software into a bundle
FROM ubuntu:22.04
LABEL Author="Bao Quan Lindgren <guslindgba@student.gu.se>"

# Copy the statically linked executable
COPY --from=builder /mem3 .
ENTRYPOINT [ "/mem3" ]
