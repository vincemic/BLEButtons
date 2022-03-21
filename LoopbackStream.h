#pragma once

#include <Stream.h>

/*
 * A LoopbackStream stores all data written in an internal buffer and returns it back when the stream is read.
 *
 * If the buffer overflows, the last bytes written are lost.
 *
 * It can be used as a buffering layer between components.
 */
class LoopbackStream : public Stream
{
  uint8_t *buffer;
  size_t buffer_size;
  size_t pos, size;
  bool shouldLoop = true;


public:
  static const uint16_t DEFAULT_SIZE = 64;

  LoopbackStream(size_t buffer_size = LoopbackStream::DEFAULT_SIZE);
  ~LoopbackStream();

  /** Clear the buffer */
  void clear();

  virtual size_t write(uint8_t);
  virtual int availableForWrite(void);
  virtual int available();
  virtual size_t availableForWriteLarge(void);
  virtual size_t availableLarge();
  virtual bool contains(char);
  virtual int read();
  virtual int peek();
  virtual void flush();
  virtual void setLoopOff();
};
