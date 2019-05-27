# 3D test model viewer

This repository contains a sample program showing how to load various 3D model formats and
how to render them using [libshadertoy](https://gitlab.inria.fr/vtaverni/libshadertoy).

## Dependencies

The GLSL code is built using the [GLSL Preprocessor](https://github.com/vtavernier/glsl-preprocessor).
It can be installed with [`cpanm`](https://metacpan.org/pod/App::cpanminus):

```bash
cpanm git://github.com/vtavernier/glsl-preprocessor.git
```

## Model viewer

This is still work in progress.

Asset loading is done using the [assimp](assimp/) library.

Shader assembly is done using m4, which needs to be installed.

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
