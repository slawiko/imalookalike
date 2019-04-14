import numpy as np
import src.app.net.preprocessing.landmarks as lm
import src.app.net.preprocessing.cropper as cr


_DEFAULT_MEAN_LANDMARKS = np.array([[-0.46911814, -0.51348481],
                                    [0.45750203, -0.53173911],
                                    [-0.00499168, 0.06126145],
                                    [-0.40616926, 0.46826089],
                                    [0.42776873, 0.45444013]])


def get_faces(img):
    return lm.get_faces(img)


def preprocess(img, face, landmarks_model_path, landmarks=None, mean_landmarks=_DEFAULT_MEAN_LANDMARKS):
    if landmarks is None:
        landmarks = lm.compute_landmarks(img, face, landmarks_model_path)

    img_aligned = cr.align_crop(img, landmarks, mean_landmarks)
    return img_aligned

