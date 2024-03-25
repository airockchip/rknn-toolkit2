## Codegen

### 1.Introduction

RKNN-Toolkit2 support using `rknn.codegen` to generate cpp deploy demo.



### 2.How to use

1.After "**export_rknn**", calling "**codegen**" interface to generate cpp demo as followed:

```python
rknn.codegen(output_path='./rknn_app_demo', inputs=['./dog_224x224.jpg'], overwrite=False)
```

- If overwrite set False and './deploy_demo' already exists, skip generation.
- Inputs feed a list of input file. File could be **jpg/png/bmp/npy**.



2.After calling "**codegen**", a folder named "**./rknn_app_demo**" should be created. Then follow the introduction of "**./rknn_app_demo/README.md**" to execute the demo.



