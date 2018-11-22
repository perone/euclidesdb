import argparse

import euclides

from PIL import Image
import numpy as np


def center_crop(image, new_height, new_width):
    height, width = image.size
    left = np.ceil((width - new_width)/2.)
    top = np.ceil((height - new_height)/2.)
    right = np.floor((width + new_width)/2.)
    bottom = np.floor((height + new_height)/2.)
    return image.crop((left, top, right, bottom))


def run_main():
    parser = argparse.ArgumentParser(
        description='Add a new image into database.')
    parser.add_argument('--id', dest='image_id', type=int, required=True,
                        help='ID of the image to add into EuclidesDB.')
    parser.add_argument('--file', dest='filename', type=str, required=True,
                        help='Image file name.')
    args = parser.parse_args()

    image = Image.open(args.filename)
    image_id = int(args.image_id)
    image.thumbnail((300, 300), Image.ANTIALIAS)
    image = center_crop(image, 224, 224)

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
