class Race {
  static final int COLUMN_1 = 20;
  
  static final int COLUMN_2 = 160;
  
  static final int ROW_1 = 400;
  
  static final int ROW_HEIGHT = 18;
  
  private final Thingspeak thingspeak = new Thingspeak();
  private final PFont textFont;
  private final CenterText centerText;
  private int lap;
  private int bestLap;
  private long lapStartTime;
  private long lastLapTime;
  private long bestLapTime;
  
  Race(PFont textFont, CenterText centerText) {
    this.textFont = textFont;
    this.centerText = centerText;
    bestLap = 0;
    bestLapTime = Long.MAX_VALUE;
  }
  
  void ready() {
    centerText.showText("READY");
  }
  
  void startRace() {
    long now = millis();
    lapStartTime = now;
    centerText.showText("START");
    lap++;
  }
  
  void finish() {
    lastLapTime = millis() - lapStartTime;
    lapStartTime = 0;
    String text = String.format("FINISH\n%s", formatDuration(lastLapTime).trim());
    boolean newBestLap = bestLapTime > lastLapTime; 
    if (newBestLap) {
      bestLap = lap;
      bestLapTime = lastLapTime;
      text += "!";
    }
    centerText.showText(text);
    thingspeak.sendRunTime(lastLapTime);
  }
  
  void draw() {
    boolean raceStarted = lapStartTime > 0;
    
    int boxY = ROW_1 - ROW_HEIGHT - 5;
    int boxHeight = ROW_HEIGHT * 5 + 10;
    stroke(255.0, 255.0, 255.0, 0.0);
    fill(255.0, 255.0, 255.0, 70.0);
    rect(COLUMN_1 - 10, boxY, COLUMN_2 - COLUMN_1 + 5, boxHeight);
    stroke(255.0, 128.0, 128.0, 70.0);
    fill(255.0, 128.0, 128.0, 70.0);
    rect(COLUMN_2 - 5, boxY, 120, boxHeight);

    textFont(textFont);
    textAlign(LEFT, BASELINE);

    fill(0);
    text("LAP", COLUMN_1, rowY(0));
    text("CURRENT LAP", COLUMN_1, rowY(1));
    text("LAST LAP", COLUMN_1, rowY(2));
    text("FASTEST LAP", COLUMN_1, rowY(3));

    fill(255, 255, 255);
    if (lap > 0) {
      text(String.format("% 3d", lap), COLUMN_2, rowY(0));
      if (lapStartTime > 0) {
        text("    " + formatDuration(millis() - lapStartTime), COLUMN_2, rowY(1));
      }
      text("    " + formatDuration(lastLapTime), COLUMN_2, rowY(2));
      if (bestLap > 0) {
        text(String.format("% 3d %s", bestLap, formatDuration(bestLapTime)), COLUMN_2, rowY(3));
      }
    }
  }
  
  private int rowY(int row) {
    return ROW_1 + row*ROW_HEIGHT;
  }
  
  private String formatDuration(long duration) {
    return String.format("% 2d:%02d.%02d", duration/60000, duration/1000 % 60, duration / 10 % 100);
  }
}

