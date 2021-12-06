////////////////////////////////////////////////////////////////////////////////
// Filename: entityclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "entityclass.h"


EntityClass::EntityClass()
{
  m_active = false;
}


EntityClass::EntityClass(const EntityClass& other)
{
}


EntityClass::~EntityClass()
{
}


void EntityClass::SetActive(bool active)
{
  m_active = true;
  return;
}


void EntityClass::SetId(unsigned short id)
{
  m_id = id;
  return;
}


void EntityClass::SetType(char type)
{
  m_entityType = type;
  return;
}


void EntityClass::SetPosition(float x, float y, float z)
{
  m_positionX = x;
  m_positionY = y;
  m_positionZ = z;
  return;
}


void EntityClass::SetRotation(float x, float y, float z)
{
  m_rotationX = x;
  m_rotationY = y;
  m_rotationZ = z;
  return;
}


bool EntityClass::IsActive()
{
  return m_active;
}


void EntityClass::GetId(unsigned short& id)
{
  id = m_id;
  return;
}


void EntityClass::GetType(char& type)
{
  type = m_entityType;
  return;
}


void EntityClass::GetPosition(float& x, float& y, float& z)
{
  x = m_positionX;
  y = m_positionY;
  z = m_positionZ;
  return;
}


void EntityClass::GetRotation(float& x, float& y, float& z)
{
  x = m_rotationX;
  y = m_rotationY;
  z = m_rotationZ;
  return;
}
