Phone
=====

Build the docker machine

```
./build.sh
```


Compile the code to arm

```
./run.sh
```


Push the code to your raspberry pi

```
mv out <to-your-rpi>
```


Want to compile the tool for other platforms? Modify the common.cfg and replace cc entries with your own compiler

```
cc = Obj(
  tool='clang++',
)

link = Obj(
  tool='clang++',
)
```
