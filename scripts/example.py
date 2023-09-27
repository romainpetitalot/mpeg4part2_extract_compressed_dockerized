from mpeg42compressed.numpy.mvs_videoreader import MVSVideoReader
from mpeg42compressed.numpy.extract_mvs import extract_mvs

import cv2
import numpy as np

def convert(img: object,
            img_min: int = None,
            img_max: int = None,
            target_type_min: int = 0,
            target_type_max: int = 255,
            target_type: object = None):
    """ Converts an image to a given range of values.
    
        # Arguments:
            - img: The image to convert, a numpy array.
            - img_min: The min possible value of the image, if None, the min of the image is used.
            - img_max: The max possible value of the image, if None, the max of the image is used.
            - target_type_min: Target min value once converted.
            - target_max_type: Target max value once converted.
            - target_type: The numpy type to convert to.
        
        # Returns
            The converted image. The converted image is not a pointer to the original image.
    """
    if img_min is None:
        img_min = img.min()

    if img_max is None:
        img_max = img.max()

    if target_type is None:
        target_type = np.uint8

   
    if (img_max - img_min) == 0:
        a = 0
        b = 0
    else:
        # Scaling factor
        a = (target_type_max - target_type_min) / (img_max - img_min)
        # Centering factor
        b = target_type_max - a * img_max

    # Put the image in the desired range and convert to the required type
    new_img = (a * img + b).astype(target_type)

    return new_img

file_path = "./scripts/singleball.mp4"

a, b = extract_mvs(file_path)

print(np.shape(a))
print(np.shape(b))

video_reader = MVSVideoReader(file_path)


cap = cv2.VideoCapture(file_path)


ret = True
i = 0

while ret:
    ret, frame = video_reader.read()
    _, rgb_frame = cap.read()

    if ret:
        i += 1
        mv_frame = frame[0]
        
        w, h, _ = mv_frame.shape

        temp_frame = np.zeros((w *16, h * 16, 3))

        for w_mv in range(w):
            for h_mv in range(h):
                temp_frame[w_mv*16:(w_mv+1)*16, h_mv*16:(h_mv+1)*16, :2] = mv_frame[w_mv, h_mv]

        # min/max frame convertion, but its only for display
        mv_frame = convert(temp_frame)

        print("vct", np.shape(mv_frame))
        print("rgb",np.shape(rgb_frame))
        #cv2.imshow('Motion Vectors', mv_frame)
        #cv2.imshow('RGB', rgb_frame)

        if cv2.waitKey(20) & 0xFF == ord('q'):
            break

video_reader.close()

print(i)
