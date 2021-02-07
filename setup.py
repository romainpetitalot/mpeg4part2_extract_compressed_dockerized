from setuptools import setup, Extension, find_packages

# Third-party modules - we depend on numpy for everything
import numpy

# Obtain the numpy include directory.  This logic works across numpy versions.
try:
    numpy_include = numpy.get_include()
except AttributeError:
    numpy_include = numpy.get_numpy_include()

from mpeg42compressed import __version__

# crop extension module
extract_mvs = Extension("mpeg42compressed.numpy._extract_mvs", ["mpeg42compressed/numpy/extract_mvs.i", "mpeg42compressed/common/extract_mvs.c"],
                        include_dirs=[numpy_include],
                        extra_compile_args=["--verbose", "-lavutil", "-lavformat", "-lavcodec"],
                        extra_link_args=["-lavutil", "-lavformat", "-lavcodec"])

setup(name='mpeg42compressed',
      version=__version__,
      description="Library providing a Python function to extract the Motion Vectors from a MPEG4 part 2 video.",
      author='Benjamin Deguerre',
      url='https://github.com/D3lt4lph4/mpeg4part2_extract_compressed',
      packages=find_packages(include=['mpeg42compressed.numpy']),
      ext_modules=[extract_mvs],
      py_modules=["mpeg42compressed.numpy.extract_mvs"],
      zip_safe=False)
