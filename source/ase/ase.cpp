#include <ase/ase.hpp>

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{

template<std::endian Endianness = std::endian::big, std::size_t N>
consteval std::uint32_t Magic32(const char (&TagString)[N])
{
	static_assert(N == 5, "Tag must be 4 characters");
	if constexpr( Endianness == std::endian::big )
	{
		return (
			(TagString[3] << 0) | (TagString[2] << 8) | (TagString[1] << 16)
			| (TagString[0] << 24)
		);
	}
	else
	{
		return (
			(TagString[3] << 24) | (TagString[2] << 16) | (TagString[1] << 8)
			| (TagString[0] << 0)
		);
	}
}

// Read a type from a span of bytes, and offset the span
// forward by the size of the type. Note that this will modify the incoming span
// to after the read type, allowing a chain of calls to move the span forward
// continuously. Will assert if the are not enough bytes to read the type
template<typename T>
[[nodiscard]]
inline T ReadSwap(std::span<const std::byte>&)
{
	static_assert("New Big-Endian specialization needed");
}

template<>
[[nodiscard]]
inline std::uint16_t ReadSwap<std::uint16_t>(std::span<const std::byte>& Bytes)
{
	assert(Bytes.size_bytes() >= sizeof(std::uint16_t));
	const std::uint16_t& Result
		= *reinterpret_cast<const std::uint16_t*>(Bytes.data());
	Bytes = Bytes.subspan(sizeof(std::uint16_t));
	return std::byteswap(Result);
}

template<>
[[nodiscard]]
inline std::uint32_t ReadSwap<std::uint32_t>(std::span<const std::byte>& Bytes)
{
	assert(Bytes.size_bytes() >= sizeof(std::uint32_t));
	const std::uint32_t& Result
		= *reinterpret_cast<const std::uint32_t*>(Bytes.data());
	Bytes = Bytes.subspan(sizeof(std::uint32_t));
	return std::byteswap(Result);
}

template<>
[[nodiscard]]
inline float ReadSwap<float>(std::span<const std::byte>& Bytes)
{
	static_assert(sizeof(float) == 4);
	return std::bit_cast<float>(ReadSwap<std::uint32_t>(Bytes));
}

} // namespace

