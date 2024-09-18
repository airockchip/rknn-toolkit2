# Using RKNN-ToolKit2 in WSL

## 1. Installing WSL on Windows Host
- You can refer to the following link for installing WSL: https://learn.microsoft.com/en-us/windows/wsl/install

## 2. Using RKNN-Toolkit2 in WSL
1. Refer to the document[《Rockchip_RKNPU_Quick_Start》](https://github.com/airockchip/rknn-toolkit2/blob/master/doc/) to install the RKNN-ToolKit2 environment in WSL
2. Refer to the document[《Rockchip_RKNPU_User_Guide》](https://github.com/airockchip/rknn-toolkit2/blob/master/doc/) for model conversion, quantization, and other operations in WSL.

## 3. Using RKNN-Toolkit2 for Board Debugging in WSL

### 3.1. Installing adb in WSL Terminal
```
sudo apt update
sudo apt install adb
```

### 3.2 Connecting the Device in WSL
You can choose to connect the device through Ethernet or USB.

#### 3.2.1 Ethernet Connection
```
# 1. Connect the device using Ethernet cable.

# 2. In WSL, use adb connect to connect to the device.
adb connect <IP address:port>
```
`IP address` is the IP address of the board.


#### 3.2.2 USB Connection
```
# 1. Connect the device to Windows via USB.

# 2. Use the adbkit tool in Windows to convert USB to TCP.
npm install --save adbkit
adbkit usb-device-to-tcp <device_id> -p <port>

# Note: Configure WSL to access Windows network.

# 3. In WSL, use adb connect to connect to the device.
adb connect <IP address:port>
```
- `device_id` can be obtained by running the `adb devices` command in Windows (adb tool must be installed in Windows).
- `IP address` is the IP address of the Windows host.

### 3.3 Using RKNN-Toolkit2 for Board Debugging
Refer to the document [《Rockchip_RKNPU_User_Guide》](https://github.com/airockchip/rknn-toolkit2/blob/master/doc/) for board inference, accuracy analysis, and other operations in WSL.



## NOte:
1. It is recommended to install WSL2 with Ubuntu version 22.04, which has been verified to work (other versions are theoretically feasible but not tested).
2. If you encounter the "ImportError: libGL.so.1: cannot open shared object file: No such file or directory" error while using RKNN-ToolKit2 in WSL, execute the following code to resolve it:
```
1. Install the required library:
sudo apt update
sudo apt install libgl1-mesa-glx

2. Set the environment variable:
echo 'export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu/mesa' >> ~/.bashrc
source ~/.bashrc
```