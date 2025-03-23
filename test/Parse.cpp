#include <codecvt>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <locale>

#include <ase/ase.hpp>

namespace
{

// Convert from utf16 to whatever weird `wchar_t`-type the current platform
// uses. On Windows this is utf16, on linux this is utf32
std::wstring Utf16ToWchar(std::u16string_view utf16_string)
{
	static std::wstring_convert<
		std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>>
		Utf16WcharConverter;

	return Utf16WcharConverter.from_bytes(
		reinterpret_cast<const char*>(utf16_string.data()),
		reinterpret_cast<const char*>(utf16_string.data() + utf16_string.size())
	);
}

} // namespace

class ColorPrinter : public ase::IColorCallback
{
public:
	void GroupBegin(std::u16string_view Name) override
	{
		std::wprintf(
			L"%*ls Group: \"%ls\"", Depth * 4, L"", Utf16ToWchar(Name).c_str()
		);
		++Depth;
	}

	void GroupEnd() override
	{
		--Depth;
	}

	void ColorGray(std::u16string_view Name, const ase::ColorType::Gray& Color)
		override
	{
		std::wprintf(
			L"%*ls \"%ls\" Gray:\t[ %8.4f ]\n", Depth * 4, L"",
			Utf16ToWchar(Name).c_str(), Color.f32[0]
		);
	}

	void ColorRGB(std::u16string_view Name, const ase::ColorType::RGB& Color)
		override
	{
		std::wprintf(
			L"%*ls \"%ls\" RGB:\t[ %8.4f %8.4f %8.4f ]\n", Depth * 4, L"",
			Utf16ToWchar(Name).c_str(), Color.f32[0], Color.f32[1], Color.f32[2]
		);
	}

	void ColorLAB(std::u16string_view Name, const ase::ColorType::LAB& Color)
		override
	{
		std::wprintf(
			L"%*ls \"%ls\" LAB:\t[ %8.4f %8.4f %8.4f ]\n", Depth * TabWidth,
			L"", Utf16ToWchar(Name).c_str(), Color.f32[0], Color.f32[1],
			Color.f32[2]
		);
	}

	void ColorCMYK(std::u16string_view Name, const ase::ColorType::CMYK& Color)
		override
	{
		std::wprintf(
			L"%*ls \"%ls\" CMYK:\t[ %8.4f %8.4f %8.4f %8.4f ]\n",
			Depth * TabWidth, L"", Utf16ToWchar(Name).c_str(), Color.f32[0],
			Color.f32[1], Color.f32[2], Color.f32[3]
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
