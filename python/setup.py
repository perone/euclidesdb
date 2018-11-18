from setuptools import setup, find_packages
from codecs import open
from os import path

import euclides as ec

here = path.abspath(path.dirname(__file__))

with open('requirements.txt') as f:
    requirements = f.read().splitlines()

setup(
    name='euclides',
    version=ec.__version__,
    description='EuclidesDB Python client API.',
    url='https://github.com/perone/euclidesdb',
    author='Christian S. Perone',
    author_email='christian.perone@gmail.com',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Programming Language :: Python :: 3',
    ],
    packages=find_packages(exclude=['contrib', 'docs', 'tests']),
    install_requires=requirements,
    # entry_points={
        # 'console_scripts': [
        #    'cmdname=medicaltorch.mod:function',
        # ],
    # },
)
