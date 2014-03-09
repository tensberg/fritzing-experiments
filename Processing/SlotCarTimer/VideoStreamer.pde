import java.awt.image.*;
import java.util.concurrent.TimeUnit;

import com.xuggle.mediatool.IMediaWriter;
import com.xuggle.mediatool.ToolFactory;
import com.xuggle.xuggler.ICodec;
import com.xuggle.xuggler.IRational;

class VideoStreamer {
  
  final IMediaWriter writer;

  final long startTime = System.nanoTime();
  
  VideoStreamer(String filename, int movieWidth, int movieHeight, int movieFrameRate) {
    writer = ToolFactory.makeWriter(filename);
    writer.addVideoStream(0, 0, ICodec.ID.CODEC_ID_H264, IRational.make(movieFrameRate, 1), movieWidth, movieHeight);
  }
  
  void captureDisplayWindow() {
    loadPixels();
    BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
    int[] a = ((DataBufferInt) image.getRaster().getDataBuffer()).getData();
    System.arraycopy(pixels, 0, a, 0, pixels.length);
    add(image);
  }
  
  void add(BufferedImage image) {
    BufferedImage bgrImage = convertToType(image, BufferedImage.TYPE_3BYTE_BGR);
    writer.encodeVideo(0, bgrImage, System.nanoTime() - startTime,TimeUnit.NANOSECONDS);
  }

  void close() {
    writer.close();
  }
  
  private BufferedImage convertToType(BufferedImage sourceImage, int targetType) {
    BufferedImage image;

    // if the source image is already the target type, return the source image
    if (sourceImage.getType() == targetType) {
      image = sourceImage;
    } else {
      // otherwise create a new image of the target type and draw the new image
      image = new BufferedImage(sourceImage.getWidth(), sourceImage.getHeight(), targetType);
      image.getGraphics().drawImage(sourceImage, 0, 0, null);
    }

    return image;
  }
}

