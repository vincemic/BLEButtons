#include "SoundPlayer.h"
#include <ArduinoLog.h>

#ifndef _BV
#define _BV(x) (1 << (x))
#endif

// Mapping specifc to Unexpected Maker ESP32-S3 Feather
#define VS1053_RESET -1 // VS1053 reset pin (not used!)
#define VS1053_CS 38    // VS1053 chip select pin (output)
#define VS1053_DCS 3    // VS1053 Data/command select pin (output)
#define CARDCS 33       // Card chip select pin
#define VS1053_DREQ 1   // VS1053 Data request, ideally an Interrupt pin
#define VS1053_CONTROL_SPI_SETTING SPISettings(250000, MSBFIRST, SPI_MODE0)
#define VS1053_DATA_SPI_SETTING SPISettings(8000000, MSBFIRST, SPI_MODE0)

SoundPlayerClass::SoundPlayerClass()
    : Adafruit_VS1053(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ)
{
  playingMusic = false;
  cardCS = CARDCS;
}

boolean SoundPlayerClass::begin(void)
{
  uint8_t v = Adafruit_VS1053::begin();

  if (v != 4)
  {
    Log.errorln(F("[SoundPlayer] Couldn't initialize VS1053"));
    return false;
  }

  Log.errorln(F("[SoundPlayer] Found VS1053"));

  if (!SD.begin(CARDCS))
  {
    Log.errorln(F("[SoundPlayer] Couldn't initialize SD card"));
    return false;
  }

  Log.errorln(F("[SoundPlayer] Found SD card"));

  int8_t irq = digitalPinToInterrupt(_dreq);

  attachInterrupt(
      irq, []() IRAM_ATTR
      { SoundPlayer.dREQFlag = true; },
      RISING);

  xTaskCreatePinnedToCore(
      [](void *parameters)
      {
        while (true)
        {
          if (SoundPlayer.dREQFlag)
          {
            SoundPlayer.dREQFlag = false;
            SoundPlayer.feedBuffer();
          }

          vTaskDelay(40);
        }
      },
      "feederTask",
      2048, // This stack size can be checked & adjusted by reading the Stack Highwater
      NULL, //
      2,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      &taskHandle,
      0);

  return true;
}

void SoundPlayerClass::stop()
{
  // wrap it up!
  playingMusic = false;

  delay(1000);

  if (currentTrack)
    currentTrack.close();

  // cancel all playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);

  delay(5);
}

boolean SoundPlayerClass::isStopped()
{
  return (!playingMusic);
}

// Just checks to see if the name ends in ".mp3"
boolean SoundPlayerClass::isMP3File(const char *fileName)
{
  return (strlen(fileName) > 4) && !strcasecmp(fileName + strlen(fileName) - 4, ".mp3");
}

unsigned long SoundPlayerClass::mp3_ID3Jumper(File mp3)
{

  char tag[4];
  uint32_t start;
  unsigned long current;

  start = 0;
  if (mp3)
  {
    current = mp3.position();
    if (mp3.seek(0))
    {
      if (mp3.read((uint8_t *)tag, 3))
      {
        tag[3] = '\0';
        if (!strcmp(tag, "ID3"))
        {
          if (mp3.seek(6))
          {
            start = 0ul;
            for (byte i = 0; i < 4; i++)
            {
              start <<= 7;
              start |= (0x7F & mp3.read());
            }
          }
          else
          {
            // Serial.println("Second seek failed?");
          }
        }
        else
        {
          // Serial.println("It wasn't the damn TAG.");
        }
      }
      else
      {
        // Serial.println("Read for the tag failed");
      }
    }
    else
    {
      // Serial.println("Seek failed? How can seek fail?");
    }
    mp3.seek(current); // Put you things away like you found 'em.
  }
  else
  {
    // Serial.println("They handed us a NULL file!");
  }
  // Serial.print("Jumper returning: "); Serial.println(start);
  return start;
}

boolean SoundPlayerClass::play(const char *trackfilepath)
{
  stop();

  // reset playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  // resync
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);

  delay(5);

  currentTrack = SD.open(trackfilepath);

  if (!currentTrack)
  {
    return false;
  }

  // We know we have a valid file. Check if .mp3
  // If so, check for ID3 tag and jump it if present.
  if (isMP3File(trackfilepath))
  {
    currentTrack.seek(mp3_ID3Jumper(currentTrack));
  }

  // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
  sciWrite(VS1053_REG_DECODETIME, 0x00);
  sciWrite(VS1053_REG_DECODETIME, 0x00);

  playingMusic = true;

  return true;
}

void SoundPlayerClass::feedBuffer()
{
  if (!playingMusic)
  {
    return;
  }

  // Feed the hungry buffer! :)
  while (readyForData() && playingMusic)
  {
    size_t bytesRead = 0;

    bytesRead = currentTrack.readBytes((char *)mp3buffer, VS1053_DATABUFFERLEN);

    if (bytesRead > 0)
    {
      playData(mp3buffer, bytesRead);
    }
    else
    {
      playingMusic = false;
      currentTrack.close();
    }
  }
}

void SoundPlayerClass::report(JsonDocument &jsonDocument)
{
  auto o = jsonDocument["files"];
}

void SoundPlayerClass::printDirectory(const char *path, int numTabs)
{
  File dir = SD.open(path);

  if (dir && dir.isDirectory())
    printDirectory(dir, numTabs);

  dir.close();
}
void SoundPlayerClass::printDirectory(File dir, int numTabs)
{
  while (true)
  {

    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      // Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++)
    {
      Log.trace("\t");
    }
    Serial.print(entry.name());
    if (entry.isDirectory())
    {
      Log.traceln("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Log.trace("\t\t");
      Log.traceln("%d", entry.size());
    }
    entry.close();
  }
}

FrequencyMapItem SoundPlayerClass::calculateSampleRateIndex(uint16_t frequency)
{
  FrequencyMapItem result = frequencyMap[0];

  for (FrequencyMapItem &item : frequencyMap)
  {
    if (frequency > item.topFrequency)
      break;
    result = item;
  }

  return result;
}
uint8_t SoundPlayerClass::calculateSineSkipSpeed(uint16_t frequency, uint16_t sampleRate)
{
  uint8_t result = (frequency * 128) / sampleRate;

  if (result > 31)
    result = 31;

  return result;
}

void SoundPlayerClass::playTone(uint16_t frequency, uint16_t milliseconds, uint8_t volumeLeft, uint8_t volumeRight )
{
  FrequencyMapItem sineSampleRateIndex = calculateSampleRateIndex(frequency);
  // Its a bit weird but this is how to do this accroding to what a translated from the docs
  uint8_t sineSkipSpeed = calculateSineSkipSpeed(frequency, sineSampleRateIndex.topFrequency);
  uint8_t frequencyCommand = sineSampleRateIndex.sampleRateIndex << 5; // move the sampleRateIndex over 5 bits;
  frequencyCommand = frequencyCommand | sineSkipSpeed;                 // add the sineSkipSpeed with logic <or>

  Log.traceln(F("Frequency: %d frequencyCommand: %d sineSkipSpeed: %d sineSampleRateIndex: %d"), frequency, frequencyCommand, sineSkipSpeed, sineSampleRateIndex.topFrequency);

  reset();

  setVolume(volumeLeft,volumeRight);

  uint16_t mode = sciRead(VS1053_REG_MODE);
  mode |= 0x0020;
  sciWrite(VS1053_REG_MODE, mode);

  while (!digitalRead(_dreq))
    ;
    //  delay(10);

#ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI)
    SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
#endif
  digitalWrite(_dcs, LOW);
  spiwrite(0x53);
  spiwrite(0xEF);
  spiwrite(0x6E);
  spiwrite(frequencyCommand);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  digitalWrite(_dcs, HIGH);
#ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI)
    SPI.endTransaction();
#endif

  delay(milliseconds);

#ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI)
    SPI.beginTransaction(VS1053_DATA_SPI_SETTING);
#endif
  digitalWrite(_dcs, LOW);
  spiwrite(0x45);
  spiwrite(0x78);
  spiwrite(0x69);
  spiwrite(0x74);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  digitalWrite(_dcs, HIGH);
#ifdef SPI_HAS_TRANSACTION
  if (useHardwareSPI)
    SPI.endTransaction();
#endif
}
SoundPlayerClass SoundPlayer;