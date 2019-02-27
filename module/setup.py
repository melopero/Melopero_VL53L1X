#!/usr/bin/python

import setuptools
import os

sources_files = []
lib_dirs = ['.','api/core', 'api/platform', 'api/python-interface']
for libdir in lib_dirs:
    for file in os.listdir(os.path.join(os.getcwd(), libdir)):
        if file.endswith('.c'):
            sources_files.append(os.path.join(libdir,file))


extension = setuptools.Extension(
    'vl53l1x_api',
    define_macros=[],
    include_dirs=lib_dirs,
    libraries=[],
    library_dirs=[],
    sources= sources_files)

setuptools.setup(
    name="melopero_vl53l1x",
    version="0.1.0",
    description="A module to easily access Melopero's VL53L1X sensor's features",
    url="https://github.com/melopero/Melopero_VL53L1X/tree/master/module",
    author="Melopero",
    author_email="info@melopero.com",
    license="MIT",
    packages=setuptools.find_packages(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.5",
    ],
    install_requires=["smbus2"],
)
