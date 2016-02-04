# This Dockerfile describes the setup of the container running the fantasticd
# server.
FROM ubuntu:15.04
MAINTAINER Jonas Sternisko <jonas.sternisko@gmail.com>
# Install packages. Keep the entries ordered.
RUN apt-get update  \
 && apt-get install -y  \
    git  \
    libboost-filesystem-dev  \
    libboost-locale-dev  \
    libboost-serialization-dev  \
    libboost-system-dev  \
    libboost-system1.55.0  \
 && apt-get clean  \
 && rm -rf /var/lib/apt/lists/*

# Set the locale
RUN locale-gen de_DE.UTF-8
ENV LANG=de_DE.UTF-8  \
    LANGUAGE=de:en  \
    LC_ALL=

# Set the working directory for all RUN, CMD, ENTRYPOINT, COPY and ADD
# instructions that follow.
ENV WORKING_DIR /home/fantasticd
RUN mkdir -p $WORKING_DIR
WORKDIR $WORKING_DIR
RUN mkdir bin www 

# Copy the binaries and inverted index into the container.
COPY ICD.ii data/
COPY src/searchServerMain bin/
COPY www www

# Expose port 8080 to the outside.
EXPOSE 8080

# Run the server
CMD ["./bin/searchServerMain", "8080", "data/ICD.ii"]

