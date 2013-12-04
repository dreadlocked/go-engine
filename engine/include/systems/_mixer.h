#ifndef __CSYSTEM_MIXER_H_
#define __CSYSTEM_MIXER_H_

#include "_globals.h"
#include "_system.h"
#include "_object.h"

// M�s sencillo:
// Audio Source y Audio Listener como componentes, no como audio.
class CSystem_Mixer: public CSystem
{
  friend class CComponent_Audio_Source;

  private:
    // Buffers reservados "one shots"...
    vector< ALuint > oneshot_used;
    vector< ALuint > oneshot_unused;

    // Buffers normales
    vector< ALuint > sources_used;
    vector< ALuint > sources_unused;

    static const uint NUMBER_SOURCES;
    static const uint NUMBER_SOURCES_ONESHOT;

    ALuint GetFreeSource();
    void AddFreeSource(ALuint source);

    ALuint GetFreeOneShot();

  public:
    CGameObject* listener; // Si es inv�lido o no reconocido, se usar� la c�mara principal del Render como listener.

  public:
    CSystem_Mixer(): CSystem() {};

    bool Init();
    void Close();

    void OnLoop();
};

extern CSystem_Mixer gSystem_Mixer;
extern CSystem_Mixer& gMixer;

#endif /* __CSYSTEM_MIXER_H_ */
