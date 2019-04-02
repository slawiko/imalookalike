import dlib
import numpy as np


def _shape_to_np(shape, dtype="int"):
    coords = np.zeros((68, 2), dtype=dtype)

    for i in range(0, 68):
        coords[i] = (shape.part(i).x, shape.part(i).y)

    return coords


def _get_5_landmarks(dlib_landmarks):
    left_eye = np.average(dlib_landmarks[37:42], axis=0)
    right_eye = np.average(dlib_landmarks[43:48], axis=0)
    nose = np.average([dlib_landmarks[30], dlib_landmarks[31], dlib_landmarks[34]], axis=0)
    left_mouth = dlib_landmarks[60]
    right_mouth = dlib_landmarks[54]

    return np.array([left_eye, right_eye, nose, left_mouth, right_mouth], dtype=int)


def get_faces(img):
    detector = dlib.get_frontal_face_detector()
    faces = detector(img, 1)
    return faces


def compute_landmarks(img, face, landmark_model_path):
    predictor = dlib.shape_predictor(landmark_model_path)

    dlib_landmarks = predictor(img, face)
    final_landmarks = _get_5_landmarks(_shape_to_np(dlib_landmarks))

    return final_landmarks

