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
	// Узнать текущее время.
	gettimeofday(&m_time, 0);

	// Извлеките микросекунды из структуры.
	m_startTime = m_time.tv_usec;

	return;
}


void TimerClass::Frame()
{
	long currentTime;


	// Запрашивать текущее время.
	gettimeofday(&m_time, 0);
	currentTime = m_time.tv_usec;

	// Вычислите разницу во времени с момента последнего запроса текущего времени, что даст нам время кадра.
	m_frameTime = currentTime - m_startTime;

	// Проверьте, не зациклился ли таймер и не начался ли он снова с нуля.
	if(m_frameTime < 0)
	{
		m_frameTime = currentTime;
	}

	// Перезапустить таймер.
	m_startTime = currentTime;

	return;
}


long TimerClass::GetTime()
{
	return m_frameTime;
}
