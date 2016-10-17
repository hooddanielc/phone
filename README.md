Phone
=====

Compile code for Arm Processor
```
docker-compose up
```

Push the code to your raspberry pi
```
mv out <to-your-rpi>i
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
