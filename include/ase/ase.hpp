#pragma once
#include <cmath>
#include <cstddef>
#include <span>
#include <string>

namespace ase
{
namespace ColorType
{
template<std::size_t Channels>
union ColorTuple
{
	constexpr static std::size_t ChannelCount = Channels;
	std::float_t                 f32[Channels];
	std::uint32_t                u32[Channels];

	static_assert(
		sizeof(std::float_t) == 4,
		"sizeof(std::float_t) != 4\n"
		"Ensure FLT_EVAL_METHOD is configured properly"
	);
};

using Gray = ColorTuple<1>;
using RGB  = ColorTuple<3>;
using LAB  = ColorTuple<3>;
using CMYK = ColorTuple<4>;
} // namespace ColorType

class IColorCallback
{
public:
	virtual ~IColorCallback() = default;

	virtual void GroupBegin(const std::u16string& Name);

	virtual void GroupEnd();

	virtual void
		ColorGray(const std::u16string& Name, ColorType::Gray Lightness);
	;

	virtual void ColorRGB(const std::u16string& Name, ColorType::RGB Color);
	;

	virtual void ColorLAB(const std::u16string& Name, ColorType::LAB Color);
	;

	virtual void ColorCMYK(const std::u16string& Name, ColorType::CMYK Color);
	;
};

bool LoadFromFile(IColorCallback& Callback, const char* FileName);

bool LoadFromStream(IColorCallback& Callback, std::istream& Stream);

bool LoadFromMemory(
	IColorCallback& Callback, std::span<const std::byte> Buffer
);
} // namespace ase
