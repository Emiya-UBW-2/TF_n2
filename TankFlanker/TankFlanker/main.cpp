#define NOMINMAX
#include "sub.hpp"
#include "useful.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	auto parts = std::make_unique<MainClass>(); /*�ėp�N���X*/

	DrawPixel(320, 240, GetColor(255, 255, 255)); // �_��ł�

	WaitKey(); // �L�[���͑҂�

	return 0; // �\�t�g�̏I��
}
