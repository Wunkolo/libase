#pragma once
#include <stddef.h>
#include <string>
#include <vector>

namespace ase
{
namespace ColorType
{
template< size_t Channels >
struct Colorf32
{
	float Channel[Channels];

	float& operator[](size_t Index)
	{
		return Channel[Index];
	}
};

using Gray = Colorf32<1>;
using RGB = Colorf32<3>;
using LAB = Colorf32<3>;
using CMYK = Colorf32<4>;
}

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
		ColorType::Gray Lightness
	)
	{
	};

	virtual void ColorRGB(
		const std::u16string &Name,
		ColorType::RGB Color
	)
	{
	};

	virtual void ColorLAB(
		const std::u16string &Name,
		ColorType::LAB Color
	)
	{
	};

	virtual void ColorCMYK(
		const std::u16string &Name,
		ColorType::CMYK Color
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