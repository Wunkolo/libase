#pragma once
#include <cstddef>
#include <cmath>
#include <string>
#include <vector>

namespace ase
{
namespace ColorType
{
template< std::size_t Channels >
struct Colorf32
{
	std::float_t Channel[Channels];

	std::float_t& operator[](std::size_t Index)
	{
		return Channel[Index];
	}

	static_assert(
		sizeof(std::float_t) == 4,
		"sizeof(std::float_t) != 4\n"
		"Ensure FLT_EVAL_METHOD is configured properly"
	);
};

using Gray = Colorf32<1>;
using RGB = Colorf32<3>;
using LAB = Colorf32<3>;
using CMYK = Colorf32<4>;
}

class IColorCallback
{
public:
	virtual ~IColorCallback() = default;

	virtual void GroupBegin(
		const std::u16string& Name
	);

	virtual void GroupEnd();

	virtual void ColorGray(
		const std::u16string& Name,
		ColorType::Gray Lightness
	);;

	virtual void ColorRGB(
		const std::u16string& Name,
		ColorType::RGB Color
	);;

	virtual void ColorLAB(
		const std::u16string& Name,
		ColorType::LAB Color
	);;

	virtual void ColorCMYK(
		const std::u16string& Name,
		ColorType::CMYK Color
	);;
};

bool LoadFromFile(
	IColorCallback& Callback,
	const char* FileName
);

bool LoadFromStream(
	IColorCallback& Callback,
	std::istream& Stream
);

bool LoadFromMemory(
	IColorCallback& Callback,
	const void* Buffer,
	std::size_t Size
);
}
