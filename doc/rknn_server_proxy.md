## 连板调试简介
RKNN Toolkit2的连板功能一般需要更新板端的 rknn_server 和 librknnrt.so/librknnmrt.so，并且手动启动 rknn_server 才能正常工作。
rknn_server: 是一个运行在板子上的后台代理服务，用于接收PC通过USB传输过来的协议，然后执行板端runtime对应的接口，并返回结果给PC。

- librknnrt.so: 是一个板端的RKNPU Runtime库(非RV1103/RV1106平台不适用)。
- librknnmrt.so: 是专用于RV1103/RV1106平台的RKNPU Runtime库。

有些固件默认已经集成了rknn_server，如果已经集成，可以忽略下面的启动步骤。

## 一、版本要求
- 若使用动态形状输入RKNN模型，要求rknn_server和RKNPU Runtime库版本>=1.5.0。


## 二、rknn_server存放目录
rknn_server存放在runtime目录下, 请根据板子上的系统选择相应版本的rknn_server,不同芯片和系统对应的rknn_server路径如下：
### Android平台
|系统|路径|
|-----|-----|
|32-bit Android|runtime/Android/rknn_server/arm/rknn_server|
|64-bit Android|runtime/Android/rknn_server/arm64/rknn_server|


### Linux平台

|芯片|系统|路径|
|-----|-----|-----|
|RV1103/RV1106|32-bit Linux|runtime/Linux/rknn_server/armhf-uclibc/usr/bin/rknn_server|
|其他芯片|32-bit Linux|runtime/Linux/rknn_server/armhf/usr/bin/rknn_server|
|其他芯片|64-bit Linux|runtime/Linux/rknn_server/aarch64/usr/bin/rknn_server|

## 三、 启动步骤
### Android平台

在PC端执行下列命令启动代理服务:
1. 重新挂载系统分区,使系统分区重新可写
```
adb root && adb remount
```
2. 更新代理程序
```
// 64-bit Android系统
adb push runtime/Android/rknn_server/arm64/rknn_server /vendor/bin/
// 32-bit Android系统
adb push runtime/Android/rknn_server/arm/rknn_server /vendor/bin/
```
3. 更新RKNPU runtime库 
```
// 64-bit Android系统
adb push runtime/Android/librknn_api/arm64-v8a/librknnrt.so /vendor/lib64
// 32-bit Android系统
adb push runtime/Android/librknn_api/armeabi-v7a/librknnrt.so /vendor/lib
```
4. 修改代理程序权限
```
adb shell chmod +x /vendor/bin/rknn_server
```
5. 启动代理服务
```
adb shell "nohup /vendor/bin/rknn_server >/dev/null"&
```
6. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,步骤如下:
adb shell命令进入板子shell界面后,执行下列命令启动代理服务
```
nohup /vendor/bin/rknn_server > /dev/null&
```

### Linux平台(非RV1103/RV1106)
1. 更新代理程序
```
// 64-bit Linux系统
adb push runtime/Linux/rknn_server/aarch64/usr/bin/rknn_server /usr/bin/
// 32-bit Linux系统
adb push runtime/Linux/rknn_server/armhf/usr/bin/rknn_server /usr/bin/
```
2. 更新RKNPU runtime库 
```
// 64-bit Linux系统
adb push runtime/Linux/librknn_api/aarch64/librknnrt.so /usr/lib
// 32-bit Linux系统
adb push runtime/Linux/librknn_api/armhf/librknnrt.so /usr/lib
```
3. 修改代理程序权限
```
adb shell chmod +x /usr/bin/rknn_server
```
4. 启动代理服务
```
adb shell "killall rknn_server ; nohup /usr/bin/rknn_server >/dev/null"
```
5. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令进入板子shell界面后,执行下列命令启动代理服务
```
nohup /usr/bin/rknn_server > /dev/null&
```

