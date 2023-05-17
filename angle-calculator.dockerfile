# Copyright (C) 2023  Robert Einer, Emma Litvin, Ossian Ålund, Bao Quan Lindgren, Khaled Adel Saleh Mohammed Al-Baadani
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

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

WORKDIR /usr/bin
COPY --from=builder /angle-calculator .
ENTRYPOINT [ "/usr/bin/angle-calculator" ]
