# ccToggle
Toggle for cc midi commands, most plugins require a value of 127 for on and 0 for off, but most pedalboard only send 127, this plugin is buffer saves the current state and convert the 12 from the pedalboard to 0 (or leave 127) when needed. 

to build:
```gcc -fvisibility=hidden -fPIC -Wl,-Bstatic -Wl,-Bdynamic -Wl,--as-needed -shared -pthread `pkg-config --cflags lv2` -lm `pkg-config --libs lv2` channelSwitch.c -o channelSwitch.so```

needed in ubuntu:
```sudo apt install build-essential pkg-config lv2-dev```
