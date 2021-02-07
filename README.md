# MPEG 4 part-2 compressed representation extraction

This project aims to create a python module for the extraction of the compressed frame representation of MPEG4 part-2 files.

**Disclaimers:** The whole extraction is not properly tested, you should at least visually check that you are getting the values you are expecting. 

MPEG4 part-2 compression is based on difference encoding to reduce the size of the information transmitted.
At somewhat regular interval a key frame is encoded (full frame encoding), then each subsequent frame is divided into sub-blocks and for each of the blocks, only the difference with the best matching block (motion vector information) in the reference frame is encoded (residual information).
See [this wikipedia page](https://en.wikipedia.org/wiki/Inter_frame) for a visual representation of the process.

This project makes use of [swig](http://www.swig.org/) and is based on [this example](https://github.com/FFmpeg/FFmpeg/blob/master/doc/examples/extract_mvs.c)

For now, is supported:

- Motion Vector decoding (P-frames)
- Motion Vector decoding (B-frames)



## Requirements

- libav
- numpy
- swig

## Installation

Testing installation:

```bash
# Create virtual env
mkdir .venv

cd .venv

python3 -m venv mpeg4

source mpeg4/bin/activate

cd ..

# Install
bash compile.sh

# To test quickly, go to mpeg42compressed/numpy and run python command prompt
```

Package creation:

```bash
# Create virtual env
mkdir .venv

cd .venv

python3 -m venv mpeg4

source mpeg4/bin/activate

cd ..

# Install
To come ...
```

Testing the installation:

```python
import extract_mvs

a, b = extract_mvs("my_file.mp4")
# a will be of shape (ts, w, h, 2) with the last channel being the x and y differencies
# b will be of shape (ts, w, h, 1) and contains the source position of the block (i.e -1 = previous frame, 1 = next frame)
```


# TODO

- [x] Extraction of motion vectors
- [x] Proper python setup file for local installation
- [x] Handle both P and B-frames
- [x] Extraction of the motion vectors frame by frame
- [ ] Extraction of the residual frames