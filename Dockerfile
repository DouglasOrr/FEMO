FROM nvidia/cuda:10.1-cudnn7-devel-ubuntu18.04

RUN apt-get update && apt-get install -qy wget \
    && echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main" >> /etc/apt/sources.list \
    && wget -nv -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN apt-get update \
    && apt-get install -qy \
       clang-9 \
       libsndfile1 \
       python3 \
       python3-pip \
       wget

COPY requirements.txt /tmp/requirements.txt
RUN pip3 install --upgrade -r /tmp/requirements.txt

RUN wget -nv -O /tmp/pybind11.tar.gz https://github.com/pybind/pybind11/archive/v2.4.2.tar.gz \
    && tar -C /usr/include -xf /tmp/pybind11.tar.gz --strip-components=2  pybind11-2.4.2/include/
