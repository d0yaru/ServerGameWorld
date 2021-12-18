////////////////////////////////////////////////////////////////////////////////
// Filename: networkmessages.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _NETWORKMESSAGES_H_
#define _NETWORKMESSAGES_H_


///////////////////////////
// NETWORK MESSAGE TYPES //
///////////////////////////
#define MSG_CONNECT 1000
#define MSG_NEWID 1001
#define MSG_PING 1002
#define MSG_DISCONNECT 1003
#define MSG_CHAT 1004
#define MSG_ENTITY_REQUEST 1005
#define MSG_ENTITY_INFO 1006
#define MSG_NEW_USER_LOGIN 1007
#define MSG_USER_DISCONNECT 1008
#define MSG_STATE_CHANGE 1009
#define MSG_POSITION 1010
#define MSG_AI_ROTATE 1011


////////////////////////////////
// NETWORK MESSAGE STRUCTURES //
////////////////////////////////
typedef struct
{
	unsigned short type;
}MSG_GENERIC_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_SIMPLE_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_NEWID_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_PING_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_DISCONNECT_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	char text[64];
}MSG_CHAT_DATA;

typedef struct
{
	unsigned short type;
	unsigned short entityId;
	char entityType;
	float positionX, positionY, positionZ;
	float rotationX, rotationY, rotationZ;
}MSG_ENTITY_INFO_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
}MSG_USER_DISCONNECT_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	char state;
}MSG_STATE_CHANGE_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	float positionX, positionY, positionZ;
	float rotationX, rotationY, rotationZ;
}MSG_POSITION_DATA;

typedef struct
{
	unsigned short type;
	unsigned short idNumber;
	bool rotate;
}MSG_AI_ROTATE_DATA;

#endif
