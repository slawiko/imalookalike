import numpy as np
import tensorflow as tf
import sys
from benchmark.net import facenet_predictor as facenet
from net.prediction import predictor as custom
from net.utils.file_utils import load_images_from_folder, load_image_keys, load_anno_from_file
from net.training.triplet_loss import batch_all_triplet_loss


margin = 0


def init_models(custom_model_path, facenet_model_path):
    custom.init_model(custom_model_path)
    facenet.init_model(facenet_model_path)


def get_eval_data(data_dir, labels_path, partitions_path):
    image_keys = load_image_keys(data_dir)
    labels = load_anno_from_file(image_keys, labels_path)
    labels = labels - np.ones(labels.shape)
    eval_partitions = load_anno_from_file(image_keys, partitions_path)

    eval_indices = [i for i, label in enumerate(eval_partitions) if label not in (0, 1)]

    return image_keys, labels, eval_indices


def compare_models(data_dir, image_keys, labels, eval_indices):
    eval_count_per_iteration = len(eval_indices) // 100
    indices = np.random.choice(eval_indices, eval_count_per_iteration)
    eval_data = load_images_from_folder(data_dir, image_keys[indices])
    eval_labels = labels[indices]

    custom_embeddings = custom.predict_embeddings(eval_data)
    facenet_embeddings = facenet.predict_embeddings(eval_data)

    _, custom_stat = batch_all_triplet_loss(tf.convert_to_tensor(eval_labels), tf.convert_to_tensor(custom_embeddings),
                                            margin)
    _, facenet_stat = batch_all_triplet_loss(tf.convert_to_tensor(eval_labels), tf.convert_to_tensor(facenet_embeddings),
                                            margin)

    sess = tf.Session()

    custom_acc = 1 - custom_stat.eval(session=sess)
    facenet_acc = 1 - facenet_stat.eval(session=sess)

    return custom_acc, facenet_acc


def run(data_dir, labels_path, partitions_path, custom_model_path, facenet_model_path, num_exp=3):
    image_keys, labels, eval_indices = get_eval_data(data_dir, labels_path, partitions_path)
    init_models(custom_model_path, facenet_model_path)

    custom_avg_acc, facenet_avg_acc = 0, 0
    for _ in range(num_exp):
        custom_acc, facenet_acc = compare_models(data_dir, image_keys, labels, eval_indices)
        custom_avg_acc += custom_acc
        facenet_avg_acc += facenet_acc

    print("Custom average accuracy: {}".format(custom_avg_acc / num_exp))
    print("Facenet average accuracy: {}".format(facenet_avg_acc / num_exp))


if __name__ == "__main__":
    if len(sys.argv) < 6:
        print("You must provide all args")
        sys.exit(1)
    DATA_DIR = sys.argv[1]
    LABELS_FILE = sys.argv[2]
    EVAL_PARTITIONS_FILE = sys.argv[3]
    CUSTOM_MODEL_DIR = sys.argv[4]
    FACENET_MODEL_DIR = sys.argv[5]

    run(DATA_DIR, LABELS_FILE, EVAL_PARTITIONS_FILE, CUSTOM_MODEL_DIR, FACENET_MODEL_DIR)
