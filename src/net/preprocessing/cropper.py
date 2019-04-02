import cv2
import numpy as np


def align_crop(img,
               src_landmarks,
               mean_landmarks,
               crop_size=256,
               face_factor=0.7,
               landmark_factor=0.35):

    # move
    move = np.array([img.shape[1] // 2, img.shape[0] // 2])

    # pad border
    v_border = img.shape[0] - crop_size
    w_border = img.shape[1] - crop_size
    if v_border < 0:
        v_half = (-v_border + 1) // 2
        img = np.pad(img, ((v_half, v_half), (0, 0), (0, 0)), mode='edge')
        src_landmarks += np.array([0, v_half])
        move += np.array([0, v_half])
    if w_border < 0:
        w_half = (-w_border + 1) // 2
        img = np.pad(img, ((0, 0), (w_half, w_half), (0, 0)), mode='edge')
        src_landmarks += np.array([w_half, 0])
        move += np.array([w_half, 0])

    # estimate transform matrix
    mean_landmarks -= np.array([mean_landmarks[0, :] + mean_landmarks[1, :]]) / 2.0  # middle point of eyes as center
    trg_landmarks = mean_landmarks * (crop_size * face_factor * landmark_factor) + move
    tform = cv2.estimateAffinePartial2D(trg_landmarks, src_landmarks, ransacReprojThreshold=np.Inf)[0]

    # fix the translation to match the middle point of eyes
    trg_mid = (trg_landmarks[0, :] + trg_landmarks[1, :]) / 2.0
    src_mid = (src_landmarks[0, :] + src_landmarks[1, :]) / 2.0
    new_trg_mid = cv2.transform(np.array([[trg_mid]]), tform)[0, 0]
    tform[:, 2] += src_mid - new_trg_mid

    # warp image by given transform
    output_shape = (crop_size // 2 + move[1] + 1, crop_size // 2 + move[0] + 1)
    img_align = cv2.warpAffine(img, tform, output_shape[::-1], flags=cv2.WARP_INVERSE_MAP + cv2.INTER_CUBIC,
                               borderMode=cv2.BORDER_REPLICATE)

    # crop
    img_crop = img_align[-crop_size:, -crop_size:]

    return img_crop
