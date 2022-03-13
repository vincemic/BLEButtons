#include "SoundsPlayer.h"

  

static SoundPlayer *myself;

#ifndef _BV
  #define _BV(x) (1<<(x))
#endif


volatile boolean feedBufferLock2 = false;

static void feeder(void) {  
  /* eziya76, set flag and exits ISR */
  //myself->feedBuffer();
  myself->DREQFlag = true;
}

boolean SoundPlayer::useInterrupt(uint8_t type) {
  myself = this;  // oy vey
    
  if (type == VS1053_FILEPLAYER_TIMER0_INT) {
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
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(feeder); 

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();
    
#else
    return false;
#endif
  }
  if (type == VS1053_FILEPLAYER_PIN_INT) {
    int8_t irq = digitalPinToInterrupt(_dreq);
    //Serial.print("Using IRQ "); Serial.println(irq);
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
               : Adafruit_VS1053(rst, cs, dcs, dreq) {

  playingMusic = false;
  _cardCS = cardcs;
}

SoundPlayer::SoundPlayer(
	       int8_t cs, int8_t dcs, int8_t dreq, 
	       int8_t cardcs) 
  : Adafruit_VS1053(-1, cs, dcs, dreq) {

  playingMusic = false;
  _cardCS = cardcs;
}

SoundPlayer::SoundPlayer(
               int8_t mosi, int8_t miso, int8_t clk, 
	       int8_t rst, int8_t cs, int8_t dcs, int8_t dreq, 
	       int8_t cardcs) 
               : Adafruit_VS1053(mosi, miso, clk, rst, cs, dcs, dreq) {

  playingMusic = false;
  _cardCS = cardcs;
}

boolean SoundPlayer::begin(void) {
  // Set the card to be disabled while we get the VS1053 up
  pinMode(_cardCS, OUTPUT);
  digitalWrite(_cardCS, HIGH);  

  uint8_t v  = Adafruit_VS1053::begin();   

  //dumpRegs();
  //Serial.print("Version = "); Serial.println(v);
  return (v == 4);
}


boolean SoundPlayer::playFullFile(const char *trackname) {
  if (! startPlayingFile(trackname)) return false;

  while (playingMusic) {
    // twiddle thumbs
    feedBuffer();
    delay(5);           // give IRQs a chance
  }
  // music file finished!
  return true;
}

void SoundPlayer::stopPlaying(void) {
  // cancel all playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_CANCEL);
  
  // wrap it up!
  playingMusic = false;
  currentTrack.close();
}

void SoundPlayer::pausePlaying(boolean pause) {
  if (pause) 
    playingMusic = false;
  else {
    playingMusic = true;
    feedBuffer();
  }
}

boolean SoundPlayer::paused(void) {
  return (!playingMusic && currentTrack);
}

boolean SoundPlayer::stopped(void) {
  return (!playingMusic && !currentTrack);
}

// Just checks to see if the name ends in ".mp3"
boolean SoundPlayer::isMP3File(const char* fileName) {
  return (strlen(fileName) > 4) && !strcasecmp(fileName + strlen(fileName) - 4, ".mp3");
}

unsigned long SoundPlayer::mp3_ID3Jumper(File mp3) {

  char tag[4];
  uint32_t start;
  unsigned long current;
	
  start = 0;
  if (mp3) {
  	current = mp3.position();
    if (mp3.seek(0)) {
      if (mp3.read((uint8_t*) tag,3)) {
      	tag[3] = '\0';
        if (!strcmp(tag, "ID3")) {
          if (mp3.seek(6)) {
            start = 0ul ;
            for (byte i = 0 ; i < 4 ; i++) {
              start <<= 7 ;
              start |= (0x7F & mp3.read()) ;
  	        }
          } else {
            //Serial.println("Second seek failed?");
          }
        } else {
          //Serial.println("It wasn't the damn TAG.");
        }
      } else {
        //Serial.println("Read for the tag failed");
      }
    } else {
      //Serial.println("Seek failed? How can seek fail?");
    }
    mp3.seek(current);	// Put you things away like you found 'em.
  } else {
    //Serial.println("They handed us a NULL file!");
  }
  //Serial.print("Jumper returning: "); Serial.println(start);
  return start;
}


boolean SoundPlayer::startPlayingFile(const char *trackname) {
  // reset playback
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW);
  // resync
  sciWrite(VS1053_REG_WRAMADDR, 0x1e29);
  sciWrite(VS1053_REG_WRAM, 0);

  currentTrack = SD.open(trackname);
  if (!currentTrack) {
    return false;
  }
    
  // We know we have a valid file. Check if .mp3
  // If so, check for ID3 tag and jump it if present.
  if (isMP3File(trackname)) {
    currentTrack.seek(mp3_ID3Jumper(currentTrack));
  }

  // don't let the IRQ get triggered by accident here
  noInterrupts();

  // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0
  sciWrite(VS1053_REG_DECODETIME, 0x00);
  sciWrite(VS1053_REG_DECODETIME, 0x00);

  playingMusic = true;

  // wait till its ready for data
  while (! readyForData() ) {
#if defined(ESP8266)
	yield();
#endif
  }

  // fill it up!
  while (playingMusic && readyForData()) {
    feedBuffer();
  }
  
  // ok going forward, we can use the IRQ
  interrupts();

  return true;
}

void SoundPlayer::feedBuffer(void) {
  noInterrupts();
  // dont run twice in case interrupts collided
  // This isn't a perfect lock as it may lose one feedBuffer request if
  // an interrupt occurs before feedBufferLock2 is reset to false. This
  // may cause a glitch in the audio but at least it will not corrupt
  // state.
  if (feedBufferLock2) {
    interrupts();
    return;
  }
  feedBufferLock2 = true;
  interrupts();

  feedBuffer_noLock();

  feedBufferLock2 = false;
}

void SoundPlayer::feedBuffer_noLock(void) {
  if ((! playingMusic) // paused or stopped
      || (! currentTrack) 
      || (! readyForData())) {
    return; // paused or stopped
  }

  // Feed the hungry buffer! :)
  while (readyForData()) {
    // Read some audio data from the SD card file
    int bytesread = currentTrack.read(mp3buffer, VS1053_DATABUFFERLEN);
    
    if (bytesread == 0) {
      // must be at the end of the file, wrap it up!
      playingMusic = false;
      currentTrack.close();
      break;
    }

    playData(mp3buffer, bytesread);
  }
}
