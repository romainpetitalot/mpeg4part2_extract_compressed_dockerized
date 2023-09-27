# MPEG 4 part-2 compressed representation extraction (Motion vectors)

This project aims to create a python module for the extraction of the compressed frame representation of MPEG4 part-2 files. The whole compression format details can be found [here](http://ecee.colorado.edu/~ecen5653/ecen5653/papers/ISO%2014496-2%202004.PDF).

This project makes use of [swig](http://www.swig.org/) and is based on [this example](https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/extract_mvs.c)

**Disclaimers:** The whole extraction pipeline was not thoroughly tested so you may want to check that you get correct values.

## Compression overview

MPEG4 part-2 compression format is based on difference encoding to reduce the size of the information transmitted.
At somewhat regular interval a key frame (I-frame) is encoded (full frame encoding), then each subsequent frame is divided into sub-blocks and for each of the blocks, only the difference with the best matching block in the reference frame is encoded.
The displacement from the current block to the best block is encoded as "Motion Vectors" and the difference between the two blocks is encoded as "Residual".
See [this wikipedia page](https://en.wikipedia.org/wiki/Inter_frame) for a visual representation of the process.

Usually, I-frames are encoded at regular interval to allow for error correction as well as to allow to catch a stream on the fly (One need at least one I-frame to decode the other frames).
The subsequent frames can be of two types:

- P-frames (Predictive frames)
- B-frames (Bi-directional frames)

P-frames are frame that carry the matching with previous frames while B-frames allow for matching in both of the time directions.
Of course, in case of B-frames, the display of an encoded video cannot be on exact real time.
More details about the video compression picture types can be found [here](h264 encoded video download).

This project aim to allow access in python to the Motion Vectors and the Residual information.

For now, is supported:

- Motion Vector decoding (P-frames)
- Motion Vector decoding (B-frames)

## Requirements

docker (https://docs.docker.com/engine/install/)

## Installation


```bash
# Build the Docker container
docker build -t extract_mvs .

# Run the container (add -v .:/app to have your host files and container files synchronized)
docker run {-v .:/app} -p 8888:8888 -it extract_mvs
```


## Testing the installation

There are two ways to use the provided package, either extracting the full information from the file, or frame by frame.

If you want to visualise the motion vectors extracted, you must use the jupyter notebook file by executing this command line **inside the docker container**.

``` 
jupyter notebook --ip 0.0.0.0 --no-browser --allow-root
```
After copying one of the given link in your web browser, you will be able to acces the scripts/plotVect.ipynb file, containing the motion vector representation.

Whole extraction:

```python
from mpeg42compressed.numpy.extract_mvs import extract_mvs

a, b = extract_mvs("my_file.mp4")
# a will be of shape (ts, w, h, 2) with the last channel being the x and y differencies
# b will be of shape (ts, w, h, 1) and contains the source position of the block (for isntance: -1 = previous frame, 1 = next frame)
```

Extraction frame by frame (file [example.py](scripts/example.py)):

```python
from mpeg42compressed.numpy.mvs_videoreader import MVSVideoReader
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

file_path = ""

video_reader = MVSVideoReader(file_path)


cap = cv2.VideoCapture(file_path)


ret = True
i = 0

while ret:
    ret, frame = video_reader.read()
    _, rgb_frame = cap.read()

    if ret:
        mv_frame = frame[0]
        
        w, h, _ = mv_frame.shape

        temp_frame = np.zeros((w *16, h * 16, 3))

        for w_mv in range(w):
            for h_mv in range(h):
                temp_frame[w_mv*16:(w_mv+1)*16, h_mv*16:(h_mv+1)*16, :2] = mv_frame[w_mv, h_mv]

        # min/max frame convertion, but its only for display
        mv_frame = convert(temp_frame)

        cv2.imshow('Motion Vectors', mv_frame)
        cv2.imshow('RGB', rgb_frame)

        if cv2.waitKey(20) & 0xFF == ord('q'):
            break

video_reader.close()
```



# TODO

- [x] Extraction of motion vectors
- [x] Proper python setup file for local installation
- [x] Handle both P and B-frames
- [x] Extraction of the motion vectors frame by frame
- [ ] Extraction of the residual frames
