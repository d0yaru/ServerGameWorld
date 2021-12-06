////////////////////////////////////////////////////////////////////////////////
// Filename: main.cpp
////////////////////////////////////////////////////////////////////////////////


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "systemclass.h"


int main()
{
  SystemClass* System;
  bool result;


  // Create the system object.
  System = new SystemClass;
  if(!System)
  {
    return 0;
  }

  // Initialize the system object.
  result = System->Initialize();
  if(!result)
  {
    return 0;
  }

  // Loop and run the system frame function.
  while(System->Online() == true)
  {
    System->Frame();
  }

  // Release the system object.
  System->Shutdown();
  delete System;
  System = 0;

  return 0;
}
