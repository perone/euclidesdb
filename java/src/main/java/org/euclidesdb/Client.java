package org.euclidesdb;

import euclidesdb.proto.*;
import euclidesdb.proto.SimilarGrpc;
import euclidesdb.proto.SimilarGrpc.SimilarBlockingStub;

import com.google.protobuf.ByteString;
import io.grpc.*;

import java.util.*;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;

import javax.imageio.ImageIO;
import java.awt.image.BufferedImage;
import org.imgscalr.Scalr;

public class Client {
  // Configuration
  static String hostname = "localhost";
  static Integer port = 50000;
  static Integer maxResults = 10;
  static List<String> models = Arrays.asList("resnet18"); // FIXME Cannot find the module: vgg16
  static List<String> samples = Arrays.asList(
    "cat.jpg", "cat2.jpg", "cat3.jpg",
    "elephant.jpg", "elephant2.jpg", "elephant3.jpg",
    "keyboard.jgp", "keyboard2.jpg");

  public static void main(String[] args) {
    // Create gRPC channel
    ManagedChannel channel = ManagedChannelBuilder.forAddress(hostname, port).usePlaintext().build();
    SimilarBlockingStub stub = SimilarGrpc.newBlockingStub(channel);

    System.out.println("Running client-side queries against EuclidesDB (" + hostname + ":" + port + ")");

    // TODO Make some calls to EuclidesDB
    samples.forEach((String filename) -> {
      AddImageReply reply = addImage(stub, filename, 42);
      System.out.println("Num model space added: " + reply.getVectorsCount());
    });

    removeImage(stub, 42);
    findSimilarImage(stub, samples.get(3));
    findSimilarImageById(stub, 42);

    shutdown(stub, 1);
    channel.shutdown();
  }

  private static byte[] toJPG(String imagePath) {
    byte[] result = new byte[0];

    try {
      InputStream stream = Client.class.getClassLoader().getResourceAsStream(imagePath);
      BufferedImage buffer = Scalr.resize(ImageIO.read(stream),
        Scalr.Method.SPEED, Scalr.Mode.FIT_TO_WIDTH, 300, 300, Scalr.OP_ANTIALIAS);
      buffer = Scalr.crop(buffer, 224, 224, Scalr.OP_ANTIALIAS);

      ByteArrayOutputStream baos = new ByteArrayOutputStream();
      ImageIO.write(buffer, "jpg", baos);
      baos.flush();
      result = baos.toByteArray();
      baos.close();
    } catch (Exception e) {
      System.out.println(e.toString());
      // FIXME: java.lang.IllegalArgumentException: Invalid crop bounds:
      // y + height [224] must be <= src.getHeight() [218]
    }

    return result;
  }

  public static AddImageReply addImage(SimilarBlockingStub stub, String imagePath, Integer imageId) {
    byte[] data = toJPG(imagePath);
    AddImageRequest.Builder builder = AddImageRequest.newBuilder()
      .setImageId(imageId)
      .addAllModels(models)
      .setImageData(ByteString.copyFrom(data));

    AddImageRequest request = builder.build();
    return stub.addImage(request);
  }

  public static RemoveImageReply removeImage(SimilarBlockingStub stub, Integer imageId) {
    RemoveImageRequest.Builder builder = RemoveImageRequest.newBuilder()
      .setImageId(imageId);

    RemoveImageRequest request = builder.build();
    return stub.removeImage(request);
  }

  public static FindSimilarImageReply findSimilarImage(SimilarBlockingStub stub, String imagePath) {
    byte[] data = toJPG(imagePath);
    FindSimilarImageRequest.Builder builder = FindSimilarImageRequest.newBuilder()
      .setTopK(maxResults)
      .addAllModels(models)
      .setImageData(ByteString.copyFrom(data));

    FindSimilarImageRequest request = builder.build();
    return stub.findSimilarImage(request);
  }

  public static FindSimilarImageReply findSimilarImageById(SimilarBlockingStub stub, Integer imageId) {
    FindSimilarImageByIdRequest.Builder builder = FindSimilarImageByIdRequest.newBuilder()
      .setTopK(maxResults)
      .addAllModels(models)
      .setImageId(imageId);

    FindSimilarImageByIdRequest request = builder.build();
    return stub.findSimilarImageById(request);
  }

  public static ShutdownReply shutdown(SimilarBlockingStub stub, Integer type) {
    ShutdownRequest.Builder builder = ShutdownRequest.newBuilder()
      .setShutdownType(type);

    ShutdownRequest request = builder.build();
    return stub.shutdown(request);
  }
}
