class Race {
  static final int COLUMN_1 = 20;
  
  static final int COLUMN_2 = 140;
  
  static final int COLUMN_3 = 180;
  
  static final int ROW_1 = 400;
  
  static final int ROW_HEIGHT = 18;
  
  private final String pilot;
  private final PFont textFont;
  private int lap;
  private int bestLap;
  private long raceStartTime;
  private long lapStartTime;
  private String lastLapTime;
  private String bestLapTime;
  
  Race(String pilot, PFont textFont) {
    this.pilot = pilot;
    this.textFont = textFont;
    reset();
  }
  
  void reset() {
    lap = 1;
    bestLap = 0;
    bestLapTime = "";
    lastLapTime = "";
  }
  
  void startRace() {
    long now = millis();
    raceStartTime = now;
    lapStartTime = now;
  }
  
  void nextLap(String lastLapTime, boolean newBestLap) {
    lap++;
    this.lastLapTime = lastLapTime;
    if (newBestLap) {
      this.bestLapTime = bestLapTime;
    }
  }
  
  void draw() {    
    long now = millis();
    long raceTime = now - raceStartTime;
    long lapTime = now - lapStartTime;
    
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
    int y = ROW_1;
    text("PILOT", COLUMN_1, y);
    y += ROW_HEIGHT;
    text("LAP", COLUMN_1, y);
    y += ROW_HEIGHT;
    text("RACE TIME", COLUMN_1, y);
    y += ROW_HEIGHT;
    text("LAP TIME", COLUMN_1, y);
    y += ROW_HEIGHT;
    text("FASTEST LAP", COLUMN_1, y);

    fill(255, 255, 255);
    y = ROW_1;
    text(pilot, COLUMN_2, y);
    y += ROW_HEIGHT;
    text(lap, COLUMN_2, y);
    y += ROW_HEIGHT;
    text(formatDuration(raceTime), COLUMN_2, y);
    y += ROW_HEIGHT;
    text(formatDuration(lapTime), COLUMN_2, y);
    y += ROW_HEIGHT;
    text(bestLap, COLUMN_2, y);
    text(bestLapTime, COLUMN_3, y);
  }
  
  private String formatDuration(long duration) {
    return String.format("% 2d:%02d.%02d", duration/60000, duration/1000 % 60, duration / 10 % 100);
  }
}

