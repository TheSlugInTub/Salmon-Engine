#include <salmon/sound_source.h>
#include <iostream>
#include <thread>

SoundSource::SoundSource()
{
    alGenSources(1, &p_Source);
    alSourcef(p_Source, AL_PITCH, p_Pitch);
    alSourcef(p_Source, AL_GAIN, p_Gain);
    alSource3f(p_Source, AL_POSITION, p_Position[0], p_Position[1], p_Position[2]);
    alSource3f(p_Source, AL_VELOCITY, p_Velocity[0], p_Velocity[1], p_Velocity[2]);
    alSourcei(p_Source, AL_LOOPING, p_LoopSound);
    alSourcei(p_Source, AL_BUFFER, p_Buffer);
}

SoundSource::~SoundSource()
{
    alDeleteSources(1, &p_Source);
}

void SoundSource::Play(const Sound sound)
{
    if (sound.sound != p_Buffer)
    {
        p_Buffer = sound.sound;
        alSourcei(p_Source, AL_BUFFER, (ALint)p_Buffer);
    }

    alSourcePlay(p_Source);

    // Capture 'this' in a weak_ptr to ensure the object isn't destructed prematurely
    std::weak_ptr<SoundSource> weakSelf = shared_from_this();
    std::thread playbackThread(
        [weakSelf]()
        {
            if (auto self = weakSelf.lock())
            {
                self->MonitorPlayback();
            }
        });
    playbackThread.detach();
}

void SoundSource::MonitorPlayback()
{
    ALint state = AL_PLAYING;
    while (state == AL_PLAYING && alGetError() == AL_NO_ERROR)
    {
        alGetSourcei(p_Source, AL_SOURCE_STATE, &state);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep to avoid busy-waiting
    }
}
