////////////////////////////////////////////////////////////////////////////////
// Filename: networkclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _NETWORKCLASS_H_
#define _NETWORKCLASS_H_


/////////////
// GLOBALS //
/////////////
const unsigned short PORT_NUMBER = 7000;
const int MAX_MESSAGE_SIZE = 512;
const int MAX_QUEUE_SIZE = 200;
const int MAX_CLIENTS = 1000;


//////////////
// INCLUDES //
//////////////
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
using namespace std;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "networkmessages.h"
#include "zoneclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: NetworkClass
////////////////////////////////////////////////////////////////////////////////
class NetworkClass
{
private:
  struct QueueType
  {
    bool active;
    struct sockaddr_in address;
    int size;
    char message[MAX_MESSAGE_SIZE];
  };

  struct ClientType
  {
    bool online;
    unsigned short sessionId;
    struct sockaddr_in clientAddress;
  };

public:
    NetworkClass();
    NetworkClass(const NetworkClass&);
    ~NetworkClass();

    bool Initialize();
    void Shutdown();
    void Frame();

    bool Online();
    int GetServerSocket();
    void SetZonePointer(ZoneClass*);

    void AddMessageToQueue(char*, int, struct sockaddr_in);
    void AIMessageForClients(int, unsigned short);

private:
    bool InitializeServerSocket();
    void ProcessMessageQueue();

    void HandleConnectMessage(struct sockaddr_in);
    void HandlePingMessage(int);
    void HandleDisconnectMessage(int);
    void HandleChatMessage(int);
    void HandleEntityRequestMessage(int);
    void HandleStateChangeMessage(int);
    void HandlePositionMessage(int);

    void GetNextIdNumber(unsigned short&, unsigned short&);
    bool VerifyUser(unsigned short, unsigned short, int);

    void AIStartRotateMessage(unsigned short);
    void AIStopRotateMessage(unsigned short);

private:
    bool m_online;
    int m_socket;
    QueueType* m_networkMessageQueue;
    int m_nextQueueLocation, m_nextMessageForProcessing;
    ClientType* m_clientList;
    ZoneClass* m_ZonePtr;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
void* ServerListenFunction(void*);

#endif
