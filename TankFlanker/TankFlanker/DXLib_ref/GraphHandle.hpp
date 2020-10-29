#pragma once
#include <DxLib.h>
#include <string_view>
class GraphHandle {
private:
	int handle_;
	constexpr GraphHandle(int h) noexcept : handle_(h) {}
	static constexpr int invalid_handle = -1;

public:
	constexpr GraphHandle(void) noexcept : handle_(invalid_handle) {}
	GraphHandle(const GraphHandle&) = delete;
	GraphHandle(GraphHandle&& o) noexcept : handle_(o.handle_) {
		o.handle_ = invalid_handle;
	}
	GraphHandle& operator=(const GraphHandle&) = delete;
	GraphHandle& operator=(GraphHandle&& o) noexcept {
		this->handle_ = o.handle_;
		o.handle_ = invalid_handle;
		return *this;
	}
	~GraphHandle(void) noexcept {
		if (this->handle_ != -1) { DeleteGraph(this->handle_); }
	}
	void Dispose(void) noexcept {
		if (this->handle_ != -1) {
			DeleteGraph(this->handle_);
			this->handle_ = -1;
		}
	}
	int get(void) const noexcept { return handle_; }

	GraphHandle Duplicate(void) const noexcept { return this->handle_; }

	static GraphHandle Load(std::basic_string_view<TCHAR> FileName, bool NotUse3DFlag = false) noexcept {
		return { DxLib::LoadGraphWithStrLen(FileName.data(), FileName.length(), NotUse3DFlag) };
	}
	static GraphHandle LoadDiv(std::basic_string_view<TCHAR> FileName, const int& AllNum, const int& XNum, const int& YNum, const int& XSize, const int& YSize, int *HandleArray, bool NotUse3DFlag = false) noexcept {
		return { DxLib::LoadDivGraphWithStrLen(FileName.data(), FileName.length(), AllNum, XNum, YNum,   XSize, YSize, HandleArray, NotUse3DFlag) };
	}

	static GraphHandle Make(const int& SizeX, const int& SizeY, bool trns = false) noexcept {
		return { DxLib::MakeScreen(SizeX, SizeY, (trns ? TRUE : FALSE)) };
	}

	void DrawGraph(const int& posx, const int& posy, bool trns) noexcept {
		if (this->handle_ != -1) {
			DxLib::DrawGraph(posx, posy, this->handle_, (trns ? TRUE : FALSE));
		}
	}
	void DrawExtendGraph(const int& posx1, const int& posy1, const int& posx2, const int& posy2, bool trns) noexcept {
		if (this->handle_ != -1) {
			DxLib::DrawExtendGraph(posx1, posy1, posx2, posy2, this->handle_, (trns ? TRUE : FALSE));
		}
	}
	//GetGraphSize
	void GetSize(int*xsize, int*ysize) noexcept {
		if (this->handle_ != -1) {
			GetGraphSize(this->handle_, xsize, ysize);
		}
	}
	//
	void SetDraw_Screen(const bool& clear = true) {
		SetDrawScreen(this->handle_);
		if (clear) {
			ClearDrawScreen();
		}
	}
	void SetDraw_Screen(const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup, const float& fov, const float& near_, const float& far_) {
		SetDraw_Screen(true);
		SetCameraNearFar(near_, far_);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
	}
	//
	static void SetDraw_Screen(const int& handle, const bool& clear = true) {
		SetDrawScreen(handle);
		if (clear) {
			ClearDrawScreen();
		}
	}
	static void SetDraw_Screen(const int& handle, const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup, const float& fov, const float& near_, const float& far_) {
		SetDraw_Screen(handle);
		SetCameraNearFar(near_, far_);
		SetupCamera_Perspective(fov);
		SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
	}
};
