# FEMO - Friendly Elastic Melodic Organ

A Physical Modelling Synthesis instrument modeller.

## Try it out

    docker build --rm -t femo .
    docker run --rm -it --user $UID --group-add users -p 9898:9898 femo start-notebook.sh --port 9898 --ip '0.0.0.0'
