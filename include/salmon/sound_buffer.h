#ifndef SOUND_BUFFER_H
#define SOUND_BUFFER_H

#include <AL\al.h>
#include <salmon/stl_pch.h>

struct Sound
{
    const char* filename;
    uint32_t sound;
};

class SoundBuffer
{
  public:
    static SoundBuffer* Get();

    Sound AddSoundEffect(const char* filename);
    bool RemoveSoundEffect(const ALuint& buffer);

  private:
    SoundBuffer();
    ~SoundBuffer();

    std::vector<ALuint> p_SoundEffectBuffers;
};

#endif
