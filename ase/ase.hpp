#pragma once
#include <stddef.h>
#include <string>

namespace ase
{
class ColorCallback
{
public:
	virtual void GroupBegin(
		const std::u16string &Name
	)
	{
	}

	virtual void GroupEnd()
	{
	}

	virtual void ColorGray(
		const std::u16string &Name,
		float Lightness
	)
	{
	};

	virtual void ColorRGB(
		const std::u16string &Name,
		float Red,
		float Green,
		float Blue
	)
	{
	};

	virtual void ColorLAB(
		const std::u16string &Name,
		float Lightness,
		float A,
		float B
	)
	{
	};

	virtual void ColorCYMK(
		const std::u16string &Name,
		float Cyan,
		float Magenta,
		float Yellow,
		float Key
	)
	{
	};
};

bool LoadFromFile(
	ColorCallback& Callback,
	const char* FileName
);

bool LoadFromStream(
	ColorCallback& Callback,
	std::istream &Stream
);

bool LoadFromMemory(
	ColorCallback& Callback,
	const void* Buffer,
	size_t Size
);
}