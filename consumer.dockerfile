# Build stage
FROM ubuntu:22.04 as builder
LABEL Author="Bao Quan Lindgren <guslindgba@student.gu.se>"

# Install dependencies
RUN apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y build-essential cmake

# Copy working directory
ADD ./src /opt/sources
WORKDIR /opt/sources

# Build the application
RUN cd api && \
    bash build.sh

# Copy the final executable
RUN cd api/build && \
    cp consumer /

# Clean up
RUN cd / && \
    rm -rf /opt/sources && \
    apt-get remove -y build-essential cmake

# Deploy stage
FROM scratch
LABEL Author="Bao Quan Lindgren <guslindgba@student.gu.se>"

COPY --from=builder /consumer /
ENTRYPOINT [ "/consumer" ]
