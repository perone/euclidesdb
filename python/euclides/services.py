import io
import grpc

from . import euclidesproto_pb2_grpc as ec_grpc
from . import euclidesproto_pb2 as ec_proto


class Channel(object):
    def __init__(self, hostname, port, options=None):
        self.hostname = hostname
        self.port = port
        self.options = options or []

        self.hostport = "{}:{}".format(hostname, port)

        self._channel = grpc.insecure_channel(
            target=self.hostport,
            options=self.options)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self._channel.close()


class EuclidesDB(object):
    def __init__(self, channel):
        self.channel = channel
        self.stub = ec_grpc.SimilarStub(self.channel._channel)

    def add_image(self, image_id, models, image):
        bytes_img = io.BytesIO()
        image.save(bytes_img, format="jpeg")

        request = ec_proto.AddImageRequest()
        request.image_id = int(image_id)
        request.models.extend(models)
        request.image_data = bytes_img.getvalue()
        reply = self.stub.AddImage(request)

        return reply
