import numpy as np
import os
import csv
from PIL import Image


def load_image_keys(dataset_folder):
    return np.array(sorted(os.listdir(dataset_folder)))


def load_anno_from_file(image_keys, filename):
    anno = []
    counter = 0
    with open(filename, 'r') as csvFile:
        reader = csv.reader(csvFile, delimiter=' ')
        for row in reader:
            if row[0] == image_keys[counter]:
                counter += 1
                anno.append(row[1])
    return np.array(anno, dtype=int)


def load_images_from_folder(folder, image_keys):
    images = []
    for filename in image_keys:
        img = Image.open(os.path.join(folder, filename))
        images.append(np.asarray(img))
    return np.array(images, dtype=np.float32)
