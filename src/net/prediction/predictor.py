import src.net.preprocessing.preprocess as preprocess
from tensorflow.contrib import predictor
import cv2

_MODEL_DIR = 'model'
_LANDMARKS_MODEL_PATH = 'landmarks\\shape_predictor_68_face_landmarks.dat'


predict_fn = None
landmarks_model = ''


def init_model(model_path=_MODEL_DIR, landmarks_model_path=_LANDMARKS_MODEL_PATH):
    global predict_fn
    global landmarks_model
    predict_fn = predictor.from_saved_model(model_path)
    landmarks_model = landmarks_model_path


def check_model_inited():
    return predict_fn and landmarks_model


def _predict_embedding(img):
    predictions = predict_fn({"x": [img]})

    return predictions['embedding']


def get_embedding(img_path):
    if not check_model_inited():
        raise RuntimeError("Model is not inited")

    img = cv2.imread(img_path)
    faces = preprocess.get_faces(img)
    if len(faces) != 1:
        raise RuntimeError
    face = faces[0]
    aligned_image = preprocess.preprocess(img, face, landmarks_model)

    embedding = _predict_embedding(aligned_image)

    return embedding


init_model('C:\\Users\\drovdo\\Documents\\Python\\imalookalike\\src\\net\\prediction\\model',
           'C:\\Users\\drovdo\\Documents\\Python\\imalookalike\\src\\net\\preprocessing\\landmarks\\shape_predictor_68_face_landmarks.dat')

print(get_embedding('C:\\Users\\drovdo\\Documents\\Python\\celeba-full\\img_celeba\\000002.jpg'))
