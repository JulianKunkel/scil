FROM centos:6

WORKDIR /data
RUN yum install -y git pkg-config nano gcc wget bzip2 patch gcc-c++ make
RUN wget https://cmake.org/files/v3.0/cmake-3.0.0-1-src.tar.bz2
RUN tar -xf cmake-3.0.0-1-src.tar.bz2
RUN bash -c "./cmake-3.0.0-1.sh all ; cd cmake-3.0.0/.build && make install"
RUN yum install -y unzip gsl-devel zlib-devel
RUN git clone https://github.com/lz4/lz4.git
RUN cd lz4 ; make install
