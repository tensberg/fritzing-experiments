FRITZING EXPERIMENTS
====================

Simple toy projects based on the [Fritzing creator kit](http://fritzing.org/creatorkit) and some additional parts.

Contains the Arduino, Processing and Fritzing sketches.

See [my Google+ album](https://plus.google.com/photos/106550818727370453007/albums/5975507440855889889) for demo images and movies.

# SevenSegmentDisplay

Use a seven segment display to show digits. The buttons toggle between different modes.

# Countdown

Show a countdown on a seven segment display and start a motor. Combines parts from the Fritzing creator kit with others from the Kosmos Easy Electronic box.

# Lichtsucher

A photo sensor in the head of the Fritzing Cardboard Bot is used to find the brightest spot. This can be used to track a flashlight.

[![Fritzing Cardboard Bot](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/Lichtsucher_thumb.jpg)](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/Lichtsucher.jpg)

# RoboFaceTracker

A Processing sketch which uses the Arduino firmata firmware. Uses a webcam and OpenCV for Processing to detect faces. Makes the Fritzing Cardboard Bot always look at you.

# SlotCarTimer

Uses the photo sensor to build a light barrier to measure lap times of a slot car racer. Lap and race times are shown on a 16x2 character display and printed on the serial port. The Arduino can be used standalone. You can additionally use the Processing sketch to overlay a webcam capture of your race with the race times.

[![Slot Car Timer Photo](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/SlotCarTimer_thumb.jpg "Slot Car Timer")](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/SlotCarTimer.jpg)

![Slot Car Timer Processing Screenshot](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/SlotCarTimer_Processing.jpg "Slot Car Timer Processing Screenshot")

[Slot Car Timer YouTube video](http://youtu.be/XoMMHVWk8uI)

# Murmelbahn

Switches at the start and end of the track of a Fischertechnik Dynamic marble run measure the run time. A motor controller stops the motor when the marble reaches the top and starts the run when a button is pressed. The processing sketch overlays a webcam image with the run time. In true IoT style, the marble run also has a [ThingSpeak channel](https://thingspeak.com/channels/26266).


[![Murmelbahn Photo](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/marble_run_thumb.jpg "Murmelbahn")](https://raw.github.com/wiki/tensberg/fritzing-experiments/img/marble_run.jpg)

[Murmelbahn YouTube video](http://youtu.be/2_CHgmET9PY)