namespace ase
{
enum BlockClass : std::uint16_t
{
	ColorEntry = 0x0001,
	GroupBegin = 0xC001,
	GroupEnd   = 0xC002
};

enum ColorModel : std::uint32_t
{
	// Big Endian
	CMYK = Magic32("CMYK"),
	RGB  = Magic32("RGB "),
	LAB  = Magic32("LAB "),
	GRAY = Magic32("Gray")
};

enum ColorCategory : std::uint16_t
{
	Global = 0,
	Spot   = 1,
	Normal = 2
};

struct Header
{
	std::uint32_t Identifier;
	std::uint16_t VersionMajor;
	std::uint16_t VersionMinor;
	std::uint32_t BlockCount;
};
static_assert(sizeof(Header) == 12);
static_assert(offsetof(Header, Identifier) == 0);
static_assert(offsetof(Header, VersionMajor) == 4);
static_assert(offsetof(Header, VersionMinor) == 6);
static_assert(offsetof(Header, BlockCount) == 8);

void IColorCallback::GroupBegin(std::u16string_view Name)
{
}

void IColorCallback::GroupEnd()
{
}

void IColorCallback::ColorGray(
	std::u16string_view Name, const ColorType::Gray& Lightness
)
{
}

void IColorCallback::ColorRGB(
	std::u16string_view Name, const ColorType::RGB& Color
)
{
}

void IColorCallback::ColorLAB(
	std::u16string_view Name, const ColorType::LAB& Color
)
{
}

void IColorCallback::ColorCMYK(
	std::u16string_view Name, const ColorType::CMYK& Color
)
{
}

namespace
{

Header ReadHeader(std::span<const std::byte> Buffer)
{
	Header Result;

	Result.Identifier   = ReadSwap<std::uint32_t>(Buffer);
	Result.VersionMajor = ReadSwap<std::uint16_t>(Buffer);
	Result.VersionMinor = ReadSwap<std::uint16_t>(Buffer);
	Result.BlockCount   = ReadSwap<std::uint32_t>(Buffer);

	return Result;
}

bool ReadBlock(
	IColorCallback& Callback, BlockClass CurBlockClass,
	std::span<const std::byte> Buffer
)
{
	const std::uint16_t EntryNameLength = ReadSwap<std::uint16_t>(Buffer);

	std::u16string EntryName;
	EntryName.resize(EntryNameLength, L'\0');

	// Endian swap each character
	for( std::size_t i = 0; i < EntryNameLength; i++ )
	{
		EntryName[i] = ReadSwap<std::uint16_t>(Buffer);
	}

	if( CurBlockClass == BlockClass::GroupBegin )
	{
		Callback.GroupBegin(EntryName);
	}
	else if( CurBlockClass == BlockClass::ColorEntry )
	{
		const std::uint32_t ColorModel = ReadSwap<std::uint32_t>(Buffer);

		switch( ColorModel )
		{
		case ColorModel::CMYK:
		{
			ColorType::CMYK CurColor;
			CurColor.f32[0] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[1] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[2] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[3] = ReadSwap<std::float_t>(Buffer);
			Callback.ColorCMYK(EntryName, CurColor);
			break;
		}
		case ColorModel::RGB:
		{
			ColorType::RGB CurColor;
			CurColor.f32[0] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[1] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[2] = ReadSwap<std::float_t>(Buffer);
			Callback.ColorRGB(EntryName, CurColor);
			break;
		}
		case ColorModel::LAB:
		{
			ColorType::LAB CurColor;
			CurColor.f32[0] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[1] = ReadSwap<std::float_t>(Buffer);
			CurColor.f32[2] = ReadSwap<std::float_t>(Buffer);
			Callback.ColorLAB(EntryName, CurColor);
			break;
		}
		case ColorModel::GRAY:
		{
			ColorType::Gray CurColor;
			CurColor.f32[0] = ReadSwap<std::float_t>(Buffer);
			Callback.ColorGray(EntryName, CurColor);
			break;
		}
		default:
		{
			// Unknown Block Class
			return false;
		}
		}

		const std::uint16_t ColorCategory = ReadSwap<std::uint16_t>(Buffer);
		(void)ColorCategory;
	}

	return true;
}
} // namespace

bool LoadFromFile(IColorCallback& Callback, const char* FileName)
{
	std::ifstream SwatchFile;

	SwatchFile.open(FileName, std::ios::binary);

	return LoadFromStream(Callback, SwatchFile);
}

bool LoadFromStream(IColorCallback& Callback, std::istream& Stream)
{
	if( !Stream )
	{
		return false;
	}

	// Re-used buffer for data reads from the stream
	// Mostly used to allow for a small pool of "std::span" functions
	// to exist without having to handle streams and spans differently
	std::vector<std::byte> CurBufferData(sizeof(Header), std::byte{});

	if( Stream.read(
			reinterpret_cast<char*>(CurBufferData.data()), sizeof(Header)
		);
		!Stream )
	{
		return false;
	}

	const Header CurHeader = ReadHeader(CurBufferData);

	if( CurHeader.Identifier != Magic32("ASEF") )
	{
		return false;
	}

	// Process stream
	std::uint32_t BlockCount = CurHeader.BlockCount;
	while( (BlockCount--) != 0u )
	{
		BlockClass CurBlockClass;
		{
			std::uint16_t CurBlockClassRaw;
			if( Stream.read(
					reinterpret_cast<char*>(&CurBlockClassRaw),
					sizeof(std::uint16_t)
				);
				!Stream )
			{
				return false;
			}
			CurBlockClassRaw = std::byteswap(CurBlockClassRaw);
			CurBlockClass    = static_cast<BlockClass>(CurBlockClassRaw);
		}

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			std::uint32_t CurBlockSize;
			Stream.read(
				reinterpret_cast<char*>(&CurBlockSize), sizeof(std::uint32_t)
			);
			CurBlockSize = std::byteswap(CurBlockSize);

			CurBufferData.resize(CurBlockSize, {});
			Stream.read(
				reinterpret_cast<char*>(CurBufferData.data()), CurBlockSize
			);

			ReadBlock(Callback, CurBlockClass, CurBufferData);

			break;
		}
		case BlockClass::GroupEnd:
		{
			Callback.GroupEnd();
			break;
		}
		default:
		{
			// Unknown block class
			return false;
		}
		}
	}

	return true;
}

bool LoadFromMemory(IColorCallback& Callback, std::span<const std::byte> Buffer)
{
	if( Buffer.data() == nullptr || Buffer.empty() )
	{
		return false;
	}

	const Header CurHeader = ReadHeader(Buffer);

	Buffer = Buffer.subspan(sizeof(Header));

	if( CurHeader.Identifier != Magic32("ASEF") )
	{
		return false;
	}

	// Process stream
	std::uint32_t BlockCount = CurHeader.BlockCount;
	while( (BlockCount--) != 0u )
	{
		const BlockClass CurBlockClass
			= static_cast<BlockClass>(ReadSwap<std::uint16_t>(Buffer));

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			const std::uint32_t CurBlockSize = ReadSwap<std::uint32_t>(Buffer);
			(void)CurBlockSize;

			ReadBlock(Callback, CurBlockClass, Buffer);

			Buffer = Buffer.subspan(CurBlockSize);

			break;
		}
		case BlockClass::GroupEnd:
		{
			Callback.GroupEnd();
			break;
		}
		}
	}

	return true;
}
} // namespace ase
