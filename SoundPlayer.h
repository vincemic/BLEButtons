#pragma once

#include <Adafruit_VS1053.h>
#include <ArduinoJson.h>

struct FrequencyMapItem
{
  uint16_t topFrequency = 0;
  uint8_t sampleRateIndex = 0;

  FrequencyMapItem(uint16_t topFrequency, uint8_t sampleRateIndex)
  {
    this->topFrequency = topFrequency;
    this->sampleRateIndex = sampleRateIndex;
  }
};
class SoundPlayerClass : public Adafruit_VS1053
{

public:
  SoundPlayerClass();

  FrequencyMapItem frequencyMap[8] = {{48000, 1}, {44100, 0}, {32000, 2}, {24000, 4}, {22050, 3}, {16000, 5}, {12000, 7}, {11025, 6}};
  volatile boolean dREQFlag = false;
  TaskHandle_t taskHandle;
  boolean begin();
  volatile boolean playingMusic;
  void feedBuffer();
  static boolean isMP3File(const char *fileName);
  unsigned long mp3_ID3Jumper(File mp3);
  boolean play(const char *trackfilepath);
  void stop();
  boolean isStopped();
  void pausePlaying(boolean pause);
  void playTone(uint16_t frequency, uint16_t milliseconds, uint8_t volumeLeft, uint8_t volumeRight );

  void printDirectory(const char *path, int numTabs);
  void printDirectory(File dir, int numTabs);
  void report(JsonDocument &jsonDocument);
  FrequencyMapItem calculateSampleRateIndex(uint16_t frequency);
  uint8_t calculateSineSkipSpeed(uint16_t frequency, uint16_t sampleRate);

private:
  uint8_t cardCS;
  File currentTrack;
};

extern SoundPlayerClass SoundPlayer;