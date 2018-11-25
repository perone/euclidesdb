Client APIs
===============================================================================
This section will show how to use the multiple client APIs that can communicate with EuclidesDB.

Python Client API
-------------------------------------------------------------------------------
Before using the Python client API, you just have to install it using ``pip``::

    pip install euclides

After that, if you want o add a new item into the database, just follow the example below:

.. code-block:: python

    import euclides

    with euclides.Channel("localhost", 50000) as channel:
        db = euclides.EuclidesDB(channel)
        ret_add = db.add_image(image_id, models, image)

All images are assumed to be PIL images, the same type handled by ``torchvision``. You can see a complete example below, for more examples, see the `Python package examples <https://github.com/perone/euclidesdb/tree/master/python/examples>`_ folder.

.. code-block:: python

    import sys
    import argparse

    import euclides

    from PIL import Image
    import numpy as np

    from torchvision.transforms import functional as F


    def run_main():
        parser = argparse.ArgumentParser(description='Add a new image into database.')
        parser.add_argument('--id', dest='image_id', type=int, required=True,
                            help='ID of the image to add into EuclidesDB.')
        parser.add_argument('--file', dest='filename', type=str, required=True,
                            help='Image file name.')
        args = parser.parse_args()

        image = Image.open(args.filename)
        image_id = int(args.image_id)
        image.thumbnail((300, 300), Image.ANTIALIAS)
        image = F.center_crop(image, 224)

        with euclides.Channel("localhost", 50000) as channel:
            db = euclides.EuclidesDB(channel)
            ret_add = db.add_image(image_id, ["resnet18"], image)

            # After finishing adding items, you need to tell
            # the database to refresh the indexes to add newly
            # indexed items.
            db.refresh_index()

        predictions = ret_add.vectors[0].predictions
        print("Preds Len: ", len(predictions))

        # Category should be 281: 'tabby, tabby cat' for cat.jpg
        # Classes from https://gist.github.com/yrevar/942d3a0ac09ec9e5eb3a
        print("Category : ", np.array(predictions).argmax())


    if __name__ == "__main__":
        run_main()


.. seealso:: See the `Python package examples <https://github.com/perone/euclidesdb/tree/master/python/examples>`_ folder for more information.