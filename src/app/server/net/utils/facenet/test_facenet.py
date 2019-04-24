import tensorflow as tf
import src.app.server.net.utils.facenet.facenet as facenet
import cv2

input_image_size = 160

sess = tf.Session()

facenet.load_model("C:\\Users\\drovdo\\Documents\\Python\\facenet\\20180402-114759.pb")

# Get input and output tensors
images_placeholder = tf.get_default_graph().get_tensor_by_name("input:0")
embeddings = tf.get_default_graph().get_tensor_by_name("embeddings:0")
phase_train_placeholder = tf.get_default_graph().get_tensor_by_name("phase_train:0")
embedding_size = embeddings.get_shape()[1]


def get_embedding(img):
    resized = cv2.resize(img, (input_image_size, input_image_size), interpolation=cv2.INTER_CUBIC)
    prewhitened = facenet.prewhiten(resized)
    reshaped = prewhitened.reshape(-1, input_image_size, input_image_size, 3)
    feed_dict = {images_placeholder: reshaped, phase_train_placeholder: False}
    embedding = sess.run(embeddings, feed_dict=feed_dict)
    return embedding


img = cv2.imread("C:\\Users\\drovdo\\Documents\\Python\\celeba-full\\test\\000001.jpg")
embedding = get_embedding(img)
print(embedding)
