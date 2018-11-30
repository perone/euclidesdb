Low-level gRPC API
===============================================================================
This section describes the low-level gRPC API that you can use from any other language or any other server/service proxy.

EuclidesDB implements the following gRPC calls:

.. code-block:: protobuf

    service Similar {
        rpc Shutdown (ShutdownRequest) returns (ShutdownReply) {}
        rpc FindSimilarImage (FindSimilarImageRequest) returns (FindSimilarImageReply) {}
        rpc FindSimilarImageById (FindSimilarImageByIdRequest) returns (FindSimilarImageReply) {}
        rpc AddImage (AddImageRequest) returns (AddImageReply) {}
        rpc RemoveImage (RemoveImageRequest) returns (RemoveImageReply) {}
    }

Each one of these RPC calls are described in the next sections. Errors are returned as gRPC errors with a ``CANCELED`` status.

.. seealso:: See the `gRPC documentation <https://grpc.io/>`_ for more information. If you're not familiar with ``protobuf`` syntax, please take a look on `these tutorials <https://developers.google.com/protocol-buffers/docs/tutorials>`_.

``AddImage`` -- add a new image item into the database
-------------------------------------------------------------------------------
The prototype of the ``AddImage`` call is the following::

    rpc AddImage (AddImageRequest) returns (AddImageReply) {}

This RPC call will accept a ``AddImageRequest`` request object as input and it will return a ``AddImageReply`` as result. The definition of these objects are described below:

.. code-block:: protobuf

    message AddImageRequest {
        int32 image_id = 1;
        bytes image_data = 2;
        bytes image_metadata = 3;
        repeated string models = 4;
    }

    message AddImageReply {
        repeated ItemVectors vectors = 1;
    }

These definitions are quite simple and the field names describe the meaning of each field. The ``ItemVectors`` is described below:

.. code-block:: protobuf

    message ItemVectors {
        string model = 1;
        repeated float predictions = 2;
        repeated float features = 3;
    }

Which is the predictions and features for each model space.

``RemoveImage`` -- removes an image item from the database
-------------------------------------------------------------------------------
The prototype of the ``RemoveImage`` call is the following::

    rpc RemoveImage (RemoveImageRequest) returns (RemoveImageReply) {}

This RPC call will accept a ``RemoveImageRequest`` request object as input and it will return a ``RemoveImageReply`` as result. The definition of these objects are described below:

.. code-block:: protobuf

    message RemoveImageRequest {
        int32 image_id = 1;
    }

    message RemoveImageReply {
        int32 image_id = 1;
    }

This call will accept a ``image_id`` as input and it will answer with the same field.

``FindSimilarImageById`` -- find similar items to an item existing in the database
-----------------------------------------------------------------------------------
The prototype of the ``FindSimilarImageById`` call is the following::

    rpc FindSimilarImageById (FindSimilarImageByIdRequest) returns (FindSimilarImageReply) {}

This RPC call will accept a ``FindSimilarImageByIdRequest`` request object as input and it will return a ``FindSimilarImageReply`` as result. The definition of these objects are described below:

.. code-block:: protobuf

    message FindSimilarImageByIdRequest {
        int32 top_k = 1;
        int32 image_id = 2;
        repeated string models = 3;
    }

    message FindSimilarImageReply {
        repeated SearchResults results = 1;
    }

This RPC call will accept a ``top_k`` that is the number of similar items you want EuclidesDB to return, the item id and the model spaces you want to search. The definition of the ``SearchResults`` is described below:

.. code-block:: protobuf

    message SearchResults {
        repeated int32 top_k_ids = 1;
        repeated float distances = 2;
        string model = 3;
    }

Which is basically the ids of the closest items, their distances and the model where these ids were found.

``FindSimilarImage`` -- find similar items to a new item
-----------------------------------------------------------------------------------
The prototype of the ``FindSimilarImage`` call is the following::

    rpc FindSimilarImage (FindSimilarImageRequest) returns (FindSimilarImageReply) {}

This RPC call will accept a ``FindSimilarImageRequest`` request object as input and it will return a ``FindSimilarImageReply`` as result. The definition of these objects are described below:

.. code-block:: protobuf

    message FindSimilarImageRequest {
        int32 top_k = 1;
        bytes image_data = 2;
        repeated string models = 3;
    }

    message FindSimilarImageReply {
        repeated SearchResults results = 1;
    }

This RPC call will accept a ``top_k`` that is the number of similar items you want EuclidesDB to return, the image data and the model spaces you want to search. The definition of the ``SearchResults`` is the same described in the ``FindSimilarImageById`` call.

``Shutdown`` -- request a shutdown command (shutdown/refresh indexes)
-----------------------------------------------------------------------------------
The prototype of the ``Shutdown`` call is the following::

    rpc Shutdown (ShutdownRequest) returns (ShutdownReply) {}

This RPC call will accept a ``ShutdownRequest`` request object as input and it will return a ``ShutdownReply`` as result. The definition of these objects are described below:

.. code-block:: protobuf

    message ShutdownRequest {
        int32 shutdown_type = 1;
    }

    message ShutdownReply {
        bool shutdown = 1;
    }

The ``shutdown_type`` can be one of the following:

- ``0`` - a regular database shutdown, it will shutdown EuclidesDB immediately after waiting for all the calls to complete gracefully;
- ``1`` - a request for EuclidesDB to refresh its indexes. This must be called after adding items into the database (at the end after adding all items). The semantics of this action is that EuclidesDB will gracefully wait for all requests to finish, it will then do a momentary stop while refreshing its memory indexes (this depend on the amount of data in the database and search engine selected) and then it will start to accept requests again. Any call during the refreshing process will not be processed.

This call will return ``true`` if the request was accepted or ``false`` otherwise. Currently, there is no ``false`` return from this call, because the call is always accepted.



