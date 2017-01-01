#include "ase.hpp"

#include <stdint.h>
#include <fstream>

#if defined(_MSC_VER)

#include <intrin.h>
#define SWAP32(x) _byteswap_ulong(x)
#define SWAP16(x) _byteswap_ushort(x)

#elif defined(__GNUC__) || defined(__clang__)

#define SWAP32(x) __builtin_bswap32(x)
#define SWAP16(x) __builtin_bswap16(x)
#else

inline uint16_t SWAP16(uint16_t x)
{
	return (
		((x & 0x00FF) << 8) |
		((x & 0xFF00) >> 8)
		);
}

inline uint32_t SWAP32(uint32_t x)
{
	return(
		((x & 0x000000FF) << 24) |
		((x & 0x0000FF00) << 8) |
		((x & 0x00FF0000) >> 8) |
		((x & 0xFF000000) >> 24)
		);
}

#endif

namespace ase
{
enum BlockClass : uint16_t
{
	ColorEntry = 0x0001,
	GroupBegin = 0xC001,
	GroupEnd = 0xC002
};

enum ColorModel : uint32_t
{
	// Big Endian
	CMYK = 'CMYK',
	RGB = 'RGB ',
	LAB = 'LAB ',
	GRAY = 'Gray'
};

enum ColorType : uint16_t
{
	Global = 0,
	Spot = 1,
	Normal = 2
};

bool LoadFromFile(
	ColorCallback& Callback,
	const char* FileName
)
{
	std::ifstream SwatchFile;

	SwatchFile.open(
		FileName,
		std::ios::binary
	);

	return LoadFromStream(Callback, SwatchFile);
}

bool LoadFromStream(
	ColorCallback& Callback,
	std::istream &Stream
)
{
	if( !Stream )
	{
		return false;
	}

	uint32_t Magic;
	Stream.read(
		reinterpret_cast<char*>(&Magic),
		sizeof(uint32_t)
	);

	Magic = SWAP32(Magic);

	if( Magic != 'ASEF' )
	{
		return false;
	}

	uint16_t Version[2];
	uint32_t BlockCount;

	Stream.read(
		reinterpret_cast<char*>(&Version[0]),
		sizeof(uint16_t)
	);
	Stream.read(
		reinterpret_cast<char*>(&Version[1]),
		sizeof(uint16_t)
	);
	Stream.read(
		reinterpret_cast<char*>(&BlockCount),
		sizeof(uint32_t)
	);

	Version[0] = SWAP16(Version[0]);
	Version[1] = SWAP16(Version[1]);
	BlockCount = SWAP32(BlockCount);

	uint16_t CurBlockClass;
	uint32_t CurBlockSize;
	// Process stream
	while( BlockCount-- )
	{
		Stream.read(
			reinterpret_cast<char*>(&CurBlockClass),
			sizeof(uint16_t)
		);
		CurBlockClass = SWAP16(CurBlockClass);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			Stream.read(
				reinterpret_cast<char*>(&CurBlockSize),
				sizeof(uint32_t)
			);
			CurBlockSize = SWAP32(CurBlockSize);

			std::u16string EntryName;
			uint16_t EntryNameLength;

			Stream.read(
				reinterpret_cast<char*>(&EntryNameLength),
				sizeof(uint16_t)
			);
			EntryNameLength = SWAP16(EntryNameLength);

			EntryName.clear();
			EntryName.resize(EntryNameLength);

			Stream.read(
				reinterpret_cast<char*>(&EntryName[0]),
				EntryNameLength * 2
			);

			// Endian swap each character
			for( size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = SWAP16(EntryName[i]);
			}

			if( CurBlockClass == BlockClass::GroupBegin )
			{
				Callback.GroupBegin(
					EntryName
				);
			}
			else if( CurBlockClass == BlockClass::ColorEntry )
			{
				uint32_t ColorModel;

				Stream.read(
					reinterpret_cast<char*>(&ColorModel),
					sizeof(uint32_t)
				);
				ColorModel = SWAP32(ColorModel);

				float Channels[4];

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					Stream.read(
						reinterpret_cast<char*>(Channels),
						sizeof(float) * 4
					);
					Channels[0] = SWAP32(Channels[0]);
					Channels[1] = SWAP32(Channels[1]);
					Channels[2] = SWAP32(Channels[2]);
					Channels[3] = SWAP32(Channels[3]);
					Callback.ColorCYMK(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2],
						Channels[3]
					);
					break;
				}
				case ColorModel::RGB:
				{
					Stream.read(
						reinterpret_cast<char*>(Channels),
						sizeof(float) * 3
					);
					Channels[0] = SWAP32(Channels[0]);
					Channels[1] = SWAP32(Channels[1]);
					Channels[2] = SWAP32(Channels[2]);
					Callback.ColorRGB(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2]
					);
					break;
				}
				case ColorModel::LAB:
				{
					Stream.read(
						reinterpret_cast<char*>(Channels),
						sizeof(float) * 3
					);
					Channels[0] = SWAP32(Channels[0]);
					Channels[1] = SWAP32(Channels[1]);
					Channels[2] = SWAP32(Channels[2]);
					Callback.ColorLAB(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2]
					);
					break;
				}
				case ColorModel::GRAY:
				{
					Stream.read(
						reinterpret_cast<char*>(Channels),
						sizeof(float) * 1
					);
					Channels[0] = SWAP32(Channels[0]);
					Callback.ColorGray(
						EntryName,
						Channels[0]
					);
					break;
				}
				}

				uint16_t ColorType;

				Stream.read(
					reinterpret_cast<char*>(&ColorType),
					sizeof(uint16_t)
				);
				ColorType = SWAP16(ColorType);
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

