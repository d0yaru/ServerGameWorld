////////////////////////////////////////////////////////////////////////////////
// Filename: zoneclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "zoneclass.h"


ZoneClass::ZoneClass()
{
	m_EntityList = 0;
}


ZoneClass::ZoneClass(const ZoneClass& other)
{
}


ZoneClass::~ZoneClass()
{
}


bool ZoneClass::Initialize()
{
	
	// Создайте список сущностей.
	m_EntityList = new EntityClass[MAX_ENTITIES];
	if(!m_EntityList)
	{
		return false;
	}

	// Настройка некоторых объектов искусственного интеллекта.
	m_EntityList[0].SetActive(true);
	m_EntityList[0].SetId(0);
	m_EntityList[0].SetType(ENTITY_TYPE_AI);
	m_EntityList[0].SetPosition(40.0f, 1.0f, 40.0f);
	m_EntityList[0].SetRotation(0.0f, 45.0f, 0.0f);

	m_EntityList[1].SetActive(true);
	m_EntityList[1].SetId(1);
	m_EntityList[1].SetType(ENTITY_TYPE_AI);
	m_EntityList[1].SetPosition(20.0f, 1.0f, 30.0f);
	m_EntityList[1].SetRotation(0.0f, 0.0f, 0.0f);

	m_entityCount = 2;

	// Инициализируйте первый таймер искусственного интеллекта и запустите.
	m_aiTimerOne = 0;
	m_AITrigger01 = false;

	//
	m_networkMessage = 0;

	return true;
}


void ZoneClass::Shutdown()
{
	// Отпустите список объектов зоны.
	if(m_EntityList)
	{
		delete [] m_EntityList;
		m_EntityList = 0;
	}

	return;
}


void ZoneClass::Frame(long frameTime)
{
	// Выполните обработку кадра искусственным интеллектом.
	UpdateAI(frameTime);

	return;
}


void ZoneClass::AddNewUser(unsigned short id)
{
	int i;


	// Получить позицию в массиве списка для размещения пользователя.
	i=0;
	while((m_EntityList[i].IsActive() == true) && (i < MAX_ENTITIES))
	{
		i++;
	}

	// Добавьте пользователя в список сущностей.
	if(i != MAX_ENTITIES)
	{
		m_EntityList[i].SetActive(true);
		m_EntityList[i].SetId(id);
		m_EntityList[i].SetType(ENTITY_TYPE_USER);
		m_EntityList[i].SetPosition(20.0f, 1.0f, 20.0f);
		m_EntityList[i].SetRotation(0.0f, 0.0f, 0.0f);
		m_entityCount++;
	}
	else
	{
		cout << "WARNING-0030: Max entities reached."  << endl;
	}

	return;
}


int ZoneClass::GetEntityCount()
{
	return m_entityCount;
}


void ZoneClass::GetEntityData(int index, unsigned short& id, char& type, float& posX, float& posY, float& posZ, float& rotX, float& rotY, float& rotZ)
{
	m_EntityList[index].GetId(id);
	m_EntityList[index].GetType(type);
	m_EntityList[index].GetPosition(posX, posY, posZ);
	m_EntityList[index].GetRotation(rotX, rotY, rotZ);

	return;
}


void ZoneClass::RemoveUser(unsigned short id)
{
	int i;
	bool found;
	unsigned short entityId;

	
	// Найдите местоположение пользователя в массиве.
	i=0;
	found = false;
	while(!found)
	{
		m_EntityList[i].GetId(entityId);

		if(entityId == id)
		{
			found = true;
		}
		else
		{
			i++;
		}
	}

	// Удалите пользователя из списка сущностей, так как он уже вышел из системы.
	m_EntityList[i].SetActive(false);
	m_entityCount--;

	return;
}


void ZoneClass::SetStateChange(unsigned short clientId, char state)
{

	return;
}


void ZoneClass::SetPosition(unsigned short clientId, float positionX, float positionY, float positionZ, float rotationX, float rotationY, float rotationZ)
{

	return;
}


void ZoneClass::UpdateAI(long frameTime)
{
	float rotX, rotY, rotZ;


	// Проверьте, был ли активирован триггер для первого искусственного интеллекта.
	if(m_AITrigger01 == false)
	{
		// Проверьте, не прошло ли еще пяти секунд.
		m_aiTimerOne += frameTime;
		if(m_aiTimerOne > 5000000)
		{
			m_aiTimerOne = 0;
			m_AITrigger01 = true;

			// Отправить сообщение о том, что ИИ переключается на сетевых клиентов.
			NetworkMessage(1, 0);
		}
	}

	// Если сработает искусственный интеллект, то повернитесь на 180 градусов.
	if(m_AITrigger01 == true)
	{
		m_EntityList[0].GetRotation(rotX, rotY, rotZ);

		if(rotY < 180.0f)
		{
			// Поворот с использованием времени кадра для синхронизации с сетевыми клиентами.
			rotY += ((float)frameTime / 1.0f) * 0.0001f;
		}
		else
		{
			rotY = 0.0f;
			m_AITrigger01 = false;

			// Отправить сообщение о том, что ИИ больше не переключается на сетевых клиентов.
			NetworkMessage(2, 0);
		}
		m_EntityList[0].SetRotation(rotX, rotY, rotZ);
	}

	return;
}


void ZoneClass::NetworkMessage(int messageType, unsigned short idNumber)
{
	m_networkMessage = messageType;
	m_networkId = idNumber;
	return;
}


void ZoneClass::GetNetworkMessage(int& type, unsigned short& id)
{
	type = m_networkMessage;
	id = m_networkId;
	m_networkMessage = 0;
	return;
}
