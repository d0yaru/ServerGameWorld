////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "timerclass.h"
#include "networkclass.h"
#include "zoneclass.h"



////////////////////////////////////////////////////////////////////////////////
// Class name: SystemClass
////////////////////////////////////////////////////////////////////////////////
class SystemClass
{
public:
    SystemClass();
    SystemClass(const SystemClass&);
    ~SystemClass();

    bool Initialize();
    void Shutdown();
    void Frame();

    bool Online();

private:
    bool m_online;
    TimerClass* m_Timer;
    NetworkClass* m_Network;
    ZoneClass* m_BlackForest;
};

#endif
