#include <chrono>

class Clock
{
  public:
    Clock() { Reset(); }

    void Reset() { m_Start = std::chrono::high_resolution_clock::now(); }

    float Elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() -
                                                                     m_Start)
                   .count() *
               0.001f * 0.001f;
    }
    float ElapsedMillis() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() -
                                                                     m_Start)
                   .count() *
               0.001f;
    }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};
