# RKNNToolkit2 API Difference With Toolkit1

## rknn.config
- Toolkit1:
  
        config(batch_size=100,                                  # abandoned
               caffe_mean_file=None,                            # abandoned
               dtype='float32',                                 # abandoned
               epochs=-1,                                       # abandoned
               force_gray=None,                                 # abandoned
               input_fitting='scale',                           # abandoned
               input_normalization=None,                        # abandoned
               mean_file=None,                                  # abandoned
               model_data_format='zone',                        # abandoned
               model_quantize=None,                             # abandoned
               optimize='Default',                              # abandoned
               quantized_dtype='asymmetric_affine-u8',
               quantized_moving_alpha=0.01,                     # abandoned
               quantized_algorithm='normal',
               quantized_divergence_nbins=1024,                 # abandoned
               mmse_epoch=3,                                    # abandoned
               random_brightness=None,                          # abandoned
               random_contrast=None,                            # abandoned
               random_crop=None,                                # abandoned
               random_flip=None,                                # abandoned
               random_mirror=None,                              # abandoned
               reorder_channel=None,                            # abandoned
               restart=False,                                   # abandoned
               samples=-1,                                      # abandoned
               need_horizontal_merge=False,                     # abandoned
               deconv_merge=True,                               # abandoned
               conv_mul_merge=True,                             # abandoned
               quantized_hybrid=False,                          # abandoned
               output_optimize=0,                               # abandoned
               remove_tensorflow_output_permute=False,          # abandoned
               optimization_level=3,
               target_platform=None,
               mean_values=None,
               std_values=None,
               channel_mean_value=None,                         # abandoned
               force_builtin_perm=False,                        # abandoned
               do_sparse_network=True,                          # abandoned
               merge_dequant_layer_and_output_node=False,       # abandoned
               quantize_input_node=False,                       # abandoned
               inputs_scale_range=None)                         # abandoned

- Toolkit2:

        config(mean_values=None,
               std_values=None,
               quantized_dtype='asymmetric_quantized-8',
               quantized_algorithm='normal',
               quantized_method='channel',                      # new
               target_platform=None,
               quant_img_RGB2BGR=False,                         # new
               float_dtype='float16',                           # new
               optimization_level=3,
               custom_string=None,                              # new
               remove_weight=False,                             # new
               compress_weight=False,                           # new
               inputs_yuv_fmt=None,                             # new
               single_core_mode=False)                          # new

- In addition to the above abandoned/new items, there are other differences:

        quantized_dtype:
            toolkit1: asymmetric_affine-u8, dynamic_fixed_point-i8, dynamic_fixed_point-i16
            toolkit2: asymmetric_quantized-8
        quantized_algorithm:
            toolkit1: normal(default), mmse, kl_divergence, moving_average
            toolkit2: normal(default), mmse
        target_platform:
            toolkit1: rk1808, rk3399pro, rv1109, rv1126
            toolkit2: rk3566, rk3568, rk3588, rk3588s, rv1103, rv1106, rk3562 and newer

## rknn.load_tensorflow
- Toolkit1:
        
        load_tensorflow(tf_pb,
                        inputs,
                        input_size_list,
                        outputs, 
                        predef_file=None,                       # abandoned
                        mean_values=None,                       # abandoned
                        std_values=None,                        # abandoned
                        size_with_batch=None)                   # abandoned
                        
- Toolkit2:

        load_tensorflow(tf_pb,
                        inputs,
                        input_size_list,
                        outputs)

- In addition to the above abandoned items, there are other differences:

        inputs:
            toolkit1: node list (layer name)
            toolkit2: node list (operand name)
        outputs:
            toolkit1: node list (layer name)
            toolkit2: node list (operand name)

## rknn.load_caffe
- Toolkit1:
        
        load_caffe(model,
                   proto,                                       # abandoned
                   blobs=None)
- Toolkit2:

        load_caffe(model,
                   blobs=None,
                   input_name=None)                             # new


## rknn.load_keras
- Toolkit1:
        
        load_keras(model, convert_engine='Keras')
- Toolkit2:

        Not supported yet!

## rknn.load_pytorch
- Toolkit1:
        
        load_pytorch(model,
                     input_size_list=None, 
                     inputs=None,                               # abandoned
                     outputs=None,                              # abandoned
                     convert_engine='torch')                    # abandoned
- Toolkit2:

        load_pytorch(model,
                     input_size_list)

