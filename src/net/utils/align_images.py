import cv2
import csv
import sys
import numpy as np
import src.net.preprocessing.preprocess as p

LANDMARKS_MODEL_PATH = 'landmarks\\shape_predictor_68_face_landmarks.dat'


def get_landmarks_metadata(landmark_file):
    landmarks = []

    with open(landmark_file) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter=',')
        for row in csv_reader:
            landmarks.append(row)

    return np.array(landmarks)


def process_image(input_folder, output_folder, img_id, landmarks, mean_landmarks):
    img = cv2.imread(input_folder + "/" + img_id)
    faces = p.get_faces(img)

    if len(faces) != 1:
        return
    img_aligned = p.preprocess(img, faces[0], LANDMARKS_MODEL_PATH, landmarks, mean_landmarks)

    cv2.imwrite(output_folder + "/" + img_id, img_aligned)


def get_mean_landmarks(landmarks):
    left_eye = landmarks[:, 0, :]
    right_eye = landmarks[:, 1, :]
    nose = landmarks[:, 2, :]
    left_mouth = landmarks[:, 3, :]
    right_mouth = landmarks[:, 4, :]

    left = (left_eye + nose + left_mouth) / 3.0
    right = (right_eye + nose + right_mouth) / 3.0
    top = (left_eye + nose + right_eye) / 3.0
    bottom = (left_mouth + nose + right_mouth) / 3.0
    top_mid = (top + left + right) / 3.0
    bottom_mid = (bottom + left + right) / 3.0
    mid = (top_mid + bottom_mid) / 2.0
    v_size = np.linalg.norm((left_eye + right_eye) / 2.0 - (left_mouth + right_mouth) / 2.0, axis=1)

    mid.shape = -1, 1, 2
    v_size.shape = -1, 1, 1
    norm_lm = (landmarks - mid) / v_size
    mean_lm = np.mean(norm_lm, axis=0)
    mean_lm = mean_lm / max(np.max(mean_lm[:, 0]) - np.min(mean_lm[:, 0]),
                            np.max(mean_lm[:, 1]) - np.min(mean_lm[:, 1]))

    return mean_lm


def process_images(input_folder, output_folder, landmark_file):
    landmarks_metadata = get_landmarks_metadata(landmark_file)
    ids = [row[0] for row in landmarks_metadata]
    landmarks = np.reshape(np.array(landmarks_metadata[:, 1:], dtype=int),
                           (len(landmarks_metadata), (len(landmarks_metadata[0]) - 1) // 2, -1))
    mean_landmarks = get_mean_landmarks(landmarks)

    for img_id, landmark in zip(ids, landmarks):
        process_image(input_folder, output_folder, img_id, landmark, mean_landmarks)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("input directory, output directory and landmarks file should be provided!")
        exit(1)
    input_dir = sys.argv[1]
    output_dir = sys.argv[2]
    landmarks_file = sys.argv[3]

    process_images(input_dir, output_dir, landmarks_file)
