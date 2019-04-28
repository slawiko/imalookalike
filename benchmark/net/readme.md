# Imlookalike vs Facenet 

### Data
[CelebA](http://mmlab.ie.cuhk.edu.hk/projects/CelebA.html) validation part of dataset was used for comparison.

### Facenet model
Following pretrained model was used for comparing - [20180402-114759](https://drive.google.com/file/d/1EXPBSXwTaqrSC0OhUdXNmKSh9qJUQ55-/view).
Code from facenet [github](https://github.com/davidsandberg/facenet) was used to apply alignment and model itself.

### Results

| Model name      | Avg Accuracy |
|-----------------|--------------|
| Facenet         | 0.85064      |
| Imlookalike     | 0.9885       |

**Note:** due to time limit imlookalike model saw only 1/3 of training dataset (CelebA).

### How to perform comparison

```
./compare_models.py path/to/data/folder path/to/labels-file.txt path/to/evaluation-partitions-file.txt path/to/iamlookalike/model/folder path/to/facenet/model.pb
```  
For labels and evaluation partitions file format details go to [CelebA](http://mmlab.ie.cuhk.edu.hk/projects/CelebA.html).
