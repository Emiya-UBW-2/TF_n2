#pragma once
#include <string>
#include <string_view>
#include "DXLib_ref.h"

using std::int8_t;
using std::size_t;
using std::uint16_t;
using std::uint8_t;

constexpr float M_GR = -9.8f;				  /*重力加速度*/

inline const int deskx = (GetSystemMetrics(SM_CXSCREEN)); /*デスクトップX*/
inline const int desky = (GetSystemMetrics(SM_CYSCREEN)); /*デスクトップY*/

//マウス判定
#define in2_(mx, my, x1, y1, x2, y2) (mx >= x1 && mx <= x2 && my >= y1 && my <= y2)
#define in2_mouse(x1, y1, x2, y2) (in2_(mousex, mousey, x1, y1, x2, y2))
//その他
template <typename T>
static float deg2rad(T p1) {
	return float(p1) * DX_PI_F / 180.f;
} //角度からラジアンに
template <typename T>
static float rad2deg(T p1) {
	return float(p1) * 180.f / DX_PI_F;
} //ラジアンから角度に
//
static std::string getright(const char* p1) {
	std::string tempname = p1;
	return tempname.substr(tempname.find('=') + 1);
}
//
class getparams {
public:
	static const char* _char(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return getright(mstr).c_str();
	}
	static auto _str(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return getright(mstr);
	}
	static auto get_str(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return std::string(mstr);
	}
	static const long int _int(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return std::stoi(getright(mstr));
	}
	static const long int _long(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return std::stol(getright(mstr));
	}
	static const unsigned long int _ulong(int p2) {
		char mstr[64];
		FileRead_gets(mstr, 64, p2);
		return std::stoul(getright(mstr));
	}
	static const float _float(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return std::stof(getright(mstr));
	}
	static const bool _bool(int p1) {
		char mstr[64];
		FileRead_gets(mstr, 64, p1);
		return (getright(mstr).find("true") != std::string::npos);
	}
};
//
void easing_set(float* first, const float& aim, const float& ratio, const float& fps) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		if (aim != 0.f) {
			*first += (aim - *first) * (1.f - powf(ratio, 60.f / fps));
		}
		else {
			*first *= powf(ratio, 60.f / fps);
		}
	}
};
//
void easing_set(VECTOR_ref* first, const VECTOR_ref& aim, const float& ratio, const float& fps) {
	if (ratio == 0.f) {
		*first = aim;
	}
	else {
		*first += (VECTOR_ref(aim) - *first) * (1.f - powf(ratio, 60.f / fps));
	}
};

//ID割り当て
template <class T>
void fill_id(std::vector<T>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
template <class T, size_t N>
void fill_id(std::array<T, N>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
//cosAを出す
float getcos_tri(const float& a, const float& b, const float& c) {
	if (b + c > a && c + a > b && a + b > c) {
		return std::clamp((b * b + c * c - a * a) / (2.f * b*c), -1.f, 1.f);
	}
	return 1.f;
}
//起動
void createProcess(char* szCmd, DWORD flag, bool fWait) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(STARTUPINFO));
	memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	CreateProcess(NULL, szCmd, NULL, NULL, FALSE, flag, NULL, NULL, &si, &pi);
	if (fWait) WaitForSingleObject(pi.hProcess, INFINITE);	//終了を待つ.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}
//自信を多重起動
void start_me(void) {
	char Path[MAX_PATH];
	// EXEのあるフォルダのパスを取得
	::GetModuleFileName(NULL, Path, MAX_PATH);
	createProcess(Path, SW_HIDE, false);
}
//
bool Hit_Capsule_Tri(VECTOR_ref startpos, VECTOR_ref endpos, float size, VECTOR_ref tri_p1, VECTOR_ref tri_p2, VECTOR_ref tri_p3) {
	return HitCheck_Capsule_Triangle(startpos.get(), endpos.get(), size, tri_p1.get(), tri_p2.get(), tri_p3.get()) == TRUE;
}
//