class CenterText {
  static final int ALPHA_MAX = 255;
  static final int FRAMES_FULL = 30;
  static final int FRAMES_FADE = 8;
  
  PFont font;
  int frame;
  String text;
  
  CenterText(PFont font) {
  }
  
  void showText(String text) {
    this.text = text;
    this.frame = 0;
  }
  
  void draw() {
    if (text != null && frame < FRAMES_FULL + FRAMES_FADE) {
      textFont(bigFont);
      textAlign(CENTER, CENTER);
      fill(255, 255, 255, calcAlpha());
      textLeading(70);
      text(text, width/2, height/2);
      
      frame++;
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
