////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include <chrono>
#include <thread>

#include "systemclass.h"


SystemClass::SystemClass()
{
	m_online = false;
	m_Timer = 0;
	m_Network = 0;
	m_BlackForest = 0;
}


SystemClass::SystemClass(const SystemClass& other)
{
}


SystemClass::~SystemClass()
{
}


bool SystemClass::Initialize()
{
	bool result;


	// Создайте объект таймера.
	m_Timer = new TimerClass;
	if(!m_Timer)
	{
		return false;
	}

	// Инициализировать объект таймера.
	m_Timer->Initialize();

	// Создайте объект зоны.
	m_BlackForest = new ZoneClass;
	if(!m_BlackForest)
	{
		return false;
	}

	// Инициализировать объект зоны.
	result = m_BlackForest->Initialize();
	if(!result)
	{
		cout << "Could not initialize the black forest zone object." << endl;// Не удалось инициализировать объект зоны черного леса
		return false;
	}

	// Создайте сетевой объект.
	m_Network = new NetworkClass;
	if(!m_Network)
	{
		return false;
	}

	// Предоставьте сетевому объекту доступ к объекту зоны. Это необходимо сделать до того, как сеть подключится к сети.
	m_Network->SetZonePointer(m_BlackForest);

	// Инициализируйте сетевой объект и переведите сервер в оперативный режим.
	result = m_Network->Initialize();
	if(!result)
	{
		cout << "Could not initialize the network object." << endl;// Не удалось инициализировать сетевой объект
		return false;
	}

	// Установите систему в режим онлайн.
	m_online = true;

	return true;
}


void SystemClass::Shutdown()
{
	// Выключите и освободите объект зоны.
	if(m_BlackForest)
	{
		m_BlackForest->Shutdown();
		delete m_BlackForest;
		m_BlackForest = 0;
	}

	// Выключите и освободите сетевой объект.
	if(m_Network)
	{
		m_Network->Shutdown();
		delete m_Network;
		m_Network = 0;
	}

	// Отпустите объект таймера.
	if(m_Timer)
	{
		delete m_Timer;
		m_Timer = 0;
	}

	return;
}


void SystemClass::Frame()
{
	int messageType;
	unsigned short id;


	// Обновите время кадра.
	m_Timer->Frame();

	// Выполнять обработку сетевых кадров.
	m_Network->Frame();

	// Выполнить обработку зонного кадра.
	m_BlackForest->Frame(m_Timer->GetTime());

	// Проверьте наличие сообщений из зоны для сетевых клиентов после обработки.
	m_BlackForest->GetNetworkMessage(messageType, id);
	if(messageType != 0)
	{
		m_Network->AIMessageForClients(messageType, id);
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	return;
}


bool SystemClass::Online()
{
	return m_online;
}
