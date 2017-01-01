#include "ase.hpp"

#include <stdint.h>
#include <fstream>

#include <intrin.h>// Endian Swap

namespace ase
{
const uint32_t Magic = 'ASEF';

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

	Magic = _byteswap_ulong(Magic);

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

	Version[0] = _byteswap_ushort(Version[0]);
	Version[1] = _byteswap_ushort(Version[1]);
	BlockCount = _byteswap_ulong(BlockCount);

	uint16_t CurBlockClass;
	uint32_t CurBlockSize;
	// Process stream
	while( Stream )
	{
		Stream.read(
			reinterpret_cast<char*>(&CurBlockClass),
			sizeof(uint16_t)
		);
		CurBlockClass = _byteswap_ushort(CurBlockClass);

		switch( CurBlockClass )
		{
		case BlockClass::ColorEntry:
		case BlockClass::GroupBegin:
		{
			Stream.read(
				reinterpret_cast<char*>(&CurBlockSize),
				sizeof(uint32_t)
			);
			CurBlockSize = _byteswap_ulong(CurBlockSize);

			std::u16string EntryName;
			uint16_t EntryNameLength;

			Stream.read(
				reinterpret_cast<char*>(&EntryNameLength),
				sizeof(uint16_t)
			);
			EntryNameLength = _byteswap_ushort(EntryNameLength);


			EntryName.clear();
			EntryName.resize(EntryNameLength);

			Stream.read(
				reinterpret_cast<char*>(&EntryName[0]),
				EntryNameLength * 2
			);

			// Endian swap each character
			for( size_t i = 0; i < EntryNameLength; i++ )
			{
				EntryName[i] = _byteswap_ushort(EntryName[i]);
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
				ColorModel = _byteswap_ulong(ColorModel);

				float Channels[4];

				switch( ColorModel )
				{
				case ColorModel::CMYK:
				{
					Stream.read(
						reinterpret_cast<char*>(Channels),
						sizeof(float) * 4
					);

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
				ColorType = _byteswap_ushort(ColorType);
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

bool LoadFromMemory(
	ColorCallback& Callback,
	const void* Buffer,
	size_t Size
)
{
	return false;
}
}