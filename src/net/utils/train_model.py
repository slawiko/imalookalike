#!/usr/bin/env python3
import numpy as np
import tensorflow as tf
from PIL import Image
import sys
import os
import csv

tf.logging.set_verbosity(tf.logging.INFO)

DATA_DIR = ''
LABELS_FILE = ''
EVAL_PARTITIONS_FILE = ''
MODEL_DIR = 'model'
TMP_MODEL_DIR = '/tmp/mnist_convnet_model'

image_size = 256
full_num_classes = 10172
num_classes = 10172
batch_size = 40
embedding_size = 128


def cnn_model_fn(features, labels, mode):
    input_layer = features["x"]
    # labels = tf.reshape(labels, [-1])

    conv1 = tf.layers.conv2d(
        inputs=input_layer,
        filters=32,
        kernel_size=[7, 7],
        padding="same",
        activation=tf.nn.relu)

    conv2 = tf.layers.conv2d(
        inputs=conv1,
        filters=32,
        kernel_size=[7, 7],
        padding="same",
        activation=tf.nn.relu)

    pool1 = tf.layers.max_pooling2d(inputs=conv2, pool_size=[4, 4], strides=4)

    dropout1 = tf.layers.dropout(
        inputs=pool1, rate=0.3, training=mode == tf.estimator.ModeKeys.TRAIN)

    conv3 = tf.layers.conv2d(
        inputs=dropout1,
        filters=64,
        kernel_size=[9, 9],
        padding="same",
        activation=tf.nn.relu)

    conv4 = tf.layers.conv2d(
        inputs=conv3,
        filters=64,
        kernel_size=[9, 9],
        padding="same",
        activation=tf.nn.relu)

    pool2 = tf.layers.max_pooling2d(inputs=conv4, pool_size=[4, 4], strides=4)

    pool2_flat = tf.reshape(pool2, [-1, (image_size // 16) * (image_size // 16) * 64])

    dense = tf.layers.dense(inputs=pool2_flat, units=256, activation=None)

    batch_norm = tf.layers.batch_normalization(
        inputs=dense, training=mode == tf.estimator.ModeKeys.TRAIN)

    relu = tf.nn.relu(batch_norm)

    dropout2 = tf.layers.dropout(
        inputs=relu, rate=0.4, training=mode == tf.estimator.ModeKeys.TRAIN)

    # embedding = tf.layers.dense(inputs=dropout2, units=embedding_size)
    #
    # if mode == tf.estimator.ModeKeys.PREDICT:
    #     predictions = {"embedding": embedding}
    #     return tf.estimator.EstimatorSpec(mode=mode, predictions=predictions)
    #
    # logits = tf.layers.dense(inputs=embedding, units=num_classes)
    # classes = tf.argmax(input=logits, axis=1)
    #
    # onehot_labels = tf.one_hot(indices=tf.cast(labels, tf.int32), depth=num_classes)
    # loss = tf.losses.softmax_cross_entropy(
    #     onehot_labels=onehot_labels, logits=logits)
    #
    # tf.summary.scalar("loss", loss)
    #
    # if mode == tf.estimator.ModeKeys.TRAIN:
    #     update_ops = tf.get_collection(tf.GraphKeys.UPDATE_OPS)
    #     global_step = tf.train.get_global_step()
    #     optimizer = tf.train.AdamOptimizer(learning_rate=0.001)
    #     with tf.control_dependencies(update_ops):
    #         train_op = optimizer.minimize(loss, global_step=global_step)
    #     return tf.estimator.EstimatorSpec(mode=mode, loss=loss, train_op=train_op)
    # else:
    #     eval_metric_ops = {
    #         "accuracy": tf.metrics.accuracy(labels=labels, predictions=classes)
    #     }
    #     return tf.estimator.EstimatorSpec(
    #         mode=mode, loss=loss, eval_metric_ops=eval_metric_ops)

    embedding = tf.layers.dense(inputs=dropout2, units=embedding_size)

    if mode == tf.estimator.ModeKeys.PREDICT:
        predictions = {"embedding": embedding}
        return tf.estimator.EstimatorSpec(mode=mode, predictions=predictions)

    loss = tf.contrib.losses.metric_learning.triplet_semihard_loss(labels, embedding)

    tf.summary.scalar("loss", loss)

    if mode == tf.estimator.ModeKeys.TRAIN:
        update_ops = tf.get_collection(tf.GraphKeys.UPDATE_OPS)
        global_step = tf.train.get_global_step()
        optimizer = tf.train.AdamOptimizer(learning_rate=0.001)
        with tf.control_dependencies(update_ops):
            train_op = optimizer.minimize(loss, global_step=global_step)
        return tf.estimator.EstimatorSpec(mode=mode, loss=loss, train_op=train_op)
    else:
        return tf.estimator.EstimatorSpec(
            mode=mode, loss=loss)


def serving_input_fn():
    inputs = {"x": tf.placeholder(shape=[None, image_size, image_size, 3], dtype=tf.float32)}
    return tf.estimator.export.ServingInputReceiver(inputs, inputs)


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


def batch_generator(data_folder, image_keys, labels):

    while True:
        random_label = np.random.choice(labels)

        index1 = np.random.choice(np.nonzero(labels == random_label)[0])
        index2 = np.random.choice(np.nonzero(labels == random_label)[0])

        img1 = Image.open(os.path.join(data_folder, image_keys[index1]))
        img2 = Image.open(os.path.join(data_folder, image_keys[index2]))

        yield np.array(img1), random_label
        yield np.array(img2), random_label


def input_fn(data_folder, image_keys, labels, batch_size):
    dataset = tf.data.Dataset.from_generator(generator=lambda: batch_generator(data_folder, image_keys, labels),
                                             output_types=(tf.float32, tf.int32),
                                             output_shapes=((image_size, image_size, 3), ()))

    dataset = dataset.batch(batch_size)
    iterator = dataset.make_one_shot_iterator()
    features_tensors, labels_tensors = iterator.get_next()
    features = {'x': features_tensors}
    return features, labels_tensors


def relabel(labels):
    label_map = {}
    count = 0
    new_labels = []
    for label in labels:
        if label in label_map:
            new_labels.append(label_map[label])
        else:
            new_labels.append(count)
            label_map[label] = count
            count += 1
    return np.array(new_labels)


def main(unused_argv):
    image_keys = load_image_keys(DATA_DIR)
    labels = load_anno_from_file(image_keys, LABELS_FILE)
    labels = labels - np.ones(labels.shape)
    eval_partitions = load_anno_from_file(image_keys, EVAL_PARTITIONS_FILE)

    # needed_labels = [randint(0, full_num_classes - 1) for _ in range(num_classes)]
    # indices = [i for i, label in enumerate(labels) if labels[i] in needed_labels]
    #
    # image_keys = image_keys[indices]
    # labels = relabel(labels[indices])
    # eval_partitions = eval_partitions[indices]

    train_indices = [i for i, label in enumerate(eval_partitions) if label in (0, 1)]
    eval_indices = [i for i, label in enumerate(eval_partitions) if label not in (0, 1)]

    print(len(train_indices))
    print(len(labels))

    classifier = tf.estimator.Estimator(
        model_fn=cnn_model_fn, model_dir=TMP_MODEL_DIR)

    early_stopping = tf.contrib.estimator.stop_if_no_decrease_hook(
        classifier,
        metric_name='loss',
        max_steps_without_decrease=1500,
        min_steps=500)

    for j in range(100):
        classifier.train(
            input_fn=lambda: input_fn(DATA_DIR, image_keys[train_indices], labels[train_indices], batch_size),
            steps=10,
            hooks=[early_stopping]
        )
        # eval_results = classifier.evaluate(
        #     input_fn=lambda: input_fn(DATA_DIR, image_keys[eval_indices], labels[eval_indices], batch_size))
        # print(eval_results)
    classifier.export_saved_model(MODEL_DIR, serving_input_fn)
    return 0


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("You must provide data dir, labels file and eval file")
        sys.exit(1)
    DATA_DIR = sys.argv[1]
    LABELS_FILE = sys.argv[2]
    EVAL_PARTITIONS_FILE = sys.argv[3]
    if len(sys.argv) >= 5:
        MODEL_DIR = sys.argv[4]
    if len(sys.argv) >= 6:
        TMP_MODEL_DIR = sys.argv[5]

    print("output model directory: {0}, data directory: {1}".format(MODEL_DIR, DATA_DIR))
    tf.app.run()
