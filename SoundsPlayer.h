#include <Adafruit_VS1053.h>

class SoundPlayer : public Adafruit_VS1053 {

public:
    SoundPlayer();

public:
  SoundPlayer (int8_t mosi, int8_t miso, int8_t clk, 
			      int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);
  SoundPlayer (int8_t rst, int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);
  SoundPlayer (int8_t cs, int8_t dcs, int8_t dreq,
			      int8_t cardCS);

  /* eziya76, Add flag for interrupt handling */
  boolean DREQFlag;

  boolean begin(void);
  boolean useInterrupt(uint8_t type);
  File currentTrack;
  volatile boolean playingMusic;
  void feedBuffer(void);
  static boolean isMP3File(const char* fileName);
  unsigned long mp3_ID3Jumper(File mp3);
  boolean startPlayingFile(const char *trackname);
  boolean playFullFile(const char *trackname);
  void stopPlaying(void);
  boolean paused(void);
  boolean stopped(void);
  void pausePlaying(boolean pause);

 private:
  void feedBuffer_noLock(void);

  uint8_t _cardCS;
};

