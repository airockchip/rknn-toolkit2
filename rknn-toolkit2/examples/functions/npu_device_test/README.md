# How to connect the board for debugging

## Model Source
The model used in this example come from the following open source projects:  
https://github.com/shicai/MobileNet-Caffe

## Script Usage
*Usage:*
```
python test.py
```
*Description:*
- The default target platform in script is 'rk3588', please modify the 'target_platform' parameter of 'rknn.config' according to the actual platform.
- The 'target' parameter in 'rknn.init_runtime'/'rknn.accuracy_analysis' is set to 'rk3588'.
- The 'perf_debug' and 'eval_mem' parameter of 'rknn.config' is set to True.

## Expected Results
This example would output the following results:
- List devices that connect to host
    ```
    *************************
    all device(s) with adb mode:
    rk3588
    *************************
    ```
- Print version
    ```
    ==============================================
    RKNN VERSION:
    API: librknn_api version: 1.5.2 (8babfea build@2023-08-25T02:31:12)
    DRV: rknn_server: 1.5.2 (8babfea build@2023-08-25T10:30:12)
        rknnrt: 1.5.3b9 (ba0047cf1@2023-10-11T19:55:40)
    ==============================================
    ```
- Print perf information
    ```
    ===================================================================================================================
                                Performance                              
            #### The performance result is just for debugging, ####
            #### may worse than actual performance!            ####
    ===================================================================================================================
    ID   OpType           DataType Target InputShape                                   OutputShape            DDR Cycles     NPU Cycles     Total Cycles   Time(us)       MacUsage(%)    WorkLoad(0/1/2)-ImproveTherical        Task Number    Lut Number     RW(KB)         FullName
    1    InputOperator    UINT8    CPU    \                                            (1,3,224,224)          0              0              0              7              \              0.0%/0.0%/0.0% - Up:0.0%               0              0              147.00         InputOperator:data
    2    ConvRelu         UINT8    NPU    (1,3,224,224),(32,3,3,3),(32)                (1,32,112,112)         94150          10584          94150          428            2.47           100.0%/0.0%/0.0% - Up:0.0%             3              0              543.75

    ...

    Total Operator Elapsed Time(us): 14147
    Total Memory RW Amount(MB): 0
    Operator Time-Consuming Ranking:
    OpType           Call Number     CPU Time(us)    NPU Time(us)    Total Time(us)    Time Ratio(%)
    ConvRelu         36              0               9338            9338              66.0
    ConvAdd          10              0               2155            2155              15.23
    Conv             9               0               2097            2097              14.82
    exDataConvert    1               0               269             269               1.9
    Softmax          1               0               248             248               1.75
    OutputOperator   1               33              0               33                0.23
    InputOperator    1               7               0               7                 0.04

    ===================================================================================================================
    ```
- Print memory information
    ```
    ======================================================
                Memory Profile Info Dump                  
    ======================================================
    NPU model memory detail(bytes):
        Total Weight Memory: 3.53 MiB
        Total Internal Tensor Memory: 1.67 MiB
        Total Memory: 5.66 MiB

    INFO: When evaluating memory usage, we need consider  
    the size of model, current model size is: 4.08 MiB
    ======================================================
        ```
- Print the TOP5 labels and corresponding scores of the test image classification results
    ```
    -----TOP 5-----
    [155] score:0.936523 class:"Shih-Tzu"
    [154] score:0.002403 class:"Pekinese, Pekingese, Peke"
    [204] score:0.002403 class:"Lhasa, Lhasa apso"
    [283] score:0.000658 class:"Persian cat"
    [284] score:0.000229 class:"Siamese cat, Siamese"
    ```
- Output the results of the accuracy analysis:
    ```
    # simulator_error: calculate the output error of each layer of the simulator (compared to the 'golden' value).
    #              entire: output error of each layer between 'golden' and 'simulator', these errors will accumulate layer by layer.
    #              single: single-layer output error between 'golden' and 'simulator', can better reflect the single-layer accuracy of the simulator.
    # runtime_error: calculate the output error of each layer of the runtime.
    #              entire: output error of each layer between 'golden' and 'runtime', these errors will accumulate layer by layer.
    #              single_sim: single-layer output error between 'simulator' and 'runtime', can better reflect the single-layer accuracy of runtime.

    layer_name                                simulator_error                             runtime_error                      
                                        entire              single                  entire           single_sim            
                                    cos      euc        cos      euc            cos      euc        cos      euc          
    -----------------------------------------------------------------------------------------------------------------
    [Input] data                     1.00000 | 0.0       1.00000 | 0.0           
    [exDataConvert] data_int8        0.99999 | 2.0599    0.99999 | 2.0599        
    [Conv] conv1/scale               
    [Relu] relu1                     0.99997 | 4.7572    0.99997 | 4.7572        0.99996 | 5.2593    0.99994 | 6.3273        
    [Conv] conv2_1/expand/scale      
    [Relu] relu2_1/expand            0.99993 | 6.8963    0.99994 | 6.3494        0.99992 | 7.1564    1.00000 | 1.0312        
    [Conv] conv2_1/dwise/scale       
    [Relu] relu2_1/dwise             0.99949 | 22.639    0.99958 | 20.480        0.99949 | 22.665    1.00000 | 1.4891        

    ...

    [Relu] relu6_4                   0.97924 | 64.548    0.99989 | 4.7152        0.98151 | 60.976    1.00000 | 0.2890        
    [Conv] pool6                     0.98537 | 4.8954    0.99997 | 0.2290        0.98766 | 4.5005    1.00000 | 0.0           
    [Conv] fc7                       0.98323 | 21.938    0.99990 | 1.6519        0.98521 | 20.567    1.00000 | 0.0           
    [exDataConvert] fc7__float16     0.98323 | 21.939    0.99996 | 1.1046        0.98521 | 20.570    1.00000 | 0.0512        
    [Softmax] prob                   1.00000 | 0.0010    1.00000 | 0.0001        1.00000 | 0.0569    1.00000 | 0.0566 
    ```
- Note: Different platforms, different versions of tools and drivers may have slightly different results.