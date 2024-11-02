#include <sound_buffer.h>
#include <audiofile.h>
#include <inttypes.h>
#include <AL/alext.h>
#include <vector>
#include <iostream>

SoundBuffer* SoundBuffer::Get()
{
    static SoundBuffer* sndbuf = new SoundBuffer();
    return sndbuf;
}

Sound SoundBuffer::AddSoundEffect(const char* filename)
{
    ALenum err, format;
    ALuint buffer;
    AudioFile<float> audioFile;
    short* membuf;
    ALsizei num_bytes;

    /* Open the audio file and check that it's usable. */
    bool loaded = audioFile.load(filename);
    if (!loaded)
    {
        std::cerr << "Could not open audio in " << filename << std::endl;
        std::string filenameString = filename;
    }

    const int numChannels = audioFile.getNumChannels();
    const int numSamplesPerChannel = audioFile.getNumSamplesPerChannel();

    /* Get the sound format, and figure out the OpenAL format */
    format = AL_NONE;
    if (numChannels == 1)
        format = AL_FORMAT_MONO16;
    else if (numChannels == 2)
        format = AL_FORMAT_STEREO16;
    else
    {
        std::cerr << "Unsupported channel count: " << numChannels << std::endl;
    }

    /* Decode the whole audio file to a buffer. */
    membuf = static_cast<short*>(malloc(numChannels * numSamplesPerChannel * sizeof(short)));
    for (int i = 0; i < numSamplesPerChannel; ++i)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            membuf[i * numChannels + channel] = static_cast<short>(audioFile.samples[channel][i] * 32767.0f);
        }
    }

    num_bytes = numChannels * numSamplesPerChannel * sizeof(short);

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file.
     */
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, membuf, num_bytes, audioFile.getSampleRate());

    free(membuf);

    /* Check if an error occurred, and clean up if so. */
    err = alGetError();
    if (err != AL_NO_ERROR)
    {
        std::cerr << "OpenAL Error: " << alGetString(err) << std::endl;
        std::string errorString = alGetString(err);
        if (buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
    }

    p_SoundEffectBuffers.push_back(buffer); // add to the list of known buffers

    Sound newSound(filename, buffer);

    return newSound;
}

bool SoundBuffer::RemoveSoundEffect(const ALuint& buffer)
{
    auto it = p_SoundEffectBuffers.begin();
    while (it != p_SoundEffectBuffers.end())
    {
        if (*it == buffer)
        {
            alDeleteBuffers(1, &*it);
            it = p_SoundEffectBuffers.erase(it);
            return true;
        }
        else
        {
            ++it;
        }
    }
    return false; // couldn't find to remove
}

SoundBuffer::SoundBuffer()
{
    p_SoundEffectBuffers.clear();
}

SoundBuffer::~SoundBuffer()
{
    alDeleteBuffers(p_SoundEffectBuffers.size(), p_SoundEffectBuffers.data());
    p_SoundEffectBuffers.clear();
}
