#pragma once

#include <Windows.h>
#include <string>

class OldConsole
{
	short width;
	short height;
	wchar_t* screen;
	HANDLE handle;

public:
	OldConsole(short width, short height)
		: width(width), height(height)
	{
		screen = new wchar_t[width * height];
		handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SMALL_RECT rect = { 0, 0, width - 1, height - 1 };
		SetConsoleWindowInfo(handle, TRUE, &rect);
		SetConsoleScreenBufferSize(handle, { width, height });
	}

	~OldConsole()
	{
		delete[] screen;
	}

	void SetActive()
	{
		SetConsoleActiveScreenBuffer(handle);
	}

	void Clear()
	{
		ZeroMemory(screen, width * height * sizeof(wchar_t));
	}

	void Draw(int x, int y, const std::wstring& value)
	{
		for (size_t i = 0; i < value.size(); i++)
		{
			int index = y * width + x + i;
			if (index < width * height)
				screen[index] = value[i];
		}
	}

	void Refresh()
	{
		DWORD bytesWritten = 0;
		WriteConsoleOutputCharacter(handle, screen, width * height, { 0,0 }, &bytesWritten);
	}
};