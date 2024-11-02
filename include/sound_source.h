#pragma once

#include <AL\al.h>
#include <sound_buffer.h>
#include <iostream>
#include <memory>

class SoundSource : public std::enable_shared_from_this<SoundSource>
{
  public:
    SoundSource();
    ~SoundSource();

    void Play(const Sound sound);
    void MonitorPlayback();

    float p_Pitch = 1.f;
    float p_Gain = 1.f;
    float p_Position[3] = {0, 0, 0};
    float p_Velocity[3] = {0, 0, 0};
    bool p_LoopSound = false;

  private:
    ALuint p_Source;
    ALuint p_Buffer = 0;
};
