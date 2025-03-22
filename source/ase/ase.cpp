#include <ase/ase.hpp>

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>

namespace
{

template<std::endian Endianness = std::endian::big, std::size_t N>
constexpr std::uint32_t Magic32(const char (&TagString)[N])
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

void IColorCallback::GroupBegin(std::u16string_view Name)
{
}

void IColorCallback::GroupEnd()
{
}

void IColorCallback::ColorGray(
	std::u16string_view Name, ColorType::Gray Lightness
)
{
}

void IColorCallback::ColorRGB(std::u16string_view Name, ColorType::RGB Color)
{
}

void IColorCallback::ColorLAB(std::u16string_view Name, ColorType::LAB Color)
{
}

void IColorCallback::ColorCMYK(std::u16string_view Name, ColorType::CMYK Color)
{
}

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

	std::uint32_t Magic;
	Stream.read(reinterpret_cast<char*>(&Magic), sizeof(std::uint32_t));

	Magic = std::byteswap(Magic);

	if( Magic != Magic32("ASEF") )
	{
		return false;
	}

	std::uint16_t Version[2];
	std::uint32_t BlockCount;

	Stream.read(reinterpret_cast<char*>(&Version[0]), sizeof(std::uint16_t));
	Stream.read(reinterpret_cast<char*>(&Version[1]), sizeof(std::uint16_t));
	Stream.read(reinterpret_cast<char*>(&BlockCount), sizeof(std::uint32_t));

	Version[0] = std::byteswap(Version[0]);
	Version[1] = std::byteswap(Version[1]);
	BlockCount = std::byteswap(BlockCount);

	std::uint16_t CurBlockClass;
	std::uint32_t CurBlockSize;
	// Process stream
	while( (BlockCount--) != 0u )
	{
		Stream.read(
			reinterpret_cast<char*>(&CurBlockClass), sizeof(std::uint16_t)
		);
		CurBlockClass = std::byteswap(CurBlockClass);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			Stream.read(
				reinterpret_cast<char*>(&CurBlockSize), sizeof(std::uint32_t)
			);
			CurBlockSize = std::byteswap(CurBlockSize);

			std::u16string EntryName;
			std::uint16_t  EntryNameLength;

			Stream.read(
				reinterpret_cast<char*>(&EntryNameLength), sizeof(std::uint16_t)
			);
			EntryNameLength = std::byteswap(EntryNameLength);

			EntryName.clear();
			EntryName.resize(EntryNameLength);

			Stream.read(
				reinterpret_cast<char*>(EntryName.data()),
				static_cast<std::streamsize>(EntryNameLength * 2)
			);

			// Endian swap each character
			for( std::size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = std::byteswap(EntryName[i]);
			}

			if( CurBlockClass == BlockClass::GroupBegin )
			{
				Callback.GroupBegin(EntryName);
			}
			else if( CurBlockClass == BlockClass::ColorEntry )
			{
				std::uint32_t ColorModel;

				Stream.read(
					reinterpret_cast<char*>(&ColorModel), sizeof(std::uint32_t)
				);
				ColorModel = std::byteswap(ColorModel);

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					ColorType::CMYK CurColor;
					Stream.read(
						reinterpret_cast<char*>(&CurColor),
						sizeof(ColorType::CMYK)
					);
					CurColor.u32[0] = std::byteswap(CurColor.u32[0]);
					CurColor.u32[1] = std::byteswap(CurColor.u32[1]);
					CurColor.u32[2] = std::byteswap(CurColor.u32[2]);
					CurColor.u32[3] = std::byteswap(CurColor.u32[3]);
					Callback.ColorCMYK(EntryName, CurColor);
					break;
				}
				case ColorModel::RGB:
				{
					ColorType::RGB CurColor;
					Stream.read(
						reinterpret_cast<char*>(&CurColor),
						sizeof(ColorType::RGB)
					);
					CurColor.u32[0] = std::byteswap(CurColor.u32[0]);
					CurColor.u32[1] = std::byteswap(CurColor.u32[1]);
					CurColor.u32[2] = std::byteswap(CurColor.u32[2]);
					Callback.ColorRGB(EntryName, CurColor);
					break;
				}
				case ColorModel::LAB:
				{
					ColorType::LAB CurColor;
					Stream.read(
						reinterpret_cast<char*>(&CurColor),
						sizeof(ColorType::LAB)
					);
					CurColor.u32[0] = std::byteswap(CurColor.u32[0]);
					CurColor.u32[1] = std::byteswap(CurColor.u32[1]);
					CurColor.u32[2] = std::byteswap(CurColor.u32[2]);
					Callback.ColorLAB(EntryName, CurColor);
					break;
				}
				case ColorModel::GRAY:
				{
					ColorType::Gray CurColor;
					Stream.read(
						reinterpret_cast<char*>(&CurColor),
						sizeof(ColorType::Gray)
					);
					CurColor.u32[0] = std::byteswap(CurColor.u32[0]);
					Callback.ColorGray(EntryName, CurColor);
					break;
				}
				default:
				{
					// Unknown color model
					return false;
				}
				}

				std::uint16_t ColorCategory;

				Stream.read(
					reinterpret_cast<char*>(&ColorCategory),
					sizeof(std::uint16_t)
				);
				ColorCategory = std::byteswap(ColorCategory);
			}

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

namespace
{

// Read a type from a span of bytes, and offset the span
// forward by the size of the type
template<typename T>
[[nodiscard]]
inline T ReadType(std::span<const std::byte>& Bytes)
{
	// assert(Bytes.size_bytes() >= sizeof(T));
	const T& Result = *reinterpret_cast<const T*>(Bytes.data());
	Bytes           = Bytes.subspan(sizeof(T));
	return Result;
}

template<>
[[nodiscard]]
inline std::uint16_t ReadType<std::uint16_t>(std::span<const std::byte>& Bytes)
{
	const std::uint16_t& Result
		= *reinterpret_cast<const std::uint16_t*>(Bytes.data());
	Bytes = Bytes.subspan(sizeof(std::uint16_t));
	return std::byteswap(Result);
}

template<>
[[nodiscard]]
inline std::uint32_t ReadType<std::uint32_t>(std::span<const std::byte>& Bytes)
{
	// assert(Bytes.size_bytes() >= sizeof(T));
	const std::uint32_t& Result
		= *reinterpret_cast<const std::uint32_t*>(Bytes.data());
	Bytes = Bytes.subspan(sizeof(std::uint32_t));
	return std::byteswap(Result);
}

template<>
[[nodiscard]]
inline float ReadType<float>(std::span<const std::byte>& Bytes)
{
	static_assert(sizeof(float) == 4);
	return std::bit_cast<float>(ReadType<std::uint32_t>(Bytes));
}

} // namespace

bool LoadFromMemory(IColorCallback& Callback, std::span<const std::byte> Buffer)
{
	if( Buffer.data() == nullptr || Buffer.empty() )
	{
		return false;
	}

	const std::uint32_t Magic = ReadType<std::uint32_t>(Buffer);

	if( Magic != Magic32("ASEF") )
	{
		return false;
	}

	std::uint16_t Version[2];

	Version[0]               = ReadType<std::uint16_t>(Buffer);
	Version[1]               = ReadType<std::uint16_t>(Buffer);
	std::uint32_t BlockCount = ReadType<std::uint32_t>(Buffer);

	// Process stream
	while( (BlockCount--) != 0u )
	{
		const std::uint16_t CurBlockClass = ReadType<std::uint16_t>(Buffer);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			const std::uint32_t CurBlockSize = ReadType<std::uint32_t>(Buffer);
			(void)CurBlockSize;

			const std::uint16_t EntryNameLength
				= ReadType<std::uint16_t>(Buffer);

			std::u16string EntryName;
			EntryName.resize(EntryNameLength);

			// Endian swap each character
			for( std::size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = ReadType<std::uint16_t>(Buffer);
			}

			if( CurBlockClass == BlockClass::GroupBegin )
			{
				Callback.GroupBegin(EntryName);
			}
			else if( CurBlockClass == BlockClass::ColorEntry )
			{
				const std::uint32_t ColorModel
					= ReadType<std::uint32_t>(Buffer);

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					ColorType::CMYK CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(Buffer);
					CurColor.f32[1] = ReadType<std::float_t>(Buffer);
					CurColor.f32[2] = ReadType<std::float_t>(Buffer);
					CurColor.f32[3] = ReadType<std::float_t>(Buffer);
					Callback.ColorCMYK(EntryName, CurColor);
					break;
				}
				case ColorModel::RGB:
				{
					ColorType::RGB CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(Buffer);
					CurColor.f32[1] = ReadType<std::float_t>(Buffer);
					CurColor.f32[2] = ReadType<std::float_t>(Buffer);
					Callback.ColorRGB(EntryName, CurColor);
					break;
				}
				case ColorModel::LAB:
				{
					ColorType::LAB CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(Buffer);
					CurColor.f32[1] = ReadType<std::float_t>(Buffer);
					CurColor.f32[2] = ReadType<std::float_t>(Buffer);
					Callback.ColorLAB(EntryName, CurColor);
					break;
				}
				case ColorModel::GRAY:
				{
					ColorType::Gray CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(Buffer);
					Callback.ColorGray(EntryName, CurColor);
					break;
				}
				default:
				{
					// Unknown Block Class
					return false;
				}
				}

				const std::uint16_t ColorCategory
					= ReadType<std::uint16_t>(Buffer);
				(void)ColorCategory;
			}

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
