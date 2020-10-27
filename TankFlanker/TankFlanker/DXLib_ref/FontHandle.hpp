#pragma once
#include <DxLib.h>
#include <string_view>
class FontHandle {
private:
	int handle_;
	constexpr FontHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;

public:
	constexpr FontHandle(void) noexcept : handle_(invalid_handle) {}
	FontHandle(const FontHandle&) = delete;
	FontHandle(FontHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	FontHandle& operator=(const FontHandle&) = delete;
	FontHandle& operator=(FontHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~FontHandle(void) noexcept {
		if (-1 != this->handle_) {
			DeleteFontToHandle(this->handle_);
		}
	}
	void Dispose(void) noexcept {
		if (-1 != this->handle_) {
			DeleteFontToHandle(this->handle_);
			this->handle_ = -1;
		}
	}
	int get(void) const noexcept { return handle_; }
	bool DrawString(const int& x, const int& y, std::basic_string_view<TCHAR> String, unsigned int Color, unsigned int EdgeColor = 0, bool VerticalFlag = false) const noexcept {
		return DxLib::DrawNStringToHandle(x, y, String.data(), String.size(), Color, this->handle_, EdgeColor, VerticalFlag)==TRUE;
	}

	template <typename... Args>
	bool DrawStringFormat(const int& x, const int& y, unsigned int Color, std::string String, Args&&... args) const noexcept {
		return DxLib::DrawFormatStringToHandle(x, y, Color, this->handle_, String.c_str(), args...)==TRUE;
	}


	int GetDrawWidth(std::basic_string_view<TCHAR> String, bool VerticalFlag = false) const noexcept {
		return DxLib::GetDrawNStringWidthToHandle(String.data(), String.size(), this->handle_, VerticalFlag);
	}

	template <typename... Args>
	int GetDrawWidthFormat(std::string String, Args&&... args) const noexcept {
		return DxLib::GetDrawFormatStringWidthToHandle(this->handle_,String.c_str(),args...);
	}
	//右から揃え
	bool DrawString_RIGHT(const int& x, const int& y, std::basic_string_view<TCHAR> String, unsigned int Color, unsigned int EdgeColor = 0, bool VerticalFlag = false) const noexcept {
		return DxLib::DrawNStringToHandle(x-GetDrawWidth(String), y, String.data(), String.size(), Color, this->handle_, EdgeColor, VerticalFlag) == TRUE;
	}
	template <typename... Args>
	bool DrawStringFormat_RIGHT(const int& x, const int& y, unsigned int Color, std::string String, Args&&... args) const noexcept {
		return DxLib::DrawFormatStringToHandle(x-GetDrawWidthFormat(String.c_str(), args...), y, Color, this->handle_, String.c_str(), args...) == TRUE;
	}
	//中央揃え
	bool DrawString_MID(const int& x, const int& y, std::basic_string_view<TCHAR> String, unsigned int Color, unsigned int EdgeColor = 0, bool VerticalFlag = false) const noexcept {
		return DxLib::DrawNStringToHandle(x - GetDrawWidth(String)/2, y, String.data(), String.size(), Color, this->handle_, EdgeColor, VerticalFlag) == TRUE;
	}
	template <typename... Args>
	bool DrawStringFormat_MID(const int& x, const int& y, unsigned int Color, std::string String, Args&&... args) const noexcept {
		return DxLib::DrawFormatStringToHandle(x - GetDrawWidthFormat(String.c_str(), args...)/2, y, Color, this->handle_, String.c_str(), args...) == TRUE;
	}

	static FontHandle Create(std::basic_string_view<TCHAR> FontName, const int& Size, const int& FontType = -1, const int& CharSet = -1, const int& EdgeSize = -1, bool Italic = false) noexcept {
		return { DxLib::CreateFontToHandleWithStrLen(FontName.data(), FontName.length(), Size, Size/3, FontType, CharSet, EdgeSize, Italic) };
	}
	static FontHandle Create(const int& Size, const int& FontType = -1, const int& CharSet = -1, const int& EdgeSize = -1, bool Italic = false) noexcept {
		return { DxLib::CreateFontToHandle(nullptr, Size, Size / 3, FontType, CharSet, EdgeSize, Italic) };
	}
};
