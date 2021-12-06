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


  // Seed the random number generator with the time.
  srand(time(NULL));

  // Initialize the network message queue.
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

  // Initialize the client list.
  m_clientList = new ClientType[MAX_CLIENTS];
  if(!m_clientList)
  {
    return false;
  }

  for(i=0; i<MAX_CLIENTS; i++)
  {
    m_clientList[i].online = false;
  }

  // Initialize and start the server socket to listen and process incoming connections.
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


  // Set the server to be offline.  Thread will also shut down when this is set to false.
  m_online = false;

  // Close the server socket.
  error = close(m_socket);
  if(error != 0)
  {
    cout << "Error: Could not close socket correctly." << endl;
  }

  // Release the client list.
  if(m_clientList)
  {
    delete [] m_clientList;
    m_clientList = 0;
  }

  // Release the network message queue.
  if(m_networkMessageQueue)
  {
    delete [] m_networkMessageQueue;
    m_networkMessageQueue = 0;
  }

  // Release the zone pointer.
  m_ZonePtr = 0;

  return;
}


void NetworkClass::Frame()
{
  // Process the messages in the queue.
  ProcessMessageQueue();

  return;
}


bool NetworkClass::InitializeServerSocket()
{  
  struct sockaddr_in serverAddress;
  int error;
  unsigned long setting;
  pthread_t serverThreadId;


  // Set the server to be online.
  m_online = true;

  // Create a UDP socket.
  m_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if(m_socket == -1)
  {
    cout << "Error: Could not create UDP socket." << endl;
    return false;
  }

  // Fill in the address information for binding the socket to and have the kernel set the IP address.
  memset((char*)&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(PORT_NUMBER);
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket to the address.
  error = bind(m_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  if(error == -1)
  {
    cout << "Error: Could not bind the socket." << endl;
    return false;
  }

  // Set the socket to non-blocking I/O.
  setting = 1;
  
  error = ioctl(m_socket, FIONBIO, &setting);
  if(error == -1)
  {
    cout << "Error: Could not set socket to non-blocking I/O." << endl;
    return false;
  }

  // Create a thread to listen for and accept incoming network messages from clients.
  error = pthread_create(&serverThreadId, NULL, ServerListenFunction, (void*)this);
  if(error != 0)
  {
    cout << "Error: Could not create thread." << endl;
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



  // Get a pointer to the calling object.
  networkClassPtr = (NetworkClass*)ptr;

  // Set the size of the address.
  clientLength = sizeof(clientAddress);

  while(networkClassPtr->Online())
  {
    // Check if there is a message from a client.
    bytesRead = recvfrom(networkClassPtr->GetServerSocket(), recvBuffer, 4096, 0, (struct sockaddr*)&clientAddress, &clientLength);

    // If there is a message then add it to the queue for processing.
    if(bytesRead > 0)
    {
      networkClassPtr->AddMessageToQueue(recvBuffer, bytesRead, clientAddress);
    }

  }
  
  return 0;
}


void NetworkClass::AddMessageToQueue(char* message, int messageSize, struct sockaddr_in clientAddress)
{
  char* ipAddress;


  // Check for buffer overflow.
  if(messageSize > MAX_MESSAGE_SIZE)
  {
    ipAddress = inet_ntoa(clientAddress.sin_addr);
    cout << "WARNING-0001: Possible buffer overflow attack from IP: " << ipAddress << endl;

    // Add timestamp to warnings.

  }

  // Otherwise add it to the circular message queue to be processed.
  else
  {
    m_networkMessageQueue[m_nextQueueLocation].address = clientAddress;
    m_networkMessageQueue[m_nextQueueLocation].size = messageSize;
    memcpy(m_networkMessageQueue[m_nextQueueLocation].message, message, messageSize);

    // Set it to be active last so that racing conditions in processing the queue do not exist.
    m_networkMessageQueue[m_nextQueueLocation].active = true;
    
    // Increment the queue position.
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


  // Loop through and process all the active messages in the queue.
  while(m_networkMessageQueue[m_nextMessageForProcessing].active == true)
  {
    // Coerce the message into a generic format to read the type of message.
    message = (MSG_GENERIC_DATA*)m_networkMessageQueue[m_nextMessageForProcessing].message;    

    // Process the message based on the message type.
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
	cout << "WARNING-0002: Received an unknown message type from IP: " << ipAddress << endl;
	break;
      }
    }

    // Set the message as processed.
    m_networkMessageQueue[m_nextMessageForProcessing].active = false;

    // Increment the queue position.
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


  // Get the next free ID number and session ID and then assign it to this client.
  GetNextIdNumber(newId, sessionId);
  if(newId != -1)
  {
    ipAddress = inet_ntoa(clientAddress.sin_addr);
    cout << "Received connect message from " << ipAddress << ".  Assigning id number " << newId << " with session id " << sessionId << "." << endl;

    // Set the user to online and store the client address.
    m_clientList[newId].online = true;
    m_clientList[newId].sessionId = sessionId;
    m_clientList[newId].clientAddress = clientAddress;

    // Create a new id message for the user.
    message.type = MSG_NEWID;
    message.idNumber = newId;
    message.sessionId = sessionId;

    // Send the user notification of their ids.
    bytesSent = sendto(m_socket, (char*)&message, sizeof(MSG_NEWID_DATA), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
    if(bytesSent != sizeof(MSG_NEWID_DATA))
    {
      cout << "WARNING-0003: Error sending new ID message to client with IP: " << ipAddress << endl;
    }
    
    // Add the new user to the zone.
    m_ZonePtr->AddNewUser(newId);

    // Notify all other network clients that a new user has logged in.
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


  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_PING_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received ping message from " << ipAddress << ".  Sending reply ping message." << endl;

  // Send a ping reply message back to the client.
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


  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_DISCONNECT_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received disconnect message from " << ipAddress << ".  Disconnecting them from server." << endl;

  // Disconnect the user.
  m_clientList[clientId].online = false;

  // Remove the entity from the zone.
  m_ZonePtr->RemoveUser(clientId);

  // Notify the other network clients that this user went offline.
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


  // Maybe place timer on text messages from client here to prevent attacks.

  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_CHAT_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received chat message from " << ipAddress << ".  Contents: " << msg->text << " - Sending to all clients." << endl;

  // Create the chat message.
  message.type = MSG_CHAT;
  message.idNumber = clientId;
  strcpy(message.text, msg->text);

  // Send the chat message to all other clients connected.
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


  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_SIMPLE_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received entity list request message from " << ipAddress << ".  Sending list of all users and non-users to them." << endl;

  // Check this message hasn't been requested too much from them.

  // Check that the zone pointer is valid.
  if(!m_ZonePtr)
  {
    cout << "WARNING-0020: Zone pointer invalid." << endl;    
    return;
  }

  // Get the number of entities.
  numEntities = m_ZonePtr->GetEntityCount();

  // Send the client the list of entities.
  for(i=0; i<numEntities; i++)
  {
    m_ZonePtr->GetEntityData(i, id, type, posX, posY, posZ, rotX, rotY, rotZ);

    if((type == ENTITY_TYPE_USER) && (id == clientId))
    {
      // Don't send the entity if it is themselves.
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


  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_STATE_CHANGE_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  // Get the state change from the message.
  state = msg->state;

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received state change message from " << ipAddress << " : " << (int)state << endl;

  // Update the zone with the state change.
  if(m_ZonePtr)
  {
    m_ZonePtr->SetStateChange(clientId, state);
  }
  else
  {
    cout << "WARNING-0020: Zone pointer invalid." << endl;    
  }

  // Create the state change message.
  message.type = MSG_STATE_CHANGE;
  message.idNumber = clientId;
  message.state = state;

  // Notify all the other users of this user's state change.
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


  // Coerce the message into a generic format to read the type of message.
  msg = (MSG_POSITION_DATA*)m_networkMessageQueue[queuePosition].message;

  // Get the id numbers.
  clientId = msg->idNumber;
  sessionId = msg->sessionId;

  // Verify that the client is legitimate.
  result = VerifyUser(clientId, sessionId, queuePosition);
  if(!result)
  {
    return;
  }

  // Get the position from the message.
  positionX = msg->positionX;
  positionY = msg->positionY;
  positionZ = msg->positionZ;
  rotationX = msg->rotationX;
  rotationY = msg->rotationY;
  rotationZ = msg->rotationZ;

  ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
  cout << "Received position message from: " << ipAddress << endl;

  // Validate that the position is not a hack.

  // Update the zone with the position.
  if(m_ZonePtr)
  {
    m_ZonePtr->SetPosition(clientId, positionX, positionY, positionZ, rotationX, rotationY, rotationZ);
  }
  else
  {
    cout << "WARNING-0020: Zone pointer invalid." << endl;    
  }

  // Create the position message.
  message.type = MSG_POSITION;
  message.idNumber = clientId;
  message.positionX = positionX;
  message.positionY = positionY;
  message.positionZ = positionZ;
  message.rotationX = rotationX;
  message.rotationY = rotationY;
  message.rotationZ = rotationZ;

  // Notify all the other users of this user's position update.
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


  // Initialize the loop variables.
  newId = -1;
  done = false;
  i=0;

  // Loop through all the clients and assign an ID that is not online.
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

    // Check if we have exceeded the maximum number of clients.
    if(i == MAX_CLIENTS)
    {
      done = true;
    }
  }

  // Generate a random session ID for the client.
  sessionId = rand() % 65000;

  return;
}


bool NetworkClass::VerifyUser(unsigned short clientId, unsigned short sessionId, int queuePosition)
{
  char *ipAddress1, *ipAddress2;


  // Check if the client should still be online.
  if(m_clientList[clientId].online == false)
  {
    cout << "WARNING-0005: Network nessage from client that should be disconnected." << endl;
    return false;
  }

  // Check that the session ID is correct for this client.
  if(m_clientList[clientId].sessionId != sessionId)
  {
    cout << "WARNING-0006: False session id for client." << endl;
    return false;
  }

  // Check the IP address that sent this message actually corresponds to the original IP address of the client.
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


  // Create the start rotate state change message for the AI entity.
  message.type = MSG_AI_ROTATE;
  message.idNumber = idNumber;
  message.rotate = true;

  // Notify all the network clients of the AI state change.
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


  // Create the stop rotate state change message for the AI entity.
  message.type = MSG_AI_ROTATE;
  message.idNumber = idNumber;
  message.rotate = false;

  // Notify all the network clients of the AI state change.
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
