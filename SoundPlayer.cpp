#include "SoundsPlayer.h"
#include <ArduinoLog.h>

#include "LoopbackStream.h"


static SoundPlayer *myself;
LoopbackStream superStream(50000);

#ifndef _BV
#define _BV(x) (1 << (x))
#endif

IRAM_ATTR void feeder(void)
{
  myself->_dREQFlag = true;
  myself->feedBuffer();

}

boolean SoundPlayer::useInterrupt(uint8_t type)
{
  myself = this; // oy vey

  if (type == VS1053_FILEPLAYER_TIMER0_INT)
  {
#if defined(__AVR__)
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    return true;
#elif defined(__arm__) && defined(CORE_TEENSY)
    IntervalTimer *t = new IntervalTimer();
    return (t && t->begin(feeder, 1024)) ? true : false;
#elif defined(ARDUINO_STM32_FEATHER)
    HardwareTimer timer(3);
    // Pause the timer while we're configuring it
    timer.pause();

    // Set up period
    timer.setPeriod(25000); // in microseconds

    // Set up an interrupt on channel 1
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1); // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(feeder);

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();

#else
    return false;
#endif
  }
  if (type == VS1053_FILEPLAYER_PIN_INT)
  {
    int8_t irq = digitalPinToInterrupt(_dreq);
    // Serial.print("Using IRQ "); Serial.println(irq);
    if (irq == -1)
      return false;
#if defined(SPI_HAS_TRANSACTION) && !defined(ESP8266) && !defined(ESP32) && !defined(ARDUINO_STM32_FEATHER)
    SPI.usingInterrupt(irq);
#endif
    /* eziya76, changed from CHANGE to RISING */
    attachInterrupt(irq, feeder, RISING);
    return true;
  }
  return false;
}

SoundPlayer::SoundPlayer(
    int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
    int8_t cardcs)
    : Adafruit_VS1053(rst, cs, dcs, dreq)
{
  superStream.setLoopOff();
  playingMusic = false;
  _cardCS = cardcs;
}

boolean SoundPlayer::begin(void)
{
  // Set the card to be disabled while we get the VS1053 up
  //pinMode(_cardCS, OUTPUT);
  //digitalWrite(_cardCS, HIGH);

  uint8_t v = Adafruit_VS1053::begin();

  useInterrupt(VS1053_FILEPLAYER_PIN_INT); // DREQ int

  //if (SD.begin(_cardCS))
  //{
    //printDirectory("/", 0);
 // }

 

  // dumpRegs();
  // Serial.print("Version = "); Serial.println(v);
  return (v == 4);
}

void SoundPlayer::stop(void)
{
  // wrap it up!
  playingMusic = false;
    delay(500);

  while(feedLock)
    delay(5);

  // cancel all playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);

  delay(5);
}

boolean SoundPlayer::stopped(void)
{
  return (!playingMusic);
}

// Just checks to see if the name ends in ".mp3"
boolean SoundPlayer::isMP3File(const char *fileName)
{
  return (strlen(fileName) > 4) && !strcasecmp(fileName + strlen(fileName) - 4, ".mp3");
}

unsigned long SoundPlayer::mp3_ID3Jumper(File mp3)
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

boolean SoundPlayer::play(const char *trackname)
{
  stop();

  // reset playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  // resync
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);

  delay(5);
  
  superStream.clear();
  File currentTrack = SD.open(trackname);

  if (!currentTrack)
  {
    return false;
  }

  // We know we have a valid file. Check if .mp3
  // If so, check for ID3 tag and jump it if present.
  if (isMP3File(trackname))
  {
    currentTrack.seek(mp3_ID3Jumper(currentTrack));
  }

  // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
  sciWrite(VS1053_REG_DECODETIME, 0x00);
  sciWrite(VS1053_REG_DECODETIME, 0x00);


  while (currentTrack.available() > 0 && superStream.availableForWriteLarge() >= VS1053_DATABUFFERLEN)
  {
    size_t count = currentTrack.read(mp3buffer, VS1053_DATABUFFERLEN);

    for (size_t index = 0; index < count; index++)
    {
      superStream.write(mp3buffer[index]);
    }
  }

  currentTrack.close();

  playingMusic = true;

  // fill it up!
  feedBuffer();

  return true;
}

void SoundPlayer::feedBuffer(void)
{
  if(feedLock)
    return;
  
  feedLock = true;

  if (!playingMusic)
  {
    feedLock = false;
    return; 
  }

  // Feed the hungry buffer! :)
  while (readyForData() && playingMusic)
  {
    int bytesread = 0;

    while (superStream.availableLarge() > 0 && bytesread < VS1053_DATABUFFERLEN)
    {
      mp3buffer[bytesread] = superStream.read();
      bytesread++;
    }

    if (bytesread == 0)
    {
      // must be at the end of the file, wrap it up!
      playingMusic = false;
      break;
    }

    playData(mp3buffer, bytesread);
  }

  feedLock = false;
}

void SoundPlayer::report(JsonDocument &jsonDocument)
{
    auto o = jsonDocument["files"];
}

void SoundPlayer::printDirectory(const char *path, int numTabs)
{
  File dir = SD.open(path);

  if (dir && dir.isDirectory())
    printDirectory(dir, numTabs);

  dir.close();
}
void SoundPlayer::printDirectory(File dir, int numTabs)
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
