set -e
docker build --rm -t femo .
nvidia-docker run --rm -it --user "$UID:$(id -g $UID)" -p 9898:9898 \
              -v `pwd`:/work -w /work \
              -e HOME=/work/.jupyterhome \
              -e JUPYTER_RUNTIME_DIR=/tmp/jupyter_runtime \
              -e PYTHONPATH=/work \
              femo \
              jupyter lab --port 9898 --ip '0.0.0.0'
