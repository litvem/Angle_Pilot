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
RUN cd angle-calculator && \
    bash build.sh

# Copy the final executable
RUN cd angle-calculator/build && \
    cp angle-calculator /

# Clean up
RUN cd / && \
    rm -rf /opt/sources && \
    apt-get remove -y build-essential cmake

# Deploy stage
FROM scratch
LABEL Author="Bao Quan Lindgren <guslindgba@student.gu.se>"

COPY --from=builder /angle-calculator /
ENTRYPOINT [ "/angle-calculator" ]
