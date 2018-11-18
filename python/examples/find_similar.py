import sys
import argparse

import euclides

from PIL import Image
import numpy as np

from torchvision.transforms import functional as F


def run_main():
    parser = argparse.ArgumentParser(description='Find similar images in EuclidesDB.')
    parser.add_argument('--topk', dest='topk', type=int, required=True,
                        help='Find top k results.')
    parser.add_argument('--file', dest='filename', type=str, required=True,
                        help='Image file name.')
    args = parser.parse_args()

    image = Image.open(args.filename)
    image.thumbnail((300, 300), Image.ANTIALIAS)
    image = F.center_crop(image, 224)

    with euclides.Channel("localhost", 50000) as channel:
        db = euclides.EuclidesDB(channel)
        ret_similar = db.find_similar(image, ["resnet18"], args.topk)

    print(ret_similar)


if __name__ == "__main__":
    run_main()
