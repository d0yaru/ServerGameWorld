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
  
  // Create the entity list.
  m_EntityList = new EntityClass[MAX_ENTITIES];
  if(!m_EntityList)
  {
    return false;
  }

  // Setup some AI entities.
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

  // Initialize the first AI timer and trigger.
  m_aiTimerOne = 0;
  m_AITrigger01 = false;

  //
  m_networkMessage = 0;

  return true;
}


void ZoneClass::Shutdown()
{
  // Release the zone entity list.
  if(m_EntityList)
  {
    delete [] m_EntityList;
    m_EntityList = 0;
  }

  return;
}


void ZoneClass::Frame(long frameTime)
{
  // Perform AI processing for the frame.
  UpdateAI(frameTime);

  return;
}


void ZoneClass::AddNewUser(unsigned short id)
{
  int i;


  // Get the position in the list array to place the user.
  i=0;
  while((m_EntityList[i].IsActive() == true) && (i < MAX_ENTITIES))
  {
    i++;
  }

  // Add the user to the entity list.
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

  
  // Find the location of the user in the array.
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

  // Remove the user from the entity list as they have now logged off.
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


  // Check if the trigger for the first AI has been activated yet.
  if(m_AITrigger01 == false)
  {
    // Check if five seconds have passed yet.
    m_aiTimerOne += frameTime;
    if(m_aiTimerOne > 5000000)
    {
      m_aiTimerOne = 0;
      m_AITrigger01 = true;

      // Send message that AI is rotating to network clients.
      NetworkMessage(1, 0);
    }
  }

  // If AI triggered then rotate 180 degrees.
  if(m_AITrigger01 == true)
  {
    m_EntityList[0].GetRotation(rotX, rotY, rotZ);

    if(rotY < 180.0f)
    {
      // Rotate using the frame time to synchronize with network clients.
      rotY += ((float)frameTime / 1.0f) * 0.0001f;
    }
    else
    {
      rotY = 0.0f;
      m_AITrigger01 = false;

      // Send message that AI is no longer rotating to network clients.
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
