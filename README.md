# ⚡ Ray Tracer

## Dependencies

- **Dear Imgui**: Included as a submodule in the git tree.
- **SDL2**: Ensure `SDL2` binaries are available on your system. You can figure that one out yourself.

## Setup

To set up the project, use the following commands:

For WebAssembly:
```sh
meson setup builddir_wasm -Dtarget=wasm
```

For Linux:
```sh
meson setup builddir_linux -Dtarget=linux
```

## Compilation

To compile the project, use the following commands:

For WebAssembly:
```sh
meson compile -C builddir_wasm
python -m http.server 9090 -d ./builddir_wasm
```

For Linux:
```sh
meson compile -C builddir_linux
./builddir_linux/raytracer
```