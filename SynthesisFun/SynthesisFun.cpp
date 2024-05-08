#include <iostream>
#include <vector>
#include <string>
#include "SoundEngineAttempt.h"
#include "OldConsole.h"
#include "Oscilator.h"

using namespace std;

constexpr double PI = 3.14159265359;
constexpr double PI2 = 2 * PI;

double frequency = 0;
double amplitude = 1;
Waveform waveform = Waveform::SINE;

static vector<WAVEOUTCAPS> Enumerate()
{
    int nDeviceCount = waveOutGetNumDevs();
    vector<WAVEOUTCAPS> sDevices;
    WAVEOUTCAPS woc;
    for (int n = 0; n < nDeviceCount; n++)
        if (waveOutGetDevCaps(n, &woc, sizeof(WAVEOUTCAPS)) == S_OK)
            sDevices.push_back(woc);
    return sDevices;
}

double Wavner(double time, int channel)
{
    return amplitude * osc::Oscilate(frequency, time, waveform);
}

bool IsKeyDown(int key)
{
    return (GetAsyncKeyState(key) & 0x10000) != 0;
}

int main()
{
    int channels = 1;
    SoundEngine<short> engine(44100, channels, 8 * channels, 255 * channels);
    if (!engine.Start())
    {
        std::cout << ":(" << std::endl;
        return 1;
    }
    engine.waveFunction = &Wavner;
    double baseFrequency = 110;

    engine.SetVolume(1, 1);

    OldConsole console(100, 30);
    console.SetActive();
 
    bool running = true;
    int y = 0;
    while (running)
    {
        console.Clear();

        bool keyPressed = false;
        for (int k = 0; k < 15; k++)
        {
            if (IsKeyDown("ZSXCFVGBNJMK\xbcL\xbe"[k]))
            {
                frequency = baseFrequency * pow(2, k / 12.0);
                keyPressed = true;
            }
        }

        console.Draw(0, 0, L"Amplitude: " + std::to_wstring(amplitude));
        double amplitudeSpeed = 0.0001;
        if (IsKeyDown(VK_UP))
            amplitude = amplitude + amplitudeSpeed <= 1 ? amplitude + amplitudeSpeed : 1;
        else if (IsKeyDown(VK_DOWN))
            amplitude = amplitude - amplitudeSpeed >= 0 ? amplitude - amplitudeSpeed : 0;

        const wchar_t* formNames[5];
        formNames[(int)Waveform::SINE] = L"Sine";
        formNames[(int)Waveform::TRIANGLE] = L"Triangle";
        formNames[(int)Waveform::SQUARE] = L"Square";
        formNames[(int)Waveform::SAWTOOTH] = L"Sawtooth";
        formNames[(int)Waveform::RANDOM_NOISE] = L"Random Noise";
        for (int form = 1; form <= 5; form++) {
            std::wstring formStr = std::to_wstring(form);
            std::wstring draw = formStr + L" => " + formNames[form - 1];
            if ((int)waveform == form - 1)
                draw = draw + L" (selected)";
            console.Draw(0, form, draw);
            if (IsKeyDown(formStr[0]))
                waveform = (Waveform)(form - 1);
        }

        console.Draw(0, 20, L"Use up/down keys to modify amplitude.");
        console.Draw(0, 21, L"Select waveform with number keys.");
        console.Draw(0, 22, L"Play notes using last two rows on keyboard.");
        console.Draw(0, 23, L"Press escape to quit.");

        if (!keyPressed)
            frequency = 0;

        console.Refresh();

        if (IsKeyDown(VK_ESCAPE))
        {
            engine.Stop();
            running = false;
        }
    }
}