#pragma once
#include <Adafruit_VS1053.h>
#include <ArduinoJson.h>

class SoundPlayer : public Adafruit_VS1053
{

public:
  SoundPlayer(int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
              int8_t cardCS);

  volatile boolean _dREQFlag = false;

  boolean begin(void);
  boolean useInterrupt(uint8_t type);
  volatile boolean playingMusic;
  void feedBuffer();
  static boolean isMP3File(const char *fileName);
  unsigned long mp3_ID3Jumper(File mp3);
  boolean play(const char *trackname);
  void stop(void);
  boolean stopped(void);
  void pausePlaying(boolean pause);

  void printDirectory(const char *path, int numTabs);
  void printDirectory(File dir, int numTabs);
void report(JsonDocument &jsonDocument);

private:
  uint8_t _cardCS;
  File currentTrack;
};
