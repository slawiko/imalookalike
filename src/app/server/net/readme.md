# Imlookalike model

### Dataset
[CelebA](http://mmlab.ie.cuhk.edu.hk/projects/CelebA.html) is used for training.

### Preprocessing
[Dlib library](http://dlib.net/) with [68 face lanmarks predictor](https://github.com/AKSHAYUBHAT/TensorFace/blob/master/openface/models/dlib/shape_predictor_68_face_landmarks.dat) is used for estimating 5 main landmarks (eyes, nose, left and right mouth points) 
for each input image, then it's cropped to 256 x 256 size with 0.7 face factor.

#### How to preprocess your images

```
./utils/align_images.py path/to/data/folder path/to/landmarks/shape_predictor_68_face_landmarks.dat
```

### Architecture

It's very simple CNN:

| Layer name             |
|------------------------|
| 32 Conv 7x7 + ReLU     |
| 32 Conv 7x7 + ReLU     |
| Pooling 4x4, strides 4 |
| Dropout 0.3            |
| 64 Conv 9x9 + ReLU     |
| 64 Conv 9x9 + ReLU     |
| Pooling 4x4, strides 4 |
| Dense 256              |
| Batchnorm + ReLU       |
| Dropout 0.4            |
| Dense 128              |

Last layer is output embedding.

[Batch all triplet loss](https://arxiv.org/abs/1503.03832) is used as loss function.

### Output Model 

Output model is stored in `resources` folder.

**Note:** due to time limit imlookalike model saw only 1/3 of training dataset (CelebA).

### How to apply model

```
./utils/apply_model.py path/to/images/folder path/to/iamlookalike/model/folder path/to/output/file
```  
