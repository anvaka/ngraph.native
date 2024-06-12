# ngraph.native

C++ implementation of force-based layout from ngraph. This is equivalent to
[ngraph.offline.layout](https://github.com/anvaka/ngraph.offline.layout).
Unfortunately I don't have bindings to nodejs (yet).


## Build

You need `cmake` to build this source. 

```
mkdir build
cmake ..
make 
```

The `ngraph.native` executable will be built and available in the build folder.

## Usage

```
Usage: 
  ./ngraph.native links.bin [positions.bin]
Where
 `links.bin` is a path to the serialized graph. See 
    https://github.com/anvaka/ngraph.tobinary for format description
  `positions.bin` is optional file with previously saved positions. 
    This file should match `links.bin` graph, otherwise bad things 
    will happen

```


My C++ is very much rusty, please feel free to contribute if you find something
to improve.
