class CenterText {
  static final int ALPHA_MAX = 255;
  static final int FRAMES_FULL = 25;
  static final int FRAMES_FADE = 8;
  
  PFont font;
  int frame;
  String text;
  boolean fade;
  
  CenterText(PFont font) {
    this.font = font;
  }
  
  void showText(String text) {
    showText(text, true);
  }
  
  void showText(String text, boolean fade) {
    this.text = text;
    this.frame = 0;
    this.fade = fade;
  }
  
  void draw() {
    if (text != null && frame < FRAMES_FULL + FRAMES_FADE) {
      textFont(font);
      textAlign(CENTER, CENTER);
      fill(255, 255, 255, calcAlpha());
      textLeading(70);
      text(text, width/2, height/2);
      
      if (fade) {
        frame++;
      }
    }
  }
  
  private int calcAlpha() {
    if (frame < FRAMES_FULL) {
      return ALPHA_MAX;
    } else {
      return ALPHA_MAX - (frame - FRAMES_FULL)*ALPHA_MAX/FRAMES_FADE;
    }
  }
}
