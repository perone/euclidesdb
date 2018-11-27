import argparse
import json
from pathlib import Path

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


def add_image(db, id_item, filepath):
    image = Image.open(filepath)
    image.thumbnail((300, 300), Image.ANTIALIAS)
    image = center_crop(image, 224, 224)
    return db.add_image(id_item, ["resnet18"], image)


def run_main():
    parser = argparse.ArgumentParser(
        description='Add all images from a directory into the database.')
    parser.add_argument('--directory', dest='directory', type=str, required=True,
                        help='Image file name (ex. ./images).')
    parser.add_argument('--pattern', dest='pattern', type=str, required=True,
                        help='Image file pattern (ex. *.jpg).')
    parser.add_argument('--output', dest='output', type=str, required=True,
                        help='Output filename with IDs (ex. output.json).')
    args = parser.parse_args()

    channel = euclides.Channel("localhost", 50000)
    db = euclides.EuclidesDB(channel)

    path = Path(args.directory)
    items_dict = {}
    for id_item, pfile in enumerate(path.glob(args.pattern)):
        print("Adding file", pfile.name)
        add_image(db, id_item, pfile.absolute())
        items_dict[pfile.name] = id_item

    with open(args.output, "w") as fhandle:
        json.dump(items_dict, fhandle)

    db.refresh_index()
    channel.close()


if __name__ == "__main__":
    run_main()