### Linux平台(RV1103/RV1106)
RV1103/RV1106上使用的RKNPU Runtime库是librknnmrt.so，使用armhf-uclibc目录下的rknn_server,启动步骤如下：
1. 更新代理程序
```
adb push runtime/Linux/rknn_server/armhf-uclibc/usr/bin/rknn_server /oem/usr/bin
```
2. 更新RKNPU runtime库
```
adb push runtime/Linux/librknn_api/armhf-uclibc/librknnmrt.so /oem/usr/lib
```
3. 修改代理程序权限
```
adb shell chmod +x /oem/usr/bin/rknn_server
```
4. 启动代理服务
```
adb shell "nohup /oem/usr/bin/rknn_server >/dev/null"&
```
5. 检查代理服务是否启动成功：
```
adb shell ps |grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令进入板子shell界面后,执行下列命令启动代理服务
```
nohup /oem/usr/bin/rknn_server > /dev/null&
```

### 查看rknn_server详细日志
代理服务默认不开启详细日志，如需开启详细日志，请参考相应平台执行步骤：
#### Android平台
1. 设置环境变量开启详细日志,并启动代理:
```
adb logcat -c
adb shell "killall rknn_server && setprop persist.vendor.rknn.server.log.level 5 && nohup /vendor/bin/rknn_server >/dev/null"&
```
2. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
3. 运行PC上python推理程序
4. 查看运行日志
```
adb logcat
```

#### Linux平台(非RV1103/RV1106平台)
1. 创建目录,用于保存代理服务的详细日志
```
adb shell mkdir -p /userdata
```
2. 设置环境变量开启详细日志,并启动代理:
```
adb shell "killall rknn_server && export RKNN_SERVER_LOGLEVEL=5 && nohup  /usr/bin/rknn_server >/userdata/server.log"&
```
3. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令进入板子shell界面后,执行下列命令启动代理服务
```
export RKNN_SERVER_LOGLEVEL=5
nohup /usr/bin/rknn_server >/userdata/server.log&
exit
```
4. 运行PC上python推理程序
5. 查看日志
```
adb shell cat /userdata/server.log
```

#### Linux平台(RV1103/RV1106平台)
1. 创建目录,用于保存代理服务的详细日志
```
adb shell mkdir -p /userdata
```
2. 设置环境变量开启详细日志,并启动代理:
```
adb shell "export RKNN_SERVER_LOGLEVEL=5 && nohup  /oem/usr/bin/rknn_server >/userdata/server.log"&
```
3. 检查代理服务是否启动成功：
```
adb shell ps|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令进入板子shell界面后,执行下列命令启动代理服务
```
export RKNN_SERVER_LOGLEVEL=5
nohup /oem/usr/bin/rknn_server >/userdata/server.log&
exit
```
4. 运行PC上python推理程序
5. 查看日志
```
adb shell cat /userdata/server.log
```

**注意：**

- 遇到"E RKNN: failed to allocate fd, ret: -1, errno: 12"报错，可以在RV1103/RV1106上运行RkLunch-stop.sh，关闭其他占用内存的应用后再连板推理。
- RV1103/RV1106使用init_runtime python接口时，**不支持**perf_debug=True参数。
- accuracy_analysis python接口使用时，可能会因为模型太大，板子上存储容量不够导致运行失败，可以在板子上使用df -h命令来确认。
- 如果Android系统启动代理服务遇到权限问题,可以尝试使用adb shell setenforce 0命令来关闭selinux。


# FAQ
### 1. Debian系统上rknn_server服务已经后台启动, 但是连板推理时依旧有如下报错：
```
D NPUTransfer: ERROR: socket read fd = 4, n = -1: Connection reset by peer
D NPUTransfer: Transfer client closed, fd = 4
E RKNNAPI: rknn_init,  server connect fail!  ret = -9(ERROR_PIPE)!
E build_graph: The rknn_server on the concected device is abnormal, please start the rknn_server on the device according to:
               https://github.com/airockchip/rknn-toolkit2/blob/master/doc/rknn_server_proxy.md
```

**解决方法：**
这通常是由于Debian固件上的adbd程序没有监听5037端口导致的，可以在板子上执行以下命令来判断:
```
netstat -n -t -u -a
```
如果输出结果中没有5037端口,则执行下列命令下载和更新adbd程序, 并重启板子;否则,跳过下列步骤。
```
wget -O adbd.zip https://ftzr.zbox.filez.com/v2/delivery/data/7f0ac30dfa474892841fcb2cd29ad924/adbd.zip
unzip adbd.zip
adb push adbd/linux-aarch64/adbd /usr/bin/adbd
adb shell "chmod 755 /usr/bin/adbd"
```
重启后,启动rknn_server服务,再次尝试连板推理。
