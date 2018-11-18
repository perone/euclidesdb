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
    SHUTDOWN_REGULAR = 0
    SHUTDOWN_REFRESH = 1

    def __init__(self, channel, wire_image="jpeg"):
        self.channel = channel
        self.stub = ec_grpc.SimilarStub(self.channel._channel)
        self.wire_image = wire_image

    def add_image(self, image_id, models, image):
        bytes_img = io.BytesIO()
        image.save(bytes_img, format=self.wire_image)
        request = ec_proto.AddImageRequest()
        request.image_id = int(image_id)
        request.models.extend(models)
        request.image_data = bytes_img.getvalue()
        reply = self.stub.AddImage(request)
        return reply

    def remove_image(self, image_id):
        request = ec_proto.RemoveImageRequest()
        request.image_id = int(image_id)
        reply = self.stub.RemoveImage(request)
        return reply

    def find_similar(self, image, models, top_k=5):
        bytes_img = io.BytesIO()
        image.save(bytes_img, format=self.wire_image)
        request = ec_proto.FindSimilarRequest()
        request.models.extend(models)
        request.top_k = int(top_k)
        request.image_data = bytes_img.getvalue()
        reply = self.stub.FindSimilar(request)
        return reply

    def __shutdown(self, shutdown_type):
        request = ec_proto.ShutdownRequest()
        request.shutdown_type = shutdown_type
        reply = self.stub.Shutdown(request)
        return reply

    def refresh_index(self):
        self.__shutdown(EuclidesDB.SHUTDOWN_REFRESH)

    def shutdown(self):
        self.__shutdown(EuclidesDB.SHUTDOWN_REGULAR)

