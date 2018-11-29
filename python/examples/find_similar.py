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
    parser = argparse.ArgumentParser(description='Find similar images in EuclidesDB.')
    parser.add_argument('--topk', dest='topk', type=int, required=True,
                        help='Find top k results.')
    parser.add_argument('--file', dest='filename', type=str, required=True,
                        help='Image file name.')
    args = parser.parse_args()

    image = Image.open(args.filename)
    image.thumbnail((300, 300), Image.ANTIALIAS)
    image = center_crop(image, 224, 224)

    with euclides.Channel("localhost", 50000) as channel:
        db = euclides.EuclidesDB(channel)
        ret_similar = db.find_similar_image(image, ["resnet18"], args.topk)

    print(ret_similar)


if __name__ == "__main__":
    run_main()
