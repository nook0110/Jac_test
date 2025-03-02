FROM ubuntu:24.04

# Install dependencies
RUN apt-get update
RUN apt-get install -y libcln-dev
RUN apt-get install -y libginac-dev
RUN apt-get install -y wget
RUN apt-get install -y unzip
RUN apt-get install -y sudo
RUN apt-get install -y g++
RUN apt-get install -y make
RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install -y libboost-all-dev

# Install MySQL Connector/C++
RUN wget https://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit.tar.gz
RUN tar -xzf mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit.tar.gz
RUN mkdir /usr/local/mysql-connector
RUN cp -r mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit/* /usr/local/mysql-connector/

# Install Alire
RUN wget https://github.com/alire-project/alire/releases/download/v2.0.2/alr-2.0.2-bin-x86_64-linux.zip
RUN unzip alr-2.0.2-bin-x86_64-linux.zip
RUN mv bin/alr /usr/local/bin/
RUN chmod +x /usr/local/bin/alr
RUN alr version

# Install glog
RUN git clone https://github.com/google/glog.git
RUN cd glog && cmake -S . -B build -G "Unix Makefiles"
RUN cd glog && cmake --build build
RUN cd glog && cmake --build build --target install

RUN apt-get install -y mysql-server
RUN apt-get install -y libssl-dev


# Copy source files into the image
COPY . /root/InvJac

# Build the project
WORKDIR /root/InvJac/phc/PHCpack

RUN echo "1" | alr toolchain --select gnat_native
RUN echo "1" | alr toolchain --select gprbuild
RUN alr build

WORKDIR /root/InvJac

RUN rm -r build

RUN cmake -S . -B build
RUN cmake --build build

# Run tests
ENTRYPOINT ["./build/invjac"]