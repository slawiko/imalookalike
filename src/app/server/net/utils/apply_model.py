import sys
import os
import numpy as np
import concurrent.futures
import io
import cv2
from net.prediction import predictor


def process_batch(img_keys, input_dir, output_file):
    cv2_imgs = []
    for img_key in img_keys:
        img = cv2.imread(input_dir + "/" + img_key)
        cv2_imgs.append(img)

    embeddings = predictor.predict_embeddings(cv2_imgs)

    with io.open(output_file, 'a') as f:
        for id, embedding in zip(img_keys, embeddings):
            embedding_as_str = ",".join(map(str, embedding))
            f.write('%s,%s\n' % (id, embedding_as_str))


def process_images(input_dir, model_dir, output_file, batch_size=30, num_workers=5):
    img_keys = np.array(os.listdir(input_dir))

    predictor.init_model(model_dir)
    batch_num = len(img_keys) // batch_size
    img_keys_batched = np.array_split(img_keys, batch_num)

    with concurrent.futures.ThreadPoolExecutor(max_workers=num_workers) as executor:
        for img_keys in img_keys_batched:
            executor.submit(process_batch, img_keys, input_dir, output_file)


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("input directory, model directory and output file should be provided!")
        exit(1)
    input_dir = sys.argv[1]
    model_dir = sys.argv[2]
    output_file = sys.argv[3]

    process_images(input_dir, model_dir, output_file)
