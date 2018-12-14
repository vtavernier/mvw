# 3D test model viewer

This repository contains a sample program showing how to load various 3D model formats and
how to render them using [libshadertoy](https://gitlab.inria.fr/vtaverni/libshadertoy).

## Model viewer

This is still work in progress.

Asset loading is done using the [assimp](assimp/) library.

## How to use

```bash
mkdir -p build ; cd build
cmake ..
make -j$(nproc)
./viewer
```

## Author

* Viewer code: Vincent Tavernier <vincent.tavernier@inria.fr>
* Models: [test-models](https://gitlab.inria.fr/vtaverni/test-models)
