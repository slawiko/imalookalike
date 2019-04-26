import tensorflow as tf
import benchmark.net.facenet.facenet as facenet
from benchmark.net.facenet import detect_face
import cv2
import numpy as np


# some constants kept as default from facenet
minsize = 20
threshold = [0.6, 0.7, 0.7]
factor = 0.709
margin = 44
input_image_size = 160

sess = None
images_placeholder = None
embeddings = None
phase_train_placeholder = None
embedding_size = None
pnet = None
rnet = None
onet = None


def init_model(model_path):
    global sess
    global images_placeholder
    global embeddings
    global phase_train_placeholder
    global embedding_size
    global pnet
    global rnet
    global onet

    facenet.load_model(model_path)

    sess = tf.Session()

    images_placeholder = tf.get_default_graph().get_tensor_by_name("input:0")
    embeddings = tf.get_default_graph().get_tensor_by_name("embeddings:0")
    phase_train_placeholder = tf.get_default_graph().get_tensor_by_name("phase_train:0")
    embedding_size = embeddings.get_shape()[1]
    pnet, rnet, onet = detect_face.create_mtcnn(sess, 'facenet\\align')


def get_cropped_image(img):
    img_size = np.asarray(img.shape)[0:2]
    bounding_boxes, points = detect_face.detect_face(img, minsize, pnet, rnet, onet, threshold, factor)
    if not len(bounding_boxes) == 0:
        for face in bounding_boxes:
            if face[4] > 0.50:
                det = np.squeeze(face[0:4])
                bb = np.zeros(4, dtype=np.int32)
                bb[0] = np.maximum(det[0] - margin / 2, 0)
                bb[1] = np.maximum(det[1] - margin / 2, 0)
                bb[2] = np.minimum(det[2] + margin / 2, img_size[1])
                bb[3] = np.minimum(det[3] + margin / 2, img_size[0])
                cropped = img[bb[1]:bb[3], bb[0]:bb[2], :]
                return cropped


def predict_embeddings(imgs):
    prewhiteneds = []
    for img in imgs:
        cropped = get_cropped_image(img)
        if cropped is None:
            print("not detected")
            prewhiteneds.append(img)
        else:
            resized = cv2.resize(cropped, (input_image_size, input_image_size), interpolation=cv2.INTER_CUBIC)
            prewhitened = facenet.prewhiten(resized)
            prewhiteneds.append(prewhitened)
    reshaped = np.array(prewhiteneds).reshape((-1, input_image_size, input_image_size, 3))
    feed_dict = {images_placeholder: reshaped, phase_train_placeholder: False}
    embedding = sess.run(embeddings, feed_dict=feed_dict)
    return embedding


