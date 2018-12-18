Larson Scanner has 3 levels of brightness as it moves back and forth across the LEDs.

I was using 7 LEDs with my setup, but by changing the #LEDCOUNT constant, you can easily adjust to your number of LEDs.
They will be initialized and set using 20 as the first pin, and all subsequent pins by integer.

I used a 2.2k Ohm resistor, as the LEDs I used were a little too bright for my taste. The higher the resistance used would provide an overall dimmer output. I could imagine shifting the resistances could create some variance in the 3 different levels of brightness. By adjusting the #DIM and #DIMMER values, one could compensate. (Raising these values increases the number of loops each LED is SET and CLR'd each loop, which {FOR SOME REASON} increases the brightness of each LED.)

I have attached two videos to the respository, demonstrating the scanner in action. One video shows it at full speed, and in another video, I adjusted the #DELAY1, #DELAY2 and #DELAY3 values, slowing it down which makes it easier to see the differing levels of brightness.

WOOHOO!