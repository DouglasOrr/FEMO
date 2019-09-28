FROM nvidia/cuda:10.1-cudnn7-devel-ubuntu18.04

RUN apt-get update \
    && apt-get install -qy \
       clang \
       libsndfile1 \
       python3 \
       python3-pip \
       wget

COPY requirements.txt /tmp/requirements.txt
RUN pip3 install --upgrade -r /tmp/requirements.txt

RUN wget https://github.com/pybind/pybind11/archive/v2.4.2.tar.gz -O /tmp/pybind11.tar.gz \
    && tar -C /usr/include -xf /tmp/pybind11.tar.gz --strip-components=2  pybind11-2.4.2/include/
