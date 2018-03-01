# RT1W
![demo](./demo.png)

A ray tracer written in C++, base on Peter Shirley's book *Ray Tracing in one weekend*.

## Building
You'll need CMake and libpng.

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
## Running
This will render a predefined scene and export it to render.png:
```bash
$ ./rt1w --quality=5 scene.json
```
The argument `quality=n` means that 2^n rays are traced for each
pixel. More rays mean higher quality but slower rendering. The image
above was rendered with n = 10.

You can check other options with
```bash
$ ./rt1w --help
```
## License
MIT License.
