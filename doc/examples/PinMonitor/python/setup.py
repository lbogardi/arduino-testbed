import os
from setuptools import setup

def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

setup(
    name = 'PinMonitor',
    version = '0.0.1',
    description = 'Process binary data sent by an Arduino',
    long_description=read('README.md'),
    author = 'Matyas Liptak',
    author_email = 'liptak.matyas@gmail.com',
    license = 'BSD',
    keywords = 'arduino usb serial example',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Topic :: Example',
        'License :: OSI Approved :: BSD License',
    ],

    include_package_data=True,

    install_requires=['pyserial'],
    setup_requires=['pytest-runner'],
    tests_require=['pytest'],

    packages=['binstruct'],
)

