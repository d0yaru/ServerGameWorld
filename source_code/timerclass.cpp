////////////////////////////////////////////////////////////////////////////////
// Filename: timerclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "timerclass.h"


TimerClass::TimerClass()
{
}


TimerClass::TimerClass(const TimerClass& other)
{
}


TimerClass::~TimerClass()
{
}


void TimerClass::Initialize()
{
  // Get the current time.
  gettimeofday(&m_time, 0);

  // Get the microseconds from the struct.
  m_startTime = m_time.tv_usec;

  return;
}


void TimerClass::Frame()
{
  long currentTime;


  // Query the current time.
  gettimeofday(&m_time, 0);
  currentTime = m_time.tv_usec;

  // Calculate the difference in time since the last time we queried for the current time giving us the frame time.
  m_frameTime = currentTime - m_startTime;

  // Check if the timer has looped over and started at zero again.
  if(m_frameTime < 0)
  {
    m_frameTime = currentTime;
  }

  // Restart the timer.
  m_startTime = currentTime;

  return;
}


long TimerClass::GetTime()
{
  return m_frameTime;
}
