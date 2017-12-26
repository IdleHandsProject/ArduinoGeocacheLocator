#include <Wire.h>
#include <HMC6352.h>
#include <TinyGPS++.h>


static const double Loc_Lat[] = {0.267732, 0.269746, 0.270536, 0.266319};
static const double Loc_Lon[] = {0.947902, 0.968261, 0.947758,  0.942634};

int CurrDest = 0;
int LastDest = 0;

int red = 0;
int green = 0;
int blue = 0;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define BUTT 4
#define PIN            1
#define NUMPIXELS      16
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int spread = 0;
int dircourse = 0;
int DestDist = 0;

TinyGPSPlus gps;

void setup()
{
 
  pinMode(BUTT, INPUT_PULLUP);
  Wire.begin();
  Serial.begin(57600);
  Serial1.begin(57600);

  pinMode(BUTT, INPUT_PULLUP);
  pixels.begin(); // This initializes the NeoPixel library.
  delay(100);

  Serial.println("RST");
  delay(100);
  while (Serial1.available() > 0)
    gps.encode(Serial1.read());
  while (gps.location.isUpdated() == 0) {
    rainbowCycle(1);
    while (Serial1.available() > 0)
      gps.encode(Serial1.read());
    delay(100);
    Serial.println("Waiting for GPS");
  }

  DestColorBlink(CurrDest);

}

void loop()
{
  int buttstate = digitalRead(BUTT);
  if (buttstate == 0) {
    DestChange();
  }


  HMC6352.Wake();
  int north = HMC6352.GetHeading();
  HMC6352.Sleep();


  while (Serial1.available() > 0)
    gps.encode(Serial1.read());

  if (gps.location.isValid())
  {

    double distanceToLoc =
      TinyGPSPlus::distanceBetween(
        gps.location.lat(),
        gps.location.lng(),
        Loc_Lat[CurrDest],
        Loc_Lon[CurrDest]);
    double courseToLoc =
      TinyGPSPlus::courseTo(
        gps.location.lat(),
        gps.location.lng(),
        Loc_Lat[CurrDest],
        Loc_Lon[CurrDest]);

    spread = map(distanceToLoc, 0 , 1000, 7, 0);
    DestDist = distanceToLoc;
    spread = constrain(spread, 0, 6);
    dircourse = north - courseToLoc;
  }
  delay(100);
  if (DestDist > 500) {
    red = map(DestDist, 500, 1000, 5, 25);
    red = constrain(red, 0, 35);
  }
  else {
    red = 0;
  }
  if (DestDist < 700) {
    green = map(DestDist, 0, 700, 25, 0);
    green = constrain(green, 0, 25);
  }
  else {
    green = 0;
  }
  if (dircourse < 0) {
    dircourse = dircourse + 360;
  }
  Serial.println(DestDist);
  if (DestDist > 10) {
    dirSpread(dircourse, spread);
  }
  else {
    theaterChaseRainbow(50);
  }
  pixels.show();
}

void dirSpread(int dir, int spread) {
  int dirs = map(dir, 0, 360, -4, 12);
  if (dirs < 0) {
    dirs = dirs + 16;
  }
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  for (int i = 0; i < NUMPIXELS; i++) {
    int upper = dirs + spread;
    int lower = dirs - spread;
    if ((i <= upper) && (i >= lower) ) {
      pixels.setPixelColor(i, pixels.Color(red, green, 0)); // Moderately bright green color.
    }
    if (upper > 15) {
      if (i == 15) {
        int upset = upper - 15;
        //Serial.println(upset);
        for (int j = 0; j < upset; j++) {
          int upperpix = i + j;
          upperpix = upperpix - 15;
          pixels.setPixelColor(upperpix, pixels.Color(red, green, 0));

        }
      }
    }
    if (lower < 0) {
      if (i == 0) {
        int lowset = lower + 16;
        //Serial.println(lowset);
        for (int k = lowset; k < 16; k++) {
          //Serial.println(k);
          int lowerpix = i + k;
          //lowerpix = lowerpix + 15;
          //Serial.println(lowerpix);
          pixels.setPixelColor(lowerpix, pixels.Color(red, green, 0));
        }
      }
    }
  }
}

void DestChange() {
  pixels.show();
  int buttstate = digitalRead(4);
  while (buttstate == 0) {
    buttstate = digitalRead(4);
  }
  CurrDest = CurrDest + 1;
  if (CurrDest > 3) {
    CurrDest = 0;
  }
  if (LastDest != CurrDest) {
    DestColorBlink(CurrDest);
    LastDest = CurrDest;
  }
}

void DestColorBlink(int color) {
  if (color == 0) {
    colorWipe(pixels.Color(100, 0, 0), 50);
  }
  if (color == 1) {
    colorWipe(pixels.Color(0, 100, 0), 50);
  }
  if (color == 2) {
    colorWipe(pixels.Color(0, 0, 100), 50);
  }
  if (color == 3) {
    colorWipe(pixels.Color(100, 100, 0), 50);
  }
  delay(1000);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i = 0; i < pixels.numPixels(); i = i + 3) {
        pixels.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
