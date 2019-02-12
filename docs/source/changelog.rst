Changelog
===============================================================================
Changelog for the EuclidesDB releases.

Release v.0.2.0
-------------------------------------------------------------------------------
This is a bug-fix and feature addition release with many good news ! The main new features are: integration with Faiss (see :ref:`search-config` for more information), new models, database compression, new exact linear search and internal codebase refactoring.

Thanks for all the users that opened issues and contributors who helped with this release.

Changes in this release:

	- **[Enhancement]**: using libtorch 1.0.1 now, latest stable release (`#19 <https://github.com/perone/euclidesdb/issues/19>`_);
	- **[Enhancement]**: examples doesn't require ``torchvision`` anymore (`#8 <https://github.com/perone/euclidesdb/pull/8>`_);
	- **[Bug]**: wrong model name in client call can cause the server to quit (`#1 <https://github.com/perone/euclidesdb/issues/1>`_);
	- **[Enhancement]**: major refactoring of indexing types, they're now called **Search Engines** and have their own units and configuration;
	- **[Bug]**: search engines were called with Variables instead of Tensors;
	- **[Enhancement]**: added the new search engine called ``exact_dist`` that will do a on-disk search (as opposed to in-memory search) using linear exact search (see :ref:`search-config` for more information);
	- **[Enhancement]**: each search engine has now their own requirement for refresh the index upon adding new items or not;
	- **[Enhancement]**: added the new search engine called ``faiss`` that integrated Faiss/OpenMP/Blas together with EuclidesDB, any Faiss index type is now supported on EuclidesDB (see :ref:`search-config` for more information);
	- **[Enhancement]**: to avoid memory allocations and improve performance, the reply vectors are now pre-allocated with top-k size;
	- **[Enhancement]**: enabled database compression support (snappy);
	- **[Enhancement]**: added Resnet101 model support;
	- **[Enhancement]**: added internal database versioning machanism to support future underlying changes;
	- **[Bug]**: fixed an issue with Python API (missing ``close()`` channel call);
	- **[Enhancement]**: ``FindSimilar`` RPC call is now called ``FindSimilarImage``;
	- **[Enhancement]**: added a new RPC call called ``FindSimilarImageById`` to search similar items based on items already indexed;
	- **[Enhancement]**: added documentation for each Search Engine and their configurations (see :ref:`search-config` for more information);
	- **[Enhancement]**: added documentation for each low-level gRPC call for advanced users (see :ref:`grpc-api` for more information);


Release v.0.1.1
-------------------------------------------------------------------------------
    - Bug-fix release;
    - Fixed the issue with models prediction softmax (`#2 <https://github.com/perone/euclidesdb/issues/2>`_).

Release v.0.1.0
-------------------------------------------------------------------------------
    - Initial release.
