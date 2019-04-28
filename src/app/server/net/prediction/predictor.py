from tensorflow.contrib import predictor
import cv2
import numpy as np

import net.preprocessing.preprocess as preprocess

# TODO: must be taken from os.environ probably
_MODEL_DIR = './resources/model'
_LANDMARKS_MODEL_PATH = './resources/landmarks/shape_predictor_68_face_landmarks.dat'


predict_fn = None
landmarks_model = ''


def init_model(model_path=_MODEL_DIR, landmarks_model_path=_LANDMARKS_MODEL_PATH):
    global predict_fn
    global landmarks_model
    predict_fn = predictor.from_saved_model(model_path)
    landmarks_model = landmarks_model_path


def check_model_inited():
    return predict_fn and landmarks_model


def predict_embeddings(imgs):
    predictions = predict_fn({"x": imgs})

    return predictions['embedding']


def get_embedding(image):
    if not check_model_inited():
        raise RuntimeError("Model is not inited")

    if type(image) is str:
        img = cv2.imread(image)
    elif type(image) is bytes:
        img = cv2.imdecode(np.fromstring(image, np.uint8), flags=cv2.IMREAD_COLOR)
    else:
        raise TypeError('Argument must have type str or bytes')

    faces = preprocess.get_faces(img)
    if len(faces) == 0:
        raise RuntimeError("No faces detected")
    elif len(faces) > 1:
        raise RuntimeError("Multiple faces detected")
    face = faces[0]
    aligned_image = preprocess.preprocess(img, face, landmarks_model)

    embedding = predict_embeddings([aligned_image])

    return embedding


