class Race {
  static final int COLUMN_1 = 20;
  
  static final int COLUMN_2 = 140;
  
  static final int ROW_1 = 400;
  
  static final int ROW_HEIGHT = 18;
  
  private final String pilot;
  private final PFont textFont;
  private final CenterText centerText;
  private int lap;
  private int totalLaps;
  private int bestLap;
  private long raceStartTime;
  private long raceFinishTime;
  private long lapStartTime;
  private long bestLapTime;
  
  Race(String pilot, PFont textFont, CenterText centerText) {
    this.pilot = pilot;
    this.textFont = textFont;
    this.centerText = centerText;
    reset();
  }
  
  void reset() {
    lap = 1;
    raceStartTime = 0;
    bestLap = 0;
    raceFinishTime = 0;
    bestLapTime = Long.MAX_VALUE;
  }
  
  void startRace(int totalLaps) {
    this.totalLaps = totalLaps;
    long now = millis();
    raceStartTime = now;
    lapStartTime = now;
    centerText.showText("START");
  }
  
  void nextLap() {
    long now = millis();
    long lastLapTime = now - lapStartTime;
    lapStartTime = now;
    String text = formatDuration(lastLapTime).trim();
    
    boolean newBestLap = bestLapTime > lastLapTime; 
    if (newBestLap) {
      bestLap = lap;
      bestLapTime = lastLapTime;
      text += "!";
    }
    
    lap++;
    if (lap == totalLaps) {
      text = "FINAL LAP\n" + text;
    }
    
    centerText.showText(text);
  }
  
  void finish() {
    raceFinishTime = millis();
    lap--;
    centerText.showText(String.format("FINISH\n%s", formatDuration(raceFinishTime - raceStartTime).trim()), false);
  }
  
  void draw() {
    boolean raceStarted = raceStartTime > 0;
    
    long finishTime;
    if (raceFinishTime > 0) {
      finishTime = raceFinishTime;
    } else {
       finishTime = millis();
    }
    long raceTime = finishTime - raceStartTime;
    long lapTime = finishTime - lapStartTime;
    
    int boxY = ROW_1 - ROW_HEIGHT - 5;
    int boxHeight = ROW_HEIGHT * 5 + 10;
    stroke(255.0, 255.0, 255.0, 70.0);
    fill(255.0, 255.0, 255.0, 70.0);
    rect(COLUMN_1 - 10, boxY, COLUMN_2 - COLUMN_1 + 5, boxHeight);
    stroke(255.0, 128.0, 128.0, 70.0);
    fill(255.0, 128.0, 128.0, 70.0);
    rect(COLUMN_2 - 5, boxY, 120, boxHeight);

    textFont(textFont);
    textAlign(LEFT, BASELINE);

    fill(0);
    text("PILOT", COLUMN_1, rowY(0));
    text("LAP", COLUMN_1, rowY(1));
    text("RACE TIME", COLUMN_1, rowY(2));
    text("LAP TIME", COLUMN_1, rowY(3));
    text("FASTEST LAP", COLUMN_1, rowY(4));

    fill(255, 255, 255);
    text(pilot, COLUMN_2, rowY(0));
    if (raceStarted) {
      text(String.format("% 3d / %d", lap, totalLaps), COLUMN_2, rowY(1));
      text("    " + formatDuration(raceTime), COLUMN_2, rowY(2));
      text("    " + formatDuration(lapTime), COLUMN_2, rowY(3));
      if (bestLap > 0) {
        text(String.format("% 3d %s", bestLap, formatDuration(bestLapTime)), COLUMN_2, rowY(4));
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

