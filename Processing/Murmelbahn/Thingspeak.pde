import http.requests.*;
import java.util.concurrent.*;

class Thingspeak {
  private final ExecutorService executor = Executors.newSingleThreadExecutor();
  
  void sendRunTime(long timeInMillis) {
    send(1, timeInMillis);
  }
  
  void send(final int field, final long value) {
    executor.execute(new Runnable() {
      public void run() {
        PostRequest post = new PostRequest("https://api.thingspeak.com/update");
        post.addData("key", "(your channel key here)");
        post.addData("field" + field, String.valueOf(value));
        post.send();
      }
    });
  }
}