## rknn.load_mxnet
- Toolkit1:
        
        load_mxnet(symbol, params, input_size_list=None)
- Toolkit2:

        Not supported yet!

## rknn.build
- Toolkit1:
        
        build(do_quantization=True, 
              dataset='dataset.txt',
              pre_compile=False,                                # abandoned
              rknn_batch_size=-1)
- Toolkit2:

        build(do_quantization=True,
              dataset='dataset.txt',
              rknn_batch_size=-1):


## rknn.direct_build
- Toolkit1:
        
        direct_build(model_input, data_input, model_quantize=None, pre_compile=False)
- Toolkit2:

        Not supported yet!

## rknn.hybrid_quantization_step1
- Toolkit1:
        
        hybrid_quantization_step1(dataset=None)
- Toolkit2:

        hybrid_quantization_step1(dataset=None,
                                  rknn_batch_size=-1,           # new
                                  proposal=False,               # new
                                  proposal_dataset_size=1)      # new


## rknn.hybrid_quantization_step2
- Toolkit1:
        
        hybrid_quantization_step2(model_input,
                                  data_input,
                                  model_quantization_cfg,
                                  dataset,                      # abandoned
                                  pre_compile=False)            # abandoned
- Toolkit2:

        hybrid_quantization_step2(model_input,
                                  data_input,
                                  model_quantization_cfg)


## rknn.accuracy_analysis
- Toolkit1:
        
        accuracy_analysis(inputs,
                          output_dir='./snapshot', 
                          calc_qnt_error=True,                  # abandoned
                          target=None,
                          device_id=None,
                          dump_file_type='tensor')              # abandoned
- Toolkit2:

        accuracy_analysis(inputs,
                          output_dir='./snapshot', 
                          target=None,
                          device_id=None)


## rknn.load_rknn
- Toolkit1:
        
        load_rknn(path, 
                  load_model_in_npu=False)                      # abandoned
- Toolkit2:

        load_rknn(path)


## rknn.export_rknn
- Toolkit1:
        
        export_rknn(export_path)
- Toolkit2:

        export_rknn(export_path,
                    **kwargs)                                   # new

## rknn.load_firmware
- Toolkit1:
        
        load_firmware(fw_dir=None)
- Toolkit2:

        Not supported yet!


## rknn.init_runtime
- Toolkit1:
        
        init_runtime(target=None,
                     target_sub_class=None,
                     device_id=None,
                     perf_debug=False,
                     eval_mem=False,
                     async_mode=False,
                     rknn2precompile=False)                     # abandoned
- Toolkit2:

        init_runtime(target=None,
                     target_sub_class=None,
                     device_id=None,
                     perf_debug=False,
                     eval_mem=False,
                     async_mode=False)

- In addition to the above abandoned items, there are other differences:

        target:
            toolkit1: None(simulator), RK3399Pro, RK1808
            toolkit2: None(simulator), RK3566, RK3568, RK3588, RK3562
        


## rknn.inference
- Toolkit1:
        
        inference(inputs,
                  data_type=None,                               # abandoned
                  data_format=None,
                  inputs_pass_through=None,
                  get_frame_id=False)
- Toolkit2:

        inference(inputs,
                  data_format=None,
                  inputs_pass_through=None,
                  get_frame_id=False)


## rknn.eval_perf
- Toolkit1:
        
        eval_perf(inputs=None,                                  # abandoned
                  data_type=None,                               # abandoned
                  data_format=None,                             # abandoned
                  is_print=True,
                  loop_cnt=1)                                   # abandoned
- Toolkit2:

        eval_perf(is_print=True)


## rknn.export_rknn_precompile_model
- Toolkit1:
        
        export_rknn_precompile_model(export_path=None)
- Toolkit2:

        Abandoned


## rknn.export_rknn_sync_model
- Toolkit1:
        
        export_rknn_sync_model(input_model=None, sync_uids=None, output_model=None)
- Toolkit2:

        Abandoned


## rknn.register_op
- Toolkit1:
        
        register_op(op_path)
- Toolkit2:

        Not supported yet


## rknn.fetch_rknn_model_config
- Toolkit1:
        
        fetch_rknn_model_config(model_path)
- Toolkit2:

        Not supported yet


## rknn.list_support_target_platform
- Toolkit1:
        
        list_support_target_platform(rknn_model=None)
- Toolkit2:

        Not supported yet
