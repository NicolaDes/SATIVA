# SAT-solver

## Dependencies

Cmake minimum required is 2.8.12, you can check with command.
```
cmake --version
```
Dot to see proof generation is required, to check if it is installed type
```
type dot
```
if it is not installed install with
```
sudo apt-get install graphviz
```

## Compile

```
bash compile.sh [proof]
``` 
Will generate the default configuration. proof parameter is optional and if specified, it will generate sativa program with proof generation mode active. Otherwise
```
mkdir build && cd build
```
Then generate make file with cmake.
```
cmake ..
```
Verbose mode is on by default, proof generation is off by default.
```
cmake .. -DP=1
```
will set the proof mode ON.

## Execute
```
./sativa -h
```
will display the usage.
```
./sativa -f [input files]
```
will launch sativa on a cnf file.
```
./sativa -p [number]
```
will launch sativa on pigeonhole of size [number].

If the prove was generated you can check it with command
```
eog graphics/proof.png
```
Or you can open graphics/proof.png with your preferred image visualizer.
