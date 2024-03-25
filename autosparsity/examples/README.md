# Sparse Infer

This tool is used for the Torch model to autosparsity the weights during the training, which can save model storage and reduce model inference time in RKNN sparse inference.

## Usage

### Step 1

Install autosparsity package

```bash
pip install ../packages/autosparsity-1.0-cp38-cp38m-linux_x86_64.whl
```

### Step 2

Taking ResNet50 in torchvision as an example to generate the sparse model.

```bash
python autosparsity.py
```
To sparsity a custom model, just add the sparsity_model functionwhen model training, as follows:

```python
# insert model autosparsity code before training
import torch
import torchvision.models as models
from autosparsity.sparsity import sparsity_model

...

model = models.resnet34(pretrained=True).cuda()
mode = 0
sparsity_model(model, optimizer, mode)

# normal training
x, y = DataLoader(args)
for epoch in range(epochs):
    y_pred = model(x)
    loss = loss_func(y_pred, y)
    loss.backward()
    optimizer.step()
    ...
```

- Note: Make sure CUDA is available


### Step3

Perfom sparse inference

```bash
python test.py
```
- Note: Only supports RK3576 target platform

### Expected Results:

This will print the , as follows:
```
-----TOP 5-----
[155] score:0.877372 class:"Shih-Tzu"
[283] score:0.042477 class:"Persian cat"
[ 82] score:0.006625 class:"ruffed grouse, partridge, Bonasa umbellus"
[154] score:0.006625 class:"Pekinese, Pekingese, Peke"
[204] score:0.004696 class:"Lhasa, Lhasa apso"
```
