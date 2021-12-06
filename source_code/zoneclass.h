////////////////////////////////////////////////////////////////////////////////
// Filename: zoneclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _ZONECLASS_H_
#define _ZONECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <iostream>
using namespace std;


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "entityclass.h"


/////////////
// DEFINES //
/////////////
const int MAX_ENTITIES = 20;


////////////////////////////////////////////////////////////////////////////////
// Class name: ZoneClass
////////////////////////////////////////////////////////////////////////////////
class ZoneClass
{
public:
    ZoneClass();
    ZoneClass(const ZoneClass&);
    ~ZoneClass();

    bool Initialize();
    void Shutdown();
    void Frame(long);

    void AddNewUser(unsigned short);
    int GetEntityCount();
    void GetEntityData(int, unsigned short&, char&, float&, float&, float&, float&, float&, float&);
    void RemoveUser(unsigned short);
    void SetStateChange(unsigned short, char);
    void SetPosition(unsigned short, float, float, float, float, float, float);
    void GetNetworkMessage(int&, unsigned short&);

private:
    void UpdateAI(long);
    void NetworkMessage(int, unsigned short);

private:
    EntityClass* m_EntityList;
    int m_entityCount;
    long m_aiTimerOne;
    bool m_AITrigger01;
    int m_networkMessage;
    unsigned short m_networkId;
};

#endif
