#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <string>
#include <locale>
#include <codecvt>

#include <ase/ase.hpp>

class ColorPrinter : public ase::IColorCallback
{
public:
	ColorPrinter()
	{
	}
	~ColorPrinter()
	{
	}

	virtual void GroupBegin(
		const std::u16string& Name
	) override
	{
	}

	virtual void GroupEnd() override
	{
	}

	virtual void ColorGray(
		const std::u16string& Name,
		ase::ColorType::Gray Color
	) override
	{
		std::printf(
			"Gray:\t[ %8.4f ]\n",
			Color.f32[0]
		);
	}

	virtual void ColorRGB(
		const std::u16string& Name,
		ase::ColorType::RGB Color
	) override
	{
		std::printf(
			"RGB:\t[ %8.4f %8.4f %8.4f ]\n",
			Color.f32[0],
			Color.f32[1],
			Color.f32[2]
		);
	}

	virtual void ColorLAB(
		const std::u16string& Name,
		ase::ColorType::LAB Color
	) override
	{
		std::printf(
			"LAB:\t[ %8.4f %8.4f %8.4f ]\n",
			Color.f32[0],
			Color.f32[1],
			Color.f32[2]
		);
	}

	virtual void ColorCMYK(
		const std::u16string& Name,
		ase::ColorType::CMYK Color
	) override
	{
		std::printf(
			"CMYK:\t[ %8.4f %8.4f %8.4f %8.4f ]\n",
			Color.f32[0],
			Color.f32[1],
			Color.f32[2],
			Color.f32[3]
		);
	}
private:
};


int main( int argc, char* argv[])
{
	if( argc < 2 )
	{
		return EXIT_FAILURE;
	}
	ColorPrinter Printer;
	ase::LoadFromFile( Printer, argv[1] );
	return EXIT_SUCCESS;
}
