# mpeg4part2_extract_compressed

This project aims to create a python module for the extraction of the compressed frame representation of MPEG4 part-2 files.

MPEG4 part-2 compression is based on difference encoding to reduce the size of the information transmitted.
At somewhat regular interval a key frame is encoded (full frame encoding), then each subsequent frame is divided into sub-blocks and for each of the blocks, only the difference with the best matching block (motion vector information) in the reference frame is encoded (residual information).
See [this wikipedia page](https://en.wikipedia.org/wiki/Inter_frame) for a visual representation of the process.

This project allows to access this compressed information.

For now, is supported:

- Motion Vector decoding (P-frames)

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

a = extract_mvs("my_file.mp4")
```


# TODO

- [x] Extraction of motion vectors
- [ ] Proper python setup file for local installation
- [ ] Extraction of the motion vectors frame by frame
- [ ] Handle both P and B-frames
- [ ] Upload to PyPI
- [ ] Extraction of the residual frames