EuclidesDB Installation
===============================================================================
EuclidesDB can be installed both on Linux and MacOS systems. We also provide a Docker image with a single ResNet-18 model already embedded.

See below how to install EuclidesDB in different systems.

Using Docker on any system
-------------------------------------------------------------------------------
The easiest way to execute EuclidesDB on any system is to use `Docker <https://www.docker.com>`_. There is an image already pre-made with ResNet-18 model already embedded, to execute the server, you just need to execute the following line below::

    docker run -p 50000:50000 \
        -v ~/database:/database \
        -it euclidesdb/euclidesdb

This command will host EuclidesDB on the local port 50000 (for RPC calls) and it will store the database data into the host (local) folder ``~/database``.

.. note:: If the database doesn't exists, it will be created by EuclidesDB on the first run.

Installing on Linux
-------------------------------------------------------------------------------
To install EuclidesDB on Linux systems, you just have to download the `last release <https://github.com/perone/euclidesdb/releases>`_ and then de-compress it and follow the instructions to configure and setup the models:

.. code-block:: shell

    ~$ tar zxvf euclidesdb-<version>-Linux.tar.gz
    ~$ cd euclidesdb
    ~/euclidesdb$ ./euclidesdb -c euclidesdb.conf

EuclidesDB has static linking and ships with all of its external dependencies, so it should work fine on many modern linux distributions without requiring external packages. See how to configure EuclidesDB on the :ref:`section-configuring` section.

Installing on MacOS
-------------------------------------------------------------------------------
To install EuclidesDB in MacOS, the best approach is to install dependencies using `homebrew <https://brew.sh/>`_ as shown below::

    brew install grpc
    brew install leveldb
    brew install libomp

After this, please go to the `Release Download <https://github.com/perone/euclidesdb/releases>`_ page in Github and download the latest stable MacOS build, extract the file and you're ready to go. See how to configure EuclidesDB on the :ref:`section-configuring` section.
