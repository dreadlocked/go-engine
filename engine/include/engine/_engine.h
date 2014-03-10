#ifndef __ENGINE_H_
#define __ENGINE_H_

#include "_globals.h"
#include "engine/_instance.h"

#include "systems/_render.h"

// Engine resource files
// icono, etc

class CEngine
{
  protected:
    //vector<CInstance*> instances;
    map<string, CInstance*> instances;
    string title;
    bool running;

    string current_instance;

    // Arguments
    vector<string> arguments;

  protected:
    void RemoveAllInstances();

  public:
    CEngine();
    virtual ~CEngine(){};

    virtual bool Init(int argc, char* argv[]);
    virtual void Close();

    virtual int OnExecute(int argc, char* argv[]);

    // Should be 32x32 RGBA format
    void SetIcon(string icon_name);

    string GetTitle()
    {
      return title;
    }

    void SetTitle(const char* text)
    {
      if(text)
        title = text;

      SDL_SetWindowTitle(gSystem_Render.window, title.c_str());
    }

    void AddInstance(fboolpointer load_gameObject_function, string resource_file, string instance_name);

    GLdouble fps()
    {
      return instances[current_instance]->fps();
    }

    CInstance* CurrentInstance()
    {
      return instances[current_instance];
    }

    vector<string>& Arguments()
    {
      return arguments;
    }

    // Exits without saving.
    // Should never be used
    void Exit()
    {
      exit(0);
    }

    void Quit()
    {
      running = false;
      if(instances.find(current_instance) != instances.end())
        instances[current_instance]->i_running = false;
    }

    string GetCurrentInstance()
    {
      return current_instance;
    }

    string GetNextInstance()
    {
      if(instances.find(current_instance) != instances.end())
        return instances[current_instance]->next_instance;

      return "";
    }

    void NextInstance(string instance)
    {
      if(instances.find(current_instance) != instances.end())
      {
        instances[current_instance]->NextInstance(instance);
      }
    }
};

extern CEngine gEngine;


#endif /* __ENGINE_H_ */
