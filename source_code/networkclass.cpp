////////////////////////////////////////////////////////////////////////////////
// Filename: networkclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "networkclass.h"


NetworkClass::NetworkClass()
{
	m_networkMessageQueue = 0;
	m_clientList = 0;
	m_ZonePtr = 0;
}


NetworkClass::NetworkClass(const NetworkClass& other)
{
}


NetworkClass::~NetworkClass()
{
}


bool NetworkClass::Initialize()
{
	bool result;
	int i;


	// Заполняйте генератор случайных чисел временем.
	srand(time(NULL));

	// Инициализировать очередь сетевых сообщений.
	m_networkMessageQueue = new QueueType[MAX_QUEUE_SIZE];
	if(!m_networkMessageQueue)
	{
		return false;
	}

	m_nextQueueLocation = 0;
	m_nextMessageForProcessing = 0;

	for(i=0; i<MAX_QUEUE_SIZE; i++)
	{
		m_networkMessageQueue[i].active = false;
	}

	// Инициализировать список клиентов.
	m_clientList = new ClientType[MAX_CLIENTS];
	if(!m_clientList)
	{
		return false;
	}

	for(i=0; i<MAX_CLIENTS; i++)
	{
		m_clientList[i].online = false;
	}

	// Инициализируйте и запустите серверный сокет для прослушивания и обработки входящих подключений.
	result = InitializeServerSocket();
	if(!result)
	{
		return false;
	}

	return true;
}


void NetworkClass::Shutdown()
{
	int error;


	// Установите сервер в автономный режим. Поток также завершится, если для этого значения установлено значение false.
	m_online = false;

	// Закройте серверный сокет.
	error = close(m_socket);
	if(error != 0)
	{
		cout << "Error: Could not close socket correctly." << endl;
	}

	// Опубликуйте список клиентов.
	if(m_clientList)
	{
		delete [] m_clientList;
		m_clientList = 0;
	}

	// Освободите очередь сетевых сообщений.
	if(m_networkMessageQueue)
	{
		delete [] m_networkMessageQueue;
		m_networkMessageQueue = 0;
	}

	// Отпустите указатель зоны.
	m_ZonePtr = 0;

	return;
}


void NetworkClass::Frame()
{
	// Обрабатывать сообщения в очереди.
	ProcessMessageQueue();

	return;
}


bool NetworkClass::InitializeServerSocket()
{  
	struct sockaddr_in serverAddress;
	int error;
	unsigned long setting;
	pthread_t serverThreadId;


	// Установите сервер в режиме онлайн.
	m_online = true;

	// Создайте сокет UDP.
	m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_socket == -1)
	{
		cout << "Error: Could not create UDP socket." << endl;// Ошибка: Не удалось создать сокет UDP
		return false;
	}

	// Заполните информацию об адресе для привязки сокета и попросите ядро установить IP-адрес.
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUMBER);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	// Привязать сокет к адресу.
	error = bind(m_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if(error == -1)
	{
		cout << "Error: Could not bind the socket." << endl;// Ошибка: Не удалось привязать сокет
		return false;
	}

	// Установите для сокета режим неблокирующего ввода-вывода.
	setting = 1;
	
	error = ioctl(m_socket, FIONBIO, &setting);
	if(error == -1)
	{
		cout << "Error: Could not set socket to non-blocking I/O." << endl;// Ошибка: Не удалось установить сокет на неблокирующий ввод-вывод
		return false;
	}

	// Создайте поток для прослушивания и приема входящих сетевых сообщений от клиентов.
	error = pthread_create(&serverThreadId, NULL, ServerListenFunction, (void*)this);
	if(error != 0)
	{
		cout << "Error: Could not create thread." << endl;// Ошибка: Не удалось создать поток
		return false;
	}

	return true;
}


bool NetworkClass::Online()
{
	return m_online;
}


int NetworkClass::GetServerSocket()
{
	return m_socket;
}


void NetworkClass::SetZonePointer(ZoneClass* ptr)
{
	m_ZonePtr = ptr;
	return;
}


