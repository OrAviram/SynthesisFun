#pragma once

#include <Windows.h>
#include <thread>
#include <mutex>
#include <iostream>

template<typename T>
class SoundEngine
{
	int samplesPerSecond;
	int channelsCount;
	int blocksCount;
	int samplesPerBlock;

	HWAVEOUT device = {};
	T* samples = nullptr;
	WAVEHDR* waveHeaders = nullptr;

	std::thread mainThread = {};
	std::atomic<bool> running = false;
	std::atomic<int> freeBlocksCount;
	int currentBlock;

	static double clip(double in, double max)
	{
		if (in >= max)
			return max;

		if (in <= -max)
			return -max;

		return in;
	}

public:
	double(*waveFunction)(double, int) = nullptr;

	SoundEngine(int samplesPerSecond, int channelsCount, int blocksCount = 8, int samplesPerBlock = 255)
		: samplesPerSecond(samplesPerSecond), channelsCount(channelsCount), blocksCount(blocksCount), samplesPerBlock(samplesPerBlock)
	{
		freeBlocksCount = blocksCount;
		currentBlock = 0;
	}

	~SoundEngine()
	{
		Stop();
	}

	bool Start()
	{
		WAVEFORMATEX waveFormat = {};
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = channelsCount;
		waveFormat.nSamplesPerSec = samplesPerSecond;
		waveFormat.wBitsPerSample = sizeof(T) * 8;
		waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
		waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
		waveFormat.cbSize = 0;

		if (waveOutOpen(&device, WAVE_MAPPER, &waveFormat, (DWORD_PTR)(&WaveOutProc), (DWORD_PTR)this, CALLBACK_FUNCTION) != S_OK)
			return false;

		samples = new T[blocksCount * samplesPerBlock];
		ZeroMemory(samples, sizeof(T) * blocksCount * samplesPerBlock);

		waveHeaders = new WAVEHDR[blocksCount];
		ZeroMemory(waveHeaders, sizeof(WAVEHDR) * blocksCount);

		for (int i = 0; i < blocksCount; i++)
		{
			waveHeaders[i].dwBufferLength = samplesPerBlock * sizeof(T);
			waveHeaders[i].lpData = (LPSTR)(samples + (i * samplesPerBlock));
		}

		mainThread = std::thread(&SoundEngine::MainThreadCallback, this);

		running = true;
		return true;
	}

	void SetVolume(unsigned short leftVolume, unsigned short rightVolume) {
		if (!running)
			return;

		unsigned short left = unsigned(0xFFFF * leftVolume);
		unsigned short right = unsigned(0xFFFF * rightVolume);

		unsigned long x = left + (right << 16);
		waveOutSetVolume(device, x);
	}

	void Stop()
	{
		if (!running)
			return;

		running = false;
		mainThread.join();
		waveOutClose(device);
		delete[] samples;
		delete[] waveHeaders;
	}

private:
	static void CALLBACK WaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		if (uMsg != WOM_DONE)
			return;

		SoundEngine& engine = *((SoundEngine*)dwInstance);
		engine.freeBlocksCount++;
	}

	void MainThreadCallback()
	{
		double timeElapsed = 0;
		double timeStep = 1.0 / samplesPerSecond;

		double maxSample = (double)(pow(2, (sizeof(T) * 8 - 1)) - 1);

		while (running)
		{
			if (freeBlocksCount == 0)
				continue;

			freeBlocksCount--;
			
			if (waveHeaders[currentBlock].dwFlags & WHDR_PREPARED)
				waveOutUnprepareHeader(device, &waveHeaders[currentBlock], sizeof(WAVEHDR));

			int samplesBaseIndex = currentBlock * samplesPerBlock;
			for (int i = 0; i < samplesPerBlock && running; i += channelsCount)
			{
				for (int channel = 0; channel < channelsCount && running; channel++)
				{
					T sample = waveFunction == nullptr ? 0 : (T)(clip(waveFunction(timeElapsed, channel), 1) * maxSample);
					samples[samplesBaseIndex + i + channel] = sample;
				}
				timeElapsed += timeStep;
			}

			waveOutPrepareHeader(device, &waveHeaders[currentBlock], sizeof(WAVEHDR));
			waveOutWrite(device, &waveHeaders[currentBlock], sizeof(WAVEHDR));
			currentBlock = (currentBlock + 1) % blocksCount;
		}
	}
};