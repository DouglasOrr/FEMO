set -e

CFLAGS="-x cuda --cuda-gpu-arch=sm_52 -isystem /usr/include/python3.6m/ -std=c++14 -Wall -Wextra -Werror -shared -fpic -g -O2"
LINKFLAGS="-L/usr/local/cuda/lib64/ -lcudart_static -ldl -lrt -pthread"

docker build --rm -t femo .
nvidia-docker run --rm -it --user "$UID:$(id -g $UID)" \
              -v `pwd`:/work -w /work \
              femo \
              clang++-9 ${CFLAGS} femo_native.cpp ${LINKFLAGS} -o femo_native.so