void* ServerListenFunction(void* ptr)
{
	NetworkClass* networkClassPtr;
	unsigned int clientLength;
	int bytesRead;
	char recvBuffer[4096];
	struct sockaddr_in clientAddress;



	// Получить указатель на вызывающий объект.
	networkClassPtr = (NetworkClass*)ptr;

	// Установите размер адреса.
	clientLength = sizeof(clientAddress);

	while(networkClassPtr->Online())
	{
		// Проверьте, есть ли сообщение от клиента.
		bytesRead = recvfrom(networkClassPtr->GetServerSocket(), recvBuffer, 4096, 0, (struct sockaddr*)&clientAddress, &clientLength);

		// Если есть сообщение, добавьте его в очередь на обработку.
		if(bytesRead > 0)
		{
			networkClassPtr->AddMessageToQueue(recvBuffer, bytesRead, clientAddress);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
	return 0;
}


void NetworkClass::AddMessageToQueue(char* message, int messageSize, struct sockaddr_in clientAddress)
{
	char* ipAddress;


	// Проверка на переполнение буфера.
	if(messageSize > MAX_MESSAGE_SIZE)
	{
		ipAddress = inet_ntoa(clientAddress.sin_addr);
		cout << "WARNING-0001: Possible buffer overflow attack from IP: " << ipAddress << endl;// Возможная атака переполнения буфера с IP

		// Добавление метки времени в предупреждения.

	}

	// В противном случае добавьте его в очередь циклических сообщений для обработки.
	else
	{
		m_networkMessageQueue[m_nextQueueLocation].address = clientAddress;
		m_networkMessageQueue[m_nextQueueLocation].size = messageSize;
		memcpy(m_networkMessageQueue[m_nextQueueLocation].message, message, messageSize);

		// Установите его активным последним, чтобы не существовало условий гонки при обработке очереди.
		m_networkMessageQueue[m_nextQueueLocation].active = true;
		
		// Увеличьте позицию в очереди.
		m_nextQueueLocation++;
		if(m_nextQueueLocation == MAX_QUEUE_SIZE)
		{
			m_nextQueueLocation = 0;
		}
	}

	return;
}


void NetworkClass::ProcessMessageQueue()
{
	MSG_GENERIC_DATA* message;
	char* ipAddress;


	// Перебирайте и обрабатывайте все активные сообщения в очереди.
	while(m_networkMessageQueue[m_nextMessageForProcessing].active == true)
	{
		// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
		message = (MSG_GENERIC_DATA*)m_networkMessageQueue[m_nextMessageForProcessing].message;    

		// Обработать сообщение в зависимости от типа сообщения.
		switch(message->type)
		{
			case MSG_CONNECT:
			{
				HandleConnectMessage(m_networkMessageQueue[m_nextMessageForProcessing].address);
				break;
			}
			case MSG_PING:
			{
				HandlePingMessage(m_nextMessageForProcessing);
				break;
			}
			case MSG_DISCONNECT:
			{
				HandleDisconnectMessage(m_nextMessageForProcessing);
				break;
			}
			case MSG_CHAT:
			{
				HandleChatMessage(m_nextMessageForProcessing);
				break;
			}
			case MSG_ENTITY_REQUEST:
			{
				HandleEntityRequestMessage(m_nextMessageForProcessing);
				break;
			}
			case MSG_STATE_CHANGE:
			{
				HandleStateChangeMessage(m_nextMessageForProcessing);
				break;
			}
			case MSG_POSITION:
			{
				HandlePositionMessage(m_nextMessageForProcessing);
				break;
			}
			default:
			{
				ipAddress = inet_ntoa(m_networkMessageQueue[m_nextMessageForProcessing].address.sin_addr);
				cout << "WARNING-0002: Received an unknown message type from IP: " << ipAddress << endl;// Получено сообщение неизвестного типа с IP
				break;
			}
		}

		// Установите сообщение как обработанное.
		m_networkMessageQueue[m_nextMessageForProcessing].active = false;

		// Увеличьте позицию в очереди.
		m_nextMessageForProcessing++;
		if(m_nextMessageForProcessing == MAX_QUEUE_SIZE)
		{
			m_nextMessageForProcessing = 0;
		}
	}

	return;
}


void NetworkClass::HandleConnectMessage(struct sockaddr_in clientAddress)
{
	unsigned short newId;
	unsigned short sessionId;
	char* ipAddress;
	MSG_NEWID_DATA message;
	int bytesSent, i;
	MSG_ENTITY_INFO_DATA message2;


	// Получите следующий бесплатный идентификационный номер и идентификатор сеанса, а затем назначьте его этому клиенту.
	GetNextIdNumber(newId, sessionId);
	if(newId != -1)
	{
		ipAddress = inet_ntoa(clientAddress.sin_addr);
		cout << "Received connect message from " << ipAddress << ".  Assigning id number " << newId << " with session id " << sessionId << "." << endl;

		// Установите пользователя в режим онлайн и сохраните адрес клиента.
		m_clientList[newId].online = true;
		m_clientList[newId].sessionId = sessionId;
		m_clientList[newId].clientAddress = clientAddress;

		// Создайте новое идентификационное сообщение для пользователя.
		message.type = MSG_NEWID;
		message.idNumber = newId;
		message.sessionId = sessionId;

		// Отправлять пользователю уведомления о своих детях.
		bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_NEWID_DATA), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
		if(bytesSent != sizeof(MSG_NEWID_DATA))
		{
			cout << "WARNING-0003: Error sending new ID message to client with IP: " << ipAddress << endl;
		}
		
		// Добавьте нового пользователя в зону.
		m_ZonePtr->AddNewUser(newId);

		// Уведомлять всех других сетевых клиентов о том, что новый пользователь вошел в систему.
		message2.type = MSG_NEW_USER_LOGIN;
		message2.entityId = newId;
		message2.entityType = ENTITY_TYPE_USER;
		message2.positionX = 20.0f;
		message2.positionY = 1.0f;
		message2.positionZ = 20.0f;
		message2.rotationX = 0.0f;
		message2.rotationY = 0.0f;
		message2.rotationZ = 0.0f;

		for(i=0; i<MAX_CLIENTS; i++)
		{
			if(m_clientList[i].online)
			{
	if(i != newId)
	{
		bytesSent = sendto(m_socket, (char*)&message2, sizeof(MSG_ENTITY_INFO_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
		if(bytesSent != sizeof(MSG_ENTITY_INFO_DATA))
		{
			cout << "WARNING-0040: Error sending new user login message to client." << endl;
		}
	}
			}
		}
	}
	else
	{
		cout << "WARNING-0004: Max clients connected reached!" << endl;
	}

	return;
}


void NetworkClass::HandlePingMessage(int queuePosition)
{
	MSG_PING_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char* ipAddress;
	MSG_GENERIC_DATA message;
	int bytesSent;


	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_PING_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	//d0 cout << "Received ping message from " << ipAddress << ".  Sending reply ping message." << endl;

	// Отправьте ответное сообщение ping обратно клиенту.
	message.type = MSG_PING;

	bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_GENERIC_DATA), 0, (struct sockaddr*)&m_clientList[clientId].clientAddress, sizeof(m_clientList[clientId].clientAddress));
	if(bytesSent != sizeof(MSG_GENERIC_DATA))
	{
		cout << "WARNING-0008: Could not send ping message." << endl;
	}
	
	return;
}


void NetworkClass::HandleDisconnectMessage(int queuePosition)
{
	MSG_DISCONNECT_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char* ipAddress;
	MSG_USER_DISCONNECT_DATA message;
	int i, bytesSent;


	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_DISCONNECT_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "Received disconnect message from " << ipAddress << ".  Disconnecting them from server." << endl;

	// Отключите пользователя.
	m_clientList[clientId].online = false;

	// Удалить объект из зоны.
	m_ZonePtr->RemoveUser(clientId);

	// Уведомлять других сетевых клиентов о том, что этот пользователь перешел в автономный режим.
	message.type = MSG_USER_DISCONNECT;
	message.idNumber = clientId;

	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_USER_DISCONNECT_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
			if(bytesSent != sizeof(MSG_USER_DISCONNECT_DATA))
			{
	cout << "WARNING-0050: Error sending user disconnect message to client." << endl;
			}
		}
	}
	
	return;
}


void NetworkClass::HandleChatMessage(int queuePosition)
{
	MSG_CHAT_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char* ipAddress;
	MSG_CHAT_DATA message;
	int i, bytesSent;


	// Возможно, установите таймер для текстовых сообщений от клиента здесь, чтобы предотвратить атаки.

	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_CHAT_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "Received chat message from " << ipAddress << ".  Contents: " << msg->text << " - Sending to all clients." << endl;

	// Создайте сообщение в чате.
	message.type = MSG_CHAT;
	message.idNumber = clientId;
	strcpy(message.text, msg->text);

	// Отправьте сообщение чата всем другим подключенным клиентам.
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			if(i != clientId)
			{
	bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_CHAT_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
	if(bytesSent != sizeof(MSG_CHAT_DATA))
	{
		cout << "WARNING-0009: Could not relay chat message." << endl;
	}
			}
		}
	}

	return;
}


void NetworkClass::HandleEntityRequestMessage(int queuePosition)
{
	MSG_SIMPLE_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char* ipAddress;
	int numEntities, i, bytesSent;
	unsigned short id;
	char type;
	float posX, posY, posZ, rotX, rotY, rotZ;
	MSG_ENTITY_INFO_DATA message;


	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_SIMPLE_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "Received entity list request message from " << ipAddress << ".  Sending list of all users and non-users to them." << endl;

	// Проверьте, не слишком ли много от них требовали этого сообщения.

	// Убедитесь, что указатель зоны действителен.
	if(!m_ZonePtr)
	{
		cout << "WARNING-0020: Zone pointer invalid." << endl;    
		return;
	}

	// Получить количество сущностей.
	numEntities = m_ZonePtr->GetEntityCount();

	// Отправить клиенту список юридических лиц.
	for(i=0; i<numEntities; i++)
	{
		m_ZonePtr->GetEntityData(i, id, type, posX, posY, posZ, rotX, rotY, rotZ);

		if((type == ENTITY_TYPE_USER) && (id == clientId))
		{
			// Не отправляйте объект, если это они сами.
		}
		else
		{
			message.type = MSG_ENTITY_INFO;
			message.entityId = id;
			message.entityType = type;
			message.positionX = posX;
			message.positionY = posY;
			message.positionZ = posZ;
			message.rotationX = rotX;
			message.rotationY = rotY;
			message.rotationZ = rotZ;

			bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_ENTITY_INFO_DATA), 0, (struct sockaddr*)&m_clientList[clientId].clientAddress, sizeof(m_clientList[clientId].clientAddress));
			if(bytesSent != sizeof(MSG_ENTITY_INFO_DATA))
			{
	cout << "WARNING-0021: Could not entity info message." << endl;
			}
		}
	}

	return;
}


void NetworkClass::HandleStateChangeMessage(int queuePosition)
{
	MSG_STATE_CHANGE_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char state;
	char* ipAddress;
	MSG_STATE_CHANGE_DATA message;
	int i, bytesSent;


	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_STATE_CHANGE_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	// Получите изменение состояния из сообщения.
	state = msg->state;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	//d0 cout << "Received state change message from " << ipAddress << " : " << (int)state << endl;

	// Обновите зону с изменением состояния.
	if(m_ZonePtr)
	{
		m_ZonePtr->SetStateChange(clientId, state);
	}
	else
	{
		cout << "WARNING-0020: Zone pointer invalid." << endl;    
	}

	// Создайте сообщение об изменении состояния.
	message.type = MSG_STATE_CHANGE;
	message.idNumber = clientId;
	message.state = state;

	// Уведомлять всех остальных пользователей об изменении состояния этого пользователя.
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			if(i != clientId)
			{
	bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_STATE_CHANGE_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
	if(bytesSent != sizeof(MSG_STATE_CHANGE_DATA))
	{
		cout << "WARNING-0050: Could not send state change message." << endl;
	}
			}
		}
	}

	return;
}


void NetworkClass::HandlePositionMessage(int queuePosition)
{
	MSG_POSITION_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	float positionX, positionY, positionZ, rotationX, rotationY, rotationZ;
	char* ipAddress;
	MSG_POSITION_DATA message;
	int i, bytesSent;


	// Принудительно преобразуйте сообщение в общий формат для чтения типа сообщения.
	msg = (MSG_POSITION_DATA*)m_networkMessageQueue[queuePosition].message;

	// Получите идентификационные номера.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Убедитесь, что клиент является законным.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	//Получите позицию из сообщения.
	positionX = msg->positionX;
	positionY = msg->positionY;
	positionZ = msg->positionZ;
	rotationX = msg->rotationX;
	rotationY = msg->rotationY;
	rotationZ = msg->rotationZ;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	//d0 cout << "Received position message from: " << ipAddress << endl;

	// Подтвердите, что позиция не является взломом.

	// Обновите зону с указанием местоположения.
	if(m_ZonePtr)
	{
		m_ZonePtr->SetPosition(clientId, positionX, positionY, positionZ, rotationX, rotationY, rotationZ);
	}
	else
	{
		cout << "WARNING-0020: Zone pointer invalid." << endl;    
	}

	// Создайте сообщение о позиции.
	message.type = MSG_POSITION;
	message.idNumber = clientId;
	message.positionX = positionX;
	message.positionY = positionY;
	message.positionZ = positionZ;
	message.rotationX = rotationX;
	message.rotationY = rotationY;
	message.rotationZ = rotationZ;

	// Уведомлять всех других пользователей об обновлении позиции этого пользователя.
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			if(i != clientId)
			{
	bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_POSITION_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
	if(bytesSent != sizeof(MSG_POSITION_DATA))
	{
		cout << "WARNING-0051: Could not send position message." << endl;
	}
			}
		}
	}

	return;
}


void NetworkClass::GetNextIdNumber(unsigned short& newId, unsigned short& sessionId)
{
	bool done;
	int i;


	// Инициализируйте переменные цикла.
	newId = -1;
	done = false;
	i=0;

	// Пройдите по всем клиентам и назначьте идентификатор, который не подключен к Сети.
	while(!done)
	{
		if(m_clientList[i].online == false)
		{
			newId = i;
			done = true;
		}
		else
		{
			i++;
		}

		// Проверьте, не превысили ли мы максимальное количество клиентов.
		if(i == MAX_CLIENTS)
		{
			done = true;
		}
	}

	// Создайте случайный идентификатор сеанса для клиента.
	sessionId = rand() % 65000;

	return;
}


bool NetworkClass::VerifyUser(unsigned short clientId, unsigned short sessionId, int queuePosition)
{
	char *ipAddress1, *ipAddress2;


	// Проверьте, должен ли клиент все еще быть онлайн.
	if(m_clientList[clientId].online == false)
	{
		cout << "WARNING-0005: Network nessage from client that should be disconnected." << endl;
		return false;
	}

	// Убедитесь, что идентификатор сеанса указан правильно для этого клиента.
	if(m_clientList[clientId].sessionId != sessionId)
	{
		cout << "WARNING-0006: False session id for client." << endl;
		return false;
	}

	// Проверьте, что IP-адрес, отправивший это сообщение, на самом деле соответствует исходному IP-адресу клиента.
	ipAddress1 = inet_ntoa(m_clientList[clientId].clientAddress.sin_addr);
	ipAddress2 = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);

	if(strcmp(ipAddress1, ipAddress2) != 0)
	{
		cout << "WARNING-0007: Incorrect IP address for client." << endl;
		return false;
	}

	return true;
}


void NetworkClass::AIMessageForClients(int messageType, unsigned short idNumber)
{
	switch(messageType)
	{
		case 1:
		{
			AIStartRotateMessage(idNumber);
			break;
		}
		case 2:
		{
			AIStopRotateMessage(idNumber);
			break;
		}
		default:
		{
			break;
		}
	}

	return;
}


void NetworkClass::AIStartRotateMessage(unsigned short idNumber)
{
	MSG_AI_ROTATE_DATA message;
	int i, bytesSent;


	// Создайте сообщение об изменении состояния начала поворота для объекта искусственного интеллекта.
	message.type = MSG_AI_ROTATE;
	message.idNumber = idNumber;
	message.rotate = true;

	// Уведомлять всех сетевых клиентов об изменении состояния искусственного интеллекта.
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_AI_ROTATE_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
			if(bytesSent != sizeof(MSG_AI_ROTATE_DATA))
			{
	cout << "WARNING-0080: Could not send AI rotate begin message." << endl;
			}
		}
	}

	return;
}


void NetworkClass::AIStopRotateMessage(unsigned short idNumber)
{
	MSG_AI_ROTATE_DATA message;
	int i, bytesSent;


	// Создайте сообщение об изменении состояния остановки поворота для объекта искусственного интеллекта.
	message.type = MSG_AI_ROTATE;
	message.idNumber = idNumber;
	message.rotate = false;

	// Уведомлять всех сетевых клиентов об изменении состояния искусственного интеллекта.
	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_AI_ROTATE_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
			if(bytesSent != sizeof(MSG_AI_ROTATE_DATA))
			{
	cout << "WARNING-0081: Could not send AI rotate end message." << endl;
			}
		}
	}

	return;
}
