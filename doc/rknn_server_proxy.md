**目录**

[TOC]

## 1. 连板调试简介
RKNN Toolkit2的连板功能一般需要更新板端的 rknn_server 和 librknnrt.so/librknnmrt.so，并且手动启动 rknn_server 才能正常工作。
rknn_server: 是一个运行在板子上的后台代理服务，用于接收PC通过USB传输过来的协议，然后执行板端runtime对应的接口，并返回结果给PC。

- librknnrt.so: 是一个板端的RKNPU Runtime库(非RV1103/RV1106/RV1103B平台不适用)。
- librknnmrt.so: 是专用于RV1103/RV1106/RV1103B平台的RKNPU Runtime库。

开机后通过ps命令查看rknn_server进程是否已存在，如果已存在，则不需要手动启动，否则需要手动启动。

## 2. 环境要求
### 2.1 硬件环境
本文档适用如下硬件平台：

- RV1103
- RV1103B
- RV1106
- RV1126B
- RK3562
- RK3566系列
- RK3568系列
- RK3576系列
- RK3588系列

### 2.2 软件环境
- 若使用动态形状输入RKNN模型，要求rknn_server和RKNPU Runtime库版本>=1.5.0。
- 若使用大于2GB的模型，要求rknn_server版本>=2.0.0b0。
- 在RV1103/RV1106/RV1103B等小内存平台上，建议rknn_server版本>=2.1.0。


## 3. rknn_server存放目录
rknn_server存放在runtime目录下, 请根据板子上的系统选择相应版本的rknn_server,不同芯片和系统对应的rknn_server路径如下：
### 3.1 Android平台
|系统|路径|
|-----|-----|
|32-bit Android|runtime/Android/rknn_server/arm/rknn_server|
|64-bit Android|runtime/Android/rknn_server/arm64/rknn_server|


### 3.2 Linux平台

|芯片|系统|路径|
|-----|-----|-----|
|RV1103/RV1106/RV1103B|32-bit Linux|runtime/Linux/rknn_server/armhf-uclibc/usr/bin/rknn_server|
|其他芯片|32-bit Linux|runtime/Linux/rknn_server/armhf/usr/bin/rknn_server|
|其他芯片|64-bit Linux|runtime/Linux/rknn_server/aarch64/usr/bin/rknn_server|

## 4. 启动步骤
### 4.1 Android平台
进入rknpu2工程的根目录，在PC端执行下列命令启动代理服务:
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
adb shell "killall rknn_server"
adb shell "nohup /vendor/bin/rknn_server >/dev/null"&
```
6. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,步骤如下:
adb shell命令**进入板子shell界面**后,执行下列命令启动代理服务

```
nohup /vendor/bin/rknn_server > /dev/null&
```

### 4.2 Linux平台(非RV1103/RV1106/RV1103B)
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
adb shell "killall rknn_server"
adb shell "nohup /usr/bin/rknn_server >/dev/null"&
```
5. 检查代理服务是否启动成功：
```
adb shell ps -ef|grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令**进入板子shell界面**后,执行下列命令启动代理服务

```
nohup /usr/bin/rknn_server > /dev/null&
```

### 4.3Linux平台(RV1103/RV1106/RV1103B)
RV1103/RV1106/RV1103B上使用的RKNPU Runtime库是librknnmrt.so，使用armhf-uclibc目录下的rknn_server,启动步骤如下：
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
adb shell "killall rknn_server"
adb shell "nohup /oem/usr/bin/rknn_server >/dev/null"&
```
5. 检查代理服务是否启动成功：
```
adb shell ps |grep rknn_server
```
查看是否有`rknn_server`的进程id,如果存在表示代理服务启动成功;否则请在板子上手动启动代理服务,方法如下:
adb shell命令**进入板子shell界面**后,执行下列命令启动代理服务

```
nohup /oem/usr/bin/rknn_server > /dev/null&
```

## 5. 查看rknn_server详细日志
代理服务默认不开启详细日志，如遇到连板过程报错，需开启详细日志来定位错误原因，请参考相应平台执行步骤：
### 5.1 Android平台
1. 设置环境变量开启详细日志,并启动代理:
```
adb logcat -c
adb shell "killall rknn_server"
adb shell "setprop persist.vendor.rknn.server.log.level 5 && nohup /vendor/bin/rknn_server >/dev/null"&
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

5. 开启详细日志会导致推理速度变慢，恢复默认日志等级的命令如下：

```sh
adb shell "killall rknn_server"
adb shell "setprop persist.vendor.rknn.server.log.level 0 && nohup /vendor/bin/rknn_server >/dev/null"&
```

### 5.2 Linux平台(非RV1103/RV1106/RV1103B)

1. 创建目录,用于保存代理服务的详细日志
```
adb shell mkdir -p /userdata
```
2. 设置环境变量开启详细日志,并启动代理:
```
adb shell "killall rknn_server"
adb shell "export RKNN_SERVER_LOGLEVEL=5 && nohup /usr/bin/rknn_server >/userdata/server.log"&
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

6. 开启详细日志会导致推理速度变慢，恢复默认日志等级的命令如下：

```sh
   adb shell "killall rknn_server"
   adb shell "export RKNN_SERVER_LOGLEVEL=0 && nohup /usr/bin/rknn_server >/userdata/server.log"&
```

### 5.3 Linux平台(RV1103/RV1106/RV1103B)

1. 创建目录,用于保存代理服务的详细日志
```
adb shell mkdir -p /userdata
```
2. 设置环境变量开启详细日志,并启动代理:
```
adb shell "killall rknn_server"
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

6. 开启详细日志会导致推理速度变慢，恢复默认日志等级的命令如下：

```sh
adb shell "killall rknn_server"
adb shell "export RKNN_SERVER_LOGLEVEL=0 && nohup  /oem/usr/bin/rknn_server >/userdata/server.log"&
```

## 6. 常见问题
### 问题1
Debian系统上rknn_server服务已经后台启动, 但是连板推理时依旧有如下报错：
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
```
进入设备shell命令，增加adbd的可执行权限

```sh
adb shell "chmod +x /usr/bin/adbd"
adb reboot
```

重启设备后，按照启动步骤启动rknn_server服务，再次尝试连板推理。


### 问题2
在RV1103/RV1106/RV1103B设备上遇到"E RKNN: failed to allocate fd, ret: -1, errno: 12"或者OOM报错。
**解决方法：**
在RV1103/RV1106/RV1103B设备上运行RkLunch-stop.sh，关闭其他占用内存的应用，并将rknn_server升级到>=2.1.0版本后再连板推理。

### 问题3
RV1103/RV1106/RV1103B使用init_runtime python接口时，没有逐层的耗时。
原因：RV1103/RV1106/RV1103B上**不支持**perf_debug=True参数。

### 问题4
RV1103/RV1106/RV1103B使用accuracy_analysis python接口使用时，因为模型太大，板子上存储容量不够导致运行失败。

**解决方法：**

在设备端df -h查看存储空间情况，假设设备存储情况如下：

```sh
Filesystem                Size      Used Available Use% Mounted on
ubi0:rootfs              17.0M     14.1M      2.9M  83% /
devtmpfs                 27.4M         0     27.4M   0% /dev
tmpfs                    27.5M         0     27.5M   0% /dev/shm
tmpfs                    27.5M      8.0K     27.5M   0% /tmp
tmpfs                    27.5M     84.0K     27.4M   0% /run
/dev/ubi5_0              20.6M     14.5M      6.1M  70% /oem
/dev/ubi6_0              52.8M     24.5M     28.2M  46% /userdata

```

从上面看出/userdata/目录的可用空间最大，故将/userdata/目录作为精度分析时保存中间结果文件的目录，再重启rknn_server，PC上执行命令如下：

```
adb shell "killall rknn_server"
adb shell "export RKNN_DUMP_DIR=/userdata/dumps && nohup /usr/bin/rknn_server >/dev/null"&
```

RV1103/RV1106/RV1103B的rknn_server默认的保存中间结果路径是/tmp/dumps，如果需要恢复默认路径，请执行下列命令：

```adb shell "killall rknn_server"
adb shell "killall rknn_server"
adb shell "unset RKNN_DUMP_DIR && nohup /usr/bin/rknn_server >/dev/null"&
```

如果设备存储空间不足，并且RV1103/RV1106/RV1103B固件支持NFS挂载，则可以将dump目录挂载到NFS目录后，再执行精度分析。假设目标服务器的IP为192.168.1.1，服务器NFS目录是/data0/nfs_shared，则在设备上执行下列命令将/userdata/dumps挂载到NFS目录

```
# 以241为例
mkdir -p /userdata/dumps
mount -t nfs -o nolock 192.168.1.1:/data0/nfs_shared /userdata/dumps
```

### 问题5

使用adb命令更新Runtime库后没有生效。

**解决方法**：

更新库后要重新启动rknn_server，下次连板推理才能生效。如果是RV1103/RV1106/RV1103B平台，确保将librknnmrt.so放在/oem/usr/lib目录，如果/usr/lib/目录下有同名的库，要删除/usr/lib/下的librknnmrt.so，并重启rknn_server。

### 问题6

Android系统启动代理服务遇到权限问题。

**解决方法**：

使用adb shell setenforce 0命令来关闭selinux。
