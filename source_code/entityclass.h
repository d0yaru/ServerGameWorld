////////////////////////////////////////////////////////////////////////////////
// Filename: entityclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _ENTITYCLASS_H_
#define _ENTITYCLASS_H_


/////////////
// DEFINES //
/////////////
const char ENTITY_TYPE_USER = 0;
const char ENTITY_TYPE_AI = 1;


////////////////////////////////////////////////////////////////////////////////
// Class name: EntityClass
////////////////////////////////////////////////////////////////////////////////
class EntityClass
{
public:
	EntityClass();
	EntityClass(const EntityClass&);
	~EntityClass();

	void SetActive(bool);
	void SetId(unsigned short);
	void SetType(char);
	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	bool IsActive();
	void GetId(unsigned short&);
	void GetType(char&);
	void GetPosition(float&, float&, float&);
	void GetRotation(float&, float&, float&);

private:
	bool m_active;
	unsigned short m_id;
	char m_entityType;
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
};

#endif
