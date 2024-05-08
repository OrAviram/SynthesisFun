#pragma once
#include <math.h>

enum class Waveform
{
	SINE = 0, TRIANGLE = 1, SQUARE = 2, SAWTOOTH = 3, RANDOM_NOISE = 4
};

namespace osc
{
    constexpr double PI = 3.14159265359;
    constexpr double PI2 = 2 * PI;

    // Let T = 1/frequency (imagine the period of our wave).
    // For time >= 0, the returned value is a number d in [0, 1) such that time = (n+x)T for some nonegative integer n.
    // So if f is a function with period T, we have f(time) = f(xT).
    inline double Remainder(double frequency, double time)
    {
        double tf = time * frequency;
        return tf - floor(tf);
    }

    inline double SineWave(double frequency, double time)
    {
        return sin(frequency * PI * time);
    }

    inline double SquareWave(double frequency, double time)
    {
        return Remainder(frequency, time) < 0.5 ? 1 : -1;
    }

    inline double SawtoothWave(double frequency, double time)
    {
        return 1 - Remainder(frequency, time);
    }

    double PulseWave(double frequency, double time, double dutyCycle)
    {
        return Remainder(frequency, time) < dutyCycle ? 1 : -1;
    }

    double RandomNoiseWave()
    {
        return 2 * ((double)rand() / RAND_MAX) - 1;
    }

    inline double TriangleWave(double frequency, double time)
    {
        double r = Remainder(frequency, time);
        double value;
        if (r < 0.25)
            value = r;
        else if (r < 0.5)
            value = 0.5 - r;
        else if (r < 0.75)
            value = r - 0.5;
        else
            value = 1 - r;

        return value * 4;
    }

    inline double Oscilate(double frequency, double time, Waveform type)
    {
        switch (type)
        {
        case Waveform::SINE:
            return SineWave(frequency, time);

        case Waveform::SQUARE:
            return SquareWave(frequency, time);

        case Waveform::TRIANGLE:
            return TriangleWave(frequency, time);

        case Waveform::SAWTOOTH:
            return SawtoothWave(frequency, time);

        case Waveform::RANDOM_NOISE:
            return RandomNoiseWave();
        }

        return 0;
    }
}