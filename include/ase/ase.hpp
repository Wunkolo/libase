#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace ase
{

namespace ColorType
{
template<std::size_t Channels>
union ColorTuple
{
	constexpr static std::size_t        ChannelCount = Channels;
	std::array<std::float_t, Channels>  f32;
	std::array<std::uint32_t, Channels> u32;
};

static_assert(
	sizeof(std::float_t) == 4,
	"sizeof(std::float_t) != 4\n"
	"Ensure FLT_EVAL_METHOD is configured properly"
);

using Gray = ColorTuple<1>;
using RGB  = ColorTuple<3>;
using LAB  = ColorTuple<3>;
using CMYK = ColorTuple<4>;
} // namespace ColorType

class IColorCallback
{
public:
	virtual ~IColorCallback() = default;

	virtual void GroupBegin(std::u16string_view Name);

	virtual void GroupEnd();

	virtual void
		ColorGray(std::u16string_view Name, const ColorType::Gray& Lightness);
	;

	virtual void
		ColorRGB(std::u16string_view Name, const ColorType::RGB& Color);
	;

	virtual void
		ColorLAB(std::u16string_view Name, const ColorType::LAB& Color);
	;

	virtual void
		ColorCMYK(std::u16string_view Name, const ColorType::CMYK& Color);
	;
};

bool LoadFromFile(IColorCallback& Callback, const char* FileName);

bool LoadFromStream(IColorCallback& Callback, std::istream& Stream);

bool LoadFromMemory(
	IColorCallback& Callback, std::span<const std::byte> Buffer
);
} // namespace ase