template< typename T >
inline T ReadType(const void* &Pointer)
{
	const T *Temp = static_cast<const T*>(Pointer);
	Pointer = static_cast<const T*>(Pointer) + static_cast<ptrdiff_t>(1);
	return *Temp;
}

template<>
inline uint32_t ReadType<uint32_t>(const void* &Pointer)
{
	const uint32_t *Temp = static_cast<const uint32_t*>(Pointer);
	Pointer = static_cast<const uint32_t*>(Pointer) + static_cast<ptrdiff_t>(1);

	return SWAP32(*Temp);
}

template<>
inline uint16_t ReadType<uint16_t>(const void* &Pointer)
{
	const uint16_t *Temp = static_cast<const uint16_t*>(Pointer);
	Pointer = static_cast<const uint16_t*>(Pointer) + static_cast<ptrdiff_t>(1);

	return SWAP16(*Temp);
}

template<>
inline float ReadType<float>(const void* &Pointer)
{
	const float *Temp = static_cast<const float*>(Pointer);
	Pointer = static_cast<const float*>(Pointer) + static_cast<ptrdiff_t>(1);

	return SWAP32(*reinterpret_cast<const uint32_t*>(Temp));
}

inline void Read(const void* &Pointer, void* Dest, size_t Count)
{
	Dest = memcpy(Dest, Pointer, Count);
	Pointer = static_cast<const uint8_t*>(Pointer) + Count;
}

bool LoadFromMemory(
	ColorCallback& Callback,
	const void* Buffer,
	size_t Size
)
{
	if( Buffer == nullptr )
	{
		return false;
	}

	const void* ReadPoint = Buffer;

	uint32_t Magic = ReadType<uint32_t>(ReadPoint);

	if( Magic != 'ASEF' )
	{
		return false;
	}

	uint16_t Version[2];
	uint32_t BlockCount;

	Version[0] = ReadType<uint16_t>(ReadPoint);
	Version[1] = ReadType<uint16_t>(ReadPoint);
	BlockCount = ReadType<uint32_t>(ReadPoint);

	uint16_t CurBlockClass;
	uint32_t CurBlockSize;
	// Process stream
	while( BlockCount-- )
	{
		CurBlockClass = ReadType<uint16_t>(ReadPoint);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			CurBlockSize = ReadType<uint32_t>(ReadPoint);

			uint16_t EntryNameLength;

			EntryNameLength = ReadType<uint16_t>(ReadPoint);

			std::u16string EntryName;
			EntryName.resize(EntryNameLength);
			Read(ReadPoint, &EntryName[0], EntryNameLength * 2);

			// Endian swap each character
			for( size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = SWAP16(EntryName[i]);
			}

			if( CurBlockClass == BlockClass::GroupBegin )
			{
				Callback.GroupBegin(
					EntryName
				);
			}
			else if( CurBlockClass == BlockClass::ColorEntry )
			{
				uint32_t ColorModel;

				ColorModel = ReadType<uint32_t>(ReadPoint);

				float Channels[4];

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					Channels[0] = ReadType<float>(ReadPoint);
					Channels[1] = ReadType<float>(ReadPoint);
					Channels[2] = ReadType<float>(ReadPoint);
					Channels[3] = ReadType<float>(ReadPoint);
					Callback.ColorCYMK(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2],
						Channels[3]
					);
					break;
				}
				case ColorModel::RGB:
				{
					Channels[0] = ReadType<float>(ReadPoint);
					Channels[1] = ReadType<float>(ReadPoint);
					Channels[2] = ReadType<float>(ReadPoint);
					Callback.ColorRGB(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2]
					);
					break;
				}
				case ColorModel::LAB:
				{
					Channels[0] = ReadType<float>(ReadPoint);
					Channels[1] = ReadType<float>(ReadPoint);
					Channels[2] = ReadType<float>(ReadPoint);
					Callback.ColorLAB(
						EntryName,
						Channels[0],
						Channels[1],
						Channels[2]
					);
					break;
				}
				case ColorModel::GRAY:
				{
					Channels[0] = ReadType<float>(ReadPoint);
					Callback.ColorGray(
						EntryName,
						Channels[0]
					);
					break;
				}
				}

				uint16_t ColorType;
				ColorType = ReadType<uint16_t>(ReadPoint);
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
}