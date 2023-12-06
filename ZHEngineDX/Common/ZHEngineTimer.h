#pragma once

#include <cmath>
#include <cstdint>
#include <exception>
#include <windows.h>

class ZHEngineTimer
{
public:
	ZHEngineTimer()noexcept(false):
		g_elapsedTicks(0),
		g_totalTicks(0),
		g_leftOverTicks(0),
		g_frameCount(0),
		g_framesPerSecond(0),
		g_framesThisSecond(0),
		g_SecondCounter(0),
		g_isFixedTimeStep(false),
		g_targetElapsedTicks(TicksPerSecond/60)
	{
		if (!QueryPerformanceFrequency(&g_Frequency))
		{
			throw std::exception("QueryPerformanceFrequency");
		}

		if (!QueryPerformanceCounter(&g_LastTime))
		{
			throw std::exception("QueryPerformanceCounter");
		}

		g_MaxDelta = static_cast<uint64_t>(g_Frequency.QuadPart / 10);
	}

public:
	uint64_t GetElapsedTicks()const noexcept { return g_elapsedTicks; }
	double GetElapsedSeconds() const noexcept { return TicksToSeconds(g_elapsedTicks); }

	uint64_t GetTotalTicks() const noexcept { return g_totalTicks; }
	double GetTotalSeconds() const noexcept { return TicksToSeconds(g_totalTicks); }

	uint32_t GetFrameCount() const noexcept { return g_frameCount; }
	uint32_t GetFramesPerSecond() const noexcept { return g_framesPerSecond; }

	void SetFixedTimeStep(bool isFixedTimestep) noexcept { g_isFixedTimeStep = isFixedTimestep; }

	void SetTargetElapsedTicks(uint64_t targetElapsed) noexcept { g_targetElapsedTicks = targetElapsed; }
	void SetTargetElapsedSeconds(double targetElapsed) noexcept { g_targetElapsedTicks = SecondsToTicks(targetElapsed); }

	static const uint64_t TicksPerSecond = 10000000;
	static constexpr double TicksToSeconds(uint64_t ticks)noexcept { return static_cast<double>(ticks) / TicksPerSecond; }
	static constexpr uint64_t SecondsToTicks(double seconds) noexcept { return static_cast<uint64_t>(seconds * TicksPerSecond); }

public:
	void ResetElapsedTime()
	{
		if (!QueryPerformanceCounter(&g_LastTime))
		{
			throw std::exception("QueryPerformanceCounter");
		}

		g_leftOverTicks = 0;
		g_framesPerSecond = 0;
		g_framesThisSecond = 0;
		g_SecondCounter = 0;
	}


    template<typename TUpdate>
    void Tick(const TUpdate& update)
    {
        // Query the current time.
        LARGE_INTEGER currentTime;

        if (!QueryPerformanceCounter(&currentTime))
        {
            throw std::exception("QueryPerformanceCounter");
        }

        uint64_t timeDelta = static_cast<uint64_t>(currentTime.QuadPart - g_LastTime.QuadPart);

        g_LastTime = currentTime;
        g_SecondCounter += timeDelta;

        // Clamp excessively large time deltas (e.g. after paused in the debugger).
        if (timeDelta > g_MaxDelta)
        {
            timeDelta = g_MaxDelta;
        }

        // Convert QPC units into a canonical tick format. This cannot overflow due to the previous clamp.
        timeDelta *= TicksPerSecond;
        timeDelta /= static_cast<uint64_t>(g_Frequency.QuadPart);

        uint32_t lastFrameCount = g_frameCount;

        if (g_isFixedTimeStep)
        {
            // Fixed timestep update logic

            // If the app is running very close to the target elapsed time (within 1/4 of a millisecond) just clamp
            // the clock to exactly match the target value. This prevents tiny and irrelevant errors
            // from accumulating over time. Without this clamping, a game that requested a 60 fps
            // fixed update, running with vsync enabled on a 59.94 NTSC display, would eventually
            // accumulate enough tiny errors that it would drop a frame. It is better to just round
            // small deviations down to zero to leave things running smoothly.

            if (static_cast<uint64_t>(std::abs(static_cast<int64_t>(timeDelta - g_targetElapsedTicks))) < TicksPerSecond / 4000)
            {
                timeDelta = g_targetElapsedTicks;
            }

            g_leftOverTicks += timeDelta;

            while (g_leftOverTicks >= g_targetElapsedTicks)
            {
                g_elapsedTicks = g_targetElapsedTicks;
                g_totalTicks += g_targetElapsedTicks;
                g_leftOverTicks -= g_targetElapsedTicks;
                g_frameCount++;

                update();
            }
        }
        else
        {
            // Variable timestep update logic.
            g_elapsedTicks = timeDelta;
            g_totalTicks += timeDelta;
            g_leftOverTicks = 0;
            g_frameCount++;

            update();
        }

        // Track the current framerate.
        if (g_frameCount != lastFrameCount)
        {
            g_framesThisSecond++;
        }

        if (g_SecondCounter >= static_cast<uint64_t>(g_Frequency.QuadPart))
        {
            g_framesPerSecond = g_framesThisSecond;
            g_framesThisSecond = 0;
            g_SecondCounter %= static_cast<uint64_t>(g_Frequency.QuadPart);
        }
    }

private:
	LARGE_INTEGER g_Frequency;
	LARGE_INTEGER g_LastTime;
	uint64_t g_MaxDelta;

	uint64_t g_elapsedTicks;
	uint64_t g_totalTicks;
	uint64_t g_leftOverTicks;

	uint32_t g_frameCount;
	uint32_t g_framesPerSecond;
	uint32_t g_framesThisSecond;
    uint64_t g_SecondCounter;

	bool g_isFixedTimeStep;
	uint64_t g_targetElapsedTicks;
};