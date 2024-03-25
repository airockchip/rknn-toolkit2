## AutoSparsity

Enables sparse training and inference for PyTorch models.


## Usage

### Step 1

Install autosparsity package

```bash
pip install packages/autosparsity-1.0-cp38-cp38m-linux_x86_64.whl
```

### Step 2

Taking ResNet50 in torchvision as an example to generate the sparse model.

```bash
python examples/autosparsity.py
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

Use RKNN-Toolkite to perfom sparse inference

```bash
python examples/test.py
```
- Note: Only supports RK3576 target platform

