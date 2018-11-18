import io
import sys
from PIL import Image
import grpc

import euclidesproto_pb2_grpc as ir_grpc
import euclidesproto_pb2 as ir

from torchvision.transforms import functional as F

def run_main():
    im = Image.open(sys.argv[1])
    im.thumbnail((300, 300), Image.ANTIALIAS)
    img = F.center_crop(im, 224)

    temp = io.BytesIO()
    img.save(temp, format="jpeg")
    #img.save("out.jpg", format="jpeg")

    with grpc.insecure_channel('localhost:50000') as channel:
        stub = ir_grpc.SimilarStub(channel)
        req = ir.FindSimilarRequest()
        req.top_k = 10
        req.models.append("resnet18")
        req.models.append("vgg16")
        req.image_data = temp.getvalue()
        reply = stub.FindSimilar(req)
        print(reply)


if __name__ == "__main__":
    run_main()