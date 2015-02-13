import io.humble.video.Codec;
import io.humble.video.Encoder;
import io.humble.video.MediaPacket;
import io.humble.video.MediaPicture;
import io.humble.video.Muxer;
import io.humble.video.MuxerFormat;
import io.humble.video.PixelFormat;
import io.humble.video.Rational;
import io.humble.video.awt.MediaPictureConverter;
import io.humble.video.awt.MediaPictureConverterFactory;

import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

// use Humble Video to encode the images to mp4
// see https://github.com/artclarke/humble-video
// you must have the humble video jars in your processing classpath

class VideoStreamer {

    private final long startTime = System.nanoTime();
    private final Muxer muxer;
    private final Encoder encoder;
    private final MediaPicture picture;
    private final MediaPacket packet = MediaPacket.make();
    private MediaPictureConverter converter;
    private int frame;

    VideoStreamer(String filename, int movieWidth, int movieHeight, int movieFrameRate) {
        muxer = Muxer.make(filename, null, null);

        final MuxerFormat format = muxer.getFormat();
        final Codec codec = Codec.findEncodingCodec(format.getDefaultVideoCodecId());
        encoder = Encoder.make(codec);
        encoder.setWidth(movieWidth);
        encoder.setHeight(movieHeight);
        final PixelFormat.Type pixelformat = PixelFormat.Type.PIX_FMT_YUV420P;
        encoder.setPixelFormat(pixelformat);
        Rational framerate = Rational.make(1, movieFrameRate);
        encoder.setTimeBase(framerate);
        if (format.getFlag(MuxerFormat.Flag.GLOBAL_HEADER)) {
            encoder.setFlag(Encoder.Flag.FLAG_GLOBAL_HEADER, true);
        }
        encoder.open(null, null);
        muxer.addNewStream(encoder);
        try {
        muxer.open(null, null);
        }catch (Exception e) {}
        picture = MediaPicture.make(encoder.getWidth(), encoder.getHeight(), pixelformat);
        picture.setTimeBase(framerate);
    }

    void captureDisplayWindow() {
        loadPixels();
        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        int[] a = ((DataBufferInt) image.getRaster().getDataBuffer()).getData();
        System.arraycopy(pixels, 0, a, 0, pixels.length);
        add(image);
    }

    void add(BufferedImage image) {
      image = convertToType(image, BufferedImage.TYPE_3BYTE_BGR);
        if (converter == null) {
            converter = MediaPictureConverterFactory.createConverter(image, picture);
        }
        converter.toPicture(picture, image, frame++);

        do {
            encoder.encode(packet, picture);
            if (packet.isComplete()) {
                muxer.write(packet, false);
            }
        } while (packet.isComplete());
    }

    void close() {
        muxer.close();
    }
    
    BufferedImage convertToType(BufferedImage sourceImage, int targetType) {
       BufferedImage image = new BufferedImage(sourceImage.getWidth(), sourceImage.getHeight(), targetType);
       image.getGraphics().drawImage(sourceImage, 0, 0, null);

       return image;
   }

}

