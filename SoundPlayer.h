#pragma once
#include <Adafruit_VS1053.h>
#include <ArduinoJson.h>

class SoundPlayerClass : public Adafruit_VS1053
{

public:
  SoundPlayerClass();

  volatile boolean dREQFlag = false;

  boolean begin();
  volatile boolean playingMusic;
  void feedBuffer();
  static boolean isMP3File(const char *fileName);
  unsigned long mp3_ID3Jumper(File mp3);
  boolean play(const char *trackfilepath);
  void stop();
  boolean isStopped();
  void pausePlaying(boolean pause);

  void printDirectory(const char *path, int numTabs);
  void printDirectory(File dir, int numTabs);
  void report(JsonDocument &jsonDocument);

private:
  uint8_t cardCS;
  File currentTrack;
};

extern SoundPlayerClass SoundPlayer;