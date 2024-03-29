#include <ase/ase.hpp>

#include <cstdint>
#include <cstring>
#include <fstream>

#if defined(_MSC_VER)

#include <intrin.h>
#define SWAP32(x) _byteswap_ulong(x)
#define SWAP16(x) _byteswap_ushort(x)

#elif defined(__GNUC__) || defined(__clang__)

#define SWAP32(x) __builtin_bswap32(x)
#define SWAP16(x) __builtin_bswap16(x)
#else

inline std::uint16_t SWAP16(std::uint16_t x)
{
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

inline std::uint32_t SWAP32(std::uint32_t x)
{
	return (
		((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8)
		| ((x & 0x00FF0000) >> 8) | ((x & 0xFF000000) >> 24)
	);
}

#endif

constexpr std::uint32_t Magic32(char Byte1, char Byte2, char Byte3, char Byte4)
{
	return static_cast<std::uint32_t>(
		static_cast<std::uint8_t>(Byte1) << 24
		| static_cast<std::uint8_t>(Byte2) << 16
		| static_cast<std::uint8_t>(Byte3) << 8
		| static_cast<std::uint8_t>(Byte4)
	);
}

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
	CMYK = Magic32('C', 'M', 'Y', 'K'),
	RGB  = Magic32('R', 'G', 'B', ' '),
	LAB  = Magic32('L', 'A', 'B', ' '),
	GRAY = Magic32('G', 'r', 'a', 'y')
};

enum ColorCategory : std::uint16_t
{
	Global = 0,
	Spot   = 1,
	Normal = 2
};

void IColorCallback::GroupBegin(const std::u16string& Name)
{
}

void IColorCallback::GroupEnd()
{
}

void IColorCallback::ColorGray(
	const std::u16string& Name, ColorType::Gray Lightness
)
{
}

void IColorCallback::ColorRGB(const std::u16string& Name, ColorType::RGB Color)
{
}

void IColorCallback::ColorLAB(const std::u16string& Name, ColorType::LAB Color)
{
}

void IColorCallback::ColorCMYK(
	const std::u16string& Name, ColorType::CMYK Color
)
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

	Magic = SWAP32(Magic);

	if( Magic != Magic32('A', 'S', 'E', 'F') )
	{
		return false;
	}

	std::uint16_t Version[2];
	std::uint32_t BlockCount;

	Stream.read(reinterpret_cast<char*>(&Version[0]), sizeof(std::uint16_t));
	Stream.read(reinterpret_cast<char*>(&Version[1]), sizeof(std::uint16_t));
	Stream.read(reinterpret_cast<char*>(&BlockCount), sizeof(std::uint32_t));

	Version[0] = SWAP16(Version[0]);
	Version[1] = SWAP16(Version[1]);
	BlockCount = SWAP32(BlockCount);

	std::uint16_t CurBlockClass;
	std::uint32_t CurBlockSize;
	// Process stream
	while( BlockCount-- )
	{
		Stream.read(
			reinterpret_cast<char*>(&CurBlockClass), sizeof(std::uint16_t)
		);
		CurBlockClass = SWAP16(CurBlockClass);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			Stream.read(
				reinterpret_cast<char*>(&CurBlockSize), sizeof(std::uint32_t)
			);
			CurBlockSize = SWAP32(CurBlockSize);

			std::u16string EntryName;
			std::uint16_t  EntryNameLength;

			Stream.read(
				reinterpret_cast<char*>(&EntryNameLength), sizeof(std::uint16_t)
			);
			EntryNameLength = SWAP16(EntryNameLength);

			EntryName.clear();
			EntryName.resize(EntryNameLength);

			Stream.read(
				reinterpret_cast<char*>(&EntryName[0]), EntryNameLength * 2
			);

			// Endian swap each character
			for( std::size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = SWAP16(EntryName[i]);
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
				ColorModel = SWAP32(ColorModel);

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					ColorType::CMYK CurColor;
					Stream.read(
						reinterpret_cast<char*>(&CurColor),
						sizeof(ColorType::CMYK)
					);
					CurColor.u32[0] = SWAP32(CurColor.u32[0]);
					CurColor.u32[1] = SWAP32(CurColor.u32[1]);
					CurColor.u32[2] = SWAP32(CurColor.u32[2]);
					CurColor.u32[3] = SWAP32(CurColor.u32[3]);
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
					CurColor.u32[0] = SWAP32(CurColor.u32[0]);
					CurColor.u32[1] = SWAP32(CurColor.u32[1]);
					CurColor.u32[2] = SWAP32(CurColor.u32[2]);
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
					CurColor.u32[0] = SWAP32(CurColor.u32[0]);
					CurColor.u32[1] = SWAP32(CurColor.u32[1]);
					CurColor.u32[2] = SWAP32(CurColor.u32[2]);
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
					CurColor.u32[0] = SWAP32(CurColor.u32[0]);
					Callback.ColorGray(EntryName, CurColor);
					break;
				}
				}

				std::uint16_t ColorCategory;

				Stream.read(
					reinterpret_cast<char*>(&ColorCategory),
					sizeof(std::uint16_t)
				);
				ColorCategory = SWAP16(ColorCategory);
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

template<typename T>
inline T ReadType(const void*& Pointer)
{
	const T* Temp = static_cast<const T*>(Pointer);
	Pointer = static_cast<const T*>(Pointer) + static_cast<std::ptrdiff_t>(1);
	return *Temp;
}

template<>
inline std::uint32_t ReadType<std::uint32_t>(const void*& Pointer)
{
	const std::uint32_t* Temp = static_cast<const std::uint32_t*>(Pointer);
	Pointer                   = static_cast<const std::uint32_t*>(Pointer)
			+ static_cast<std::ptrdiff_t>(1);

	return SWAP32(*Temp);
}

template<>
inline std::uint16_t ReadType<std::uint16_t>(const void*& Pointer)
{
	const std::uint16_t* Temp = static_cast<const std::uint16_t*>(Pointer);
	Pointer                   = static_cast<const std::uint16_t*>(Pointer)
			+ static_cast<std::ptrdiff_t>(1);

	return SWAP16(*Temp);
}

template<>
inline float ReadType<float>(const void*& Pointer)
{
	std::uint32_t Temp = SWAP32(*static_cast<const std::uint32_t*>(Pointer));
	Pointer
		= static_cast<const float*>(Pointer) + static_cast<std::ptrdiff_t>(1);
	return *reinterpret_cast<float*>(&Temp);
}

bool LoadFromMemory(IColorCallback& Callback, std::span<const std::byte> Buffer)
{
	if( Buffer.data() == nullptr || Buffer.empty() )
	{
		return false;
	}

	const void* ReadPoint = reinterpret_cast<const void*>(Buffer.data());

	const std::uint32_t Magic = ReadType<std::uint32_t>(ReadPoint);

	if( Magic != Magic32('A', 'S', 'E', 'F') )
	{
		return false;
	}

	std::uint16_t Version[2];

	Version[0]               = ReadType<std::uint16_t>(ReadPoint);
	Version[1]               = ReadType<std::uint16_t>(ReadPoint);
	std::uint32_t BlockCount = ReadType<std::uint32_t>(ReadPoint);

	// Process stream
	while( BlockCount-- )
	{
		const std::uint16_t CurBlockClass = ReadType<std::uint16_t>(ReadPoint);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			std::uint32_t CurBlockSize = ReadType<std::uint32_t>(ReadPoint);

			const std::uint16_t EntryNameLength
				= ReadType<std::uint16_t>(ReadPoint);

			std::u16string EntryName;
			EntryName.resize(EntryNameLength);

			// Endian swap each character
			for( std::size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = ReadType<std::uint16_t>(ReadPoint);
			}

			if( CurBlockClass == BlockClass::GroupBegin )
			{
				Callback.GroupBegin(EntryName);
			}
			else if( CurBlockClass == BlockClass::ColorEntry )
			{
				const std::uint32_t ColorModel
					= ReadType<std::uint32_t>(ReadPoint);

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					ColorType::CMYK CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[1] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[2] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[3] = ReadType<std::float_t>(ReadPoint);
					Callback.ColorCMYK(EntryName, CurColor);
					break;
				}
				case ColorModel::RGB:
				{
					ColorType::RGB CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[1] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[2] = ReadType<std::float_t>(ReadPoint);
					Callback.ColorRGB(EntryName, CurColor);
					break;
				}
				case ColorModel::LAB:
				{
					ColorType::LAB CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[1] = ReadType<std::float_t>(ReadPoint);
					CurColor.f32[2] = ReadType<std::float_t>(ReadPoint);
					Callback.ColorLAB(EntryName, CurColor);
					break;
				}
				case ColorModel::GRAY:
				{
					ColorType::Gray CurColor;
					CurColor.f32[0] = ReadType<std::float_t>(ReadPoint);
					Callback.ColorGray(EntryName, CurColor);
					break;
				}
				}

				std::uint16_t ColorCategory
					= ReadType<std::uint16_t>(ReadPoint);
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
