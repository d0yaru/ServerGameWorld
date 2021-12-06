////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
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


  // Create the timer object.
  m_Timer = new TimerClass;
  if(!m_Timer)
  {
    return false;
  }

  // Initialize the timer object.
  m_Timer->Initialize();

  // Create the zone object.
  m_BlackForest = new ZoneClass;
  if(!m_BlackForest)
  {
    return false;
  }

  // Initialize the zone object.
  result = m_BlackForest->Initialize();
  if(!result)
  {
    cout << "Could not initialize the black forest zone object." << endl;
    return false;
  }

  // Create the network object.
  m_Network = new NetworkClass;
  if(!m_Network)
  {
    return false;
  }

  // Give the network object access to the zone object.  Needs to be done before network online.
  m_Network->SetZonePointer(m_BlackForest);

  // Initialize the network object and bring the server online.
  result = m_Network->Initialize();
  if(!result)
  {
    cout << "Could not initialize the network object." << endl;
    return false;
  }

  // Set the system to online.
  m_online = true;

  return true;
}


void SystemClass::Shutdown()
{
  // Shutdown and release the zone object.
  if(m_BlackForest)
  {
    m_BlackForest->Shutdown();
    delete m_BlackForest;
    m_BlackForest = 0;
  }

  // Shutdown and release the network object.
  if(m_Network)
  {
    m_Network->Shutdown();
    delete m_Network;
    m_Network = 0;
  }

  // Release the timer object.
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


  // Update the frame time.
  m_Timer->Frame();

  // Perform network frame processing.
  m_Network->Frame();

  // Perform zone frame processing.
  m_BlackForest->Frame(m_Timer->GetTime());

  // Check for messages from the zone for network clients after the processing.
  m_BlackForest->GetNetworkMessage(messageType, id);
  if(messageType != 0)
  {
    m_Network->AIMessageForClients(messageType, id);
  }

  return;
}


bool SystemClass::Online()
{
  return m_online;
}
