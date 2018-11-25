.. _section-contributing:

Contributing to EuclidesDB
===============================================================================
Bug reports and code and documentation patches are welcome. You can help this project also by using the development version of EuclidesDB and by reporting any bugs you might encounter.

Reporting bugs
-------------------------------------------------------------------------------
To report any bug, please open a `new issue <https://github.com/perone/euclidesdb/issues/new>`_ on our Github repository.

Contributing code and documentation
-------------------------------------------------------------------------------
You can also contribute by coding, testing or adding documentation, but before doing it, please consider `opening an issue <https://github.com/perone/euclidesdb/issues/new>`_ in GitHub to discuss it before implementing to avoid rejected pull-requests.

Setting a development environment
-------------------------------------------------------------------------------
To set a development environment, you can just clone the repository and use ``cmake`` to generate makefiles::

    git clone https://github.com/perone/euclidesdb.git
    mkdir build
    cd build
    cmake ..
    make -j2

For preparing a release version with optimizations enabled::

    git clone https://github.com/perone/euclidesdb.git
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j2

To create release package::

    git clone https://github.com/perone/euclidesdb.git
    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j2
    make package

There is also some `Docker files in the repository <https://github.com/perone/euclidesdb/tree/master/docker>`_ where we show how to build the binary package from scratch using a self-contained Docker container.
