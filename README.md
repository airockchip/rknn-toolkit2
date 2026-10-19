# Description
  RKNN software stack can help users to quickly deploy AI models to Rockchip chips. The overall framework is as follows:
    <center class="half">
        <div style="background-color:#ffffff;">
        <img src="res/framework.png" title="RKNN"/>
    </center>

  In order to use RKNPU, users need to first run the RKNN-Toolkit2 tool on the computer, convert the trained model into an RKNN format model, and then inference on the development board using the RKNN C API or Python API.

- RKNN-Toolkit2 is a software development kit for users to perform model conversion, inference and performance evaluation on PC and Rockchip NPU platforms.

- RKNN-Toolkit-Lite2 provides Python programming interfaces for Rockchip NPU platform to help users deploy RKNN models and accelerate the implementation of AI applications.

- RKNN Runtime provides C/C++ programming interfaces for Rockchip NPU platform to help users deploy RKNN models and accelerate the implementation of AI applications.

- RKNPU kernel driver is responsible for interacting with NPU hardware. It has been open source and can be found in the Rockchip kernel code.

# Support Platform
  - RK3588 Series
  - RK3576 Series
  - RK3566/RK3568 Series
  - RK3562 Series
  - RV1103/RV1106
  - RV1103B/RV1106B
  - RV1126B
  - RK2118


Note:

​      **For RK1808/RV1109/RV1126/RK3399Pro, please refer to :**

​          https://github.com/airockchip/rknn-toolkit

​          https://github.com/airockchip/rknpu

​          https://github.com/airockchip/RK3399Pro_npu


# Download
- You can also download all packages, docker image, examples, docs and platform-tools from [RKNPU2_SDK](https://console.zbox.filez.com/l/I00fc3), fetch code: rknn
- You can get more examples from [rknn model zoo](https://github.com/airockchip/rknn_model_zoo)

# Notes
- RKNN-Toolkit2 is not compatible with [RKNN-Toolkit](https://github.com/airockchip/rknn-toolkit)
- The supported Python versions are:
  - Python 3.6
  - Python 3.7
  - Python 3.8
  - Python 3.9
  - Python 3.10
  - Python 3.11
  - Python 3.12
- Latest version:v2.5.0



# RKNN LLM

If you want to deploy LLM (Large Language Model), we have introduced a new SDK called RKNN-LLM. For details, please refer to:

https://github.com/airockchip/rknn-llm



# CHANGELOG

## v2.5.0

- Update RKNN model encryption 
- Support mini-runtime for RV1126B / RK3562 with `mini_rknn` model conversion 
- Optimize CPU load for full runtime for multi-input/output models 
- Support QAT sparse inference
- Improve operator support: GRU (NPU/GPU, multi-batch), GridSample (GPU), Sin/Cos, ReduceMax, CumSum, DepthToSpace, And, ReduceLogSumExp (CPU), Transpose, Reshape, Pow/TopK (CPU), exNorm, etc
- Support model conversion with/without first conv, enable core0 only and fp16 positve weight share option 
- Improve graph optimization: update model conversion and fusion rules for better conversion quality on some models
- MatMul API: support RV1126B and w4a8/w4a16 for RK3562  
- Fix runtime issues: dma-buf/fd and error-path memory leaks, bugfix for internal mem reuse    
- Improve operator support, including Conv, LSTM, Transpose, GRU, BN, ALU, Pooling for RK2118
- Bugfix
      

 for older version, please refer [CHANGELOG](CHANGELOG.md)

# Feedback and Community Support
- [Redmine](https://redmine.rock-chips.com) (**Feedback recommended, Please consult our sales or FAE for the redmine account**)
- QQ Group Chat: 1025468710 (full, please join group 5)
- QQ Group Chat2: 547021958 (full, please join group 5)
- QQ Group Chat3: 469385426 (full, please join group 5)
- QQ Group Chat4: 958083853 (full, please join group 5)
- QQ Group Chat5: 1077888690 
<center class="half">
  <img width="200" height="200"  src="res/QQGroupQRCode.png" title="QQ Group Chat"/>
  <img width="200" height="200"  src="res/QQGroup2QRCode.png" title="QQ Group Chat2"/>
  <img width="200" height="200"  src="res/QQGroup3QRCode.png" title="QQ Group Chat3"/>
  <img width="200" height="200"  src="res/QQGroup4QRCode.png" title="QQ Group Chat4"/>
  <img width="200" height="200"  src="res/QQGroup5QRCode.png" title="QQ Group Chat5"/>
</center>


