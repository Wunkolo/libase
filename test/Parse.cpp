#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <codecvt>
#include <iostream>
#include <locale>
#include <string>

#include <ase/ase.hpp>

class ColorPrinter : public ase::IColorCallback
{
public:
	void GroupBegin(std::u16string_view Name) override
	{
		++Depth;
	}

	void GroupEnd() override
	{
		--Depth;
	}

	void
		ColorGray(std::u16string_view Name, ase::ColorType::Gray Color) override
	{
		std::printf("%*sGray:\t[ %8.4f ]\n", Depth * 4, "", Color.f32[0]);
	}

	void ColorRGB(std::u16string_view Name, ase::ColorType::RGB Color) override
	{
		std::printf(
			"%*sRGB:\t[ %8.4f %8.4f %8.4f ]\n", Depth * 4, "", Color.f32[0],
			Color.f32[1], Color.f32[2]
		);
	}

	void ColorLAB(std::u16string_view Name, ase::ColorType::LAB Color) override
	{
		std::printf(
			"%*sLAB:\t[ %8.4f %8.4f %8.4f ]\n", Depth * TabWidth, "",
			Color.f32[0], Color.f32[1], Color.f32[2]
		);
	}

	void
		ColorCMYK(std::u16string_view Name, ase::ColorType::CMYK Color) override
	{
		std::printf(
			"%*sCMYK:\t[ %8.4f %8.4f %8.4f %8.4f ]\n", Depth * TabWidth, "",
			Color.f32[0], Color.f32[1], Color.f32[2], Color.f32[3]
		);
	}

private:
	std::int32_t                 Depth    = 0;
	static constexpr std::size_t TabWidth = 4;
};

int main(int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}
	ColorPrinter Printer;
	ase::LoadFromFile(Printer, argv[1]);
	return EXIT_SUCCESS;
}
