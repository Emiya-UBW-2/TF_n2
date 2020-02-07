#define NOMINMAX
#include "sub.hpp"
#include "useful.hpp"
#include "make_thread.hpp"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	input key{ 0 };
	output out{ 0 };
	switches aim, map, vch; /*���_�ύX*/


	auto threadparts = std::make_unique<ThreadClass>(); /*�ėp�N���X*/
	auto parts = std::make_unique<MainClass>(); /*�ėp�N���X*/

	const auto font72 = FontHandle::Create(x_r(72), y_r(72 / 3), DX_FONTTYPE_ANTIALIASING);

	do {
		aim.flug = false; /*�Ə�*/
		map.flug = false; /*�}�b�v*/
		vch.flug = false; /**/
		threadparts->thread_start(key, out);
		while (ProcessMessage() == 0 && !out.ends) {
			const auto waits = GetNowHiPerformanceCount();
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();

			font72.DrawStringFormat(0, 0, GetColor(255, 255, 255), "%d", out.y);

			parts->Screen_Flip(waits);

			if (GetActiveFlag() == TRUE) {
				SetMouseDispFlag(FALSE);
				key.get2[0] = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
				key.get2[1] = (GetMouseInput() & MOUSE_INPUT_RIGHT) != 0;
				key.get2[2] = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
				key.get2[3] = CheckHitKey(KEY_INPUT_P) != 0;
				if (true) {
					//�w��
					key.get[0] = CheckHitKey(KEY_INPUT_RSHIFT) != 0;
					//�Ə�
					key.get[1] = CheckHitKey(KEY_INPUT_LSHIFT) != 0;
					key.get[2] = CheckHitKey(KEY_INPUT_V) != 0;
					key.get[3] = CheckHitKey(KEY_INPUT_X) != 0;
					key.get[4] = CheckHitKey(KEY_INPUT_C) != 0;
					//���Ă�
					key.get[5] = CheckHitKey(KEY_INPUT_Q) != 0;
					key.get[6] = CheckHitKey(KEY_INPUT_E) != 0;
					//���c
					key.get[7] = CheckHitKey(KEY_INPUT_W) != 0;
					key.get[8] = CheckHitKey(KEY_INPUT_S) != 0;
					key.get[9] = CheckHitKey(KEY_INPUT_A) != 0;
					key.get[10] = CheckHitKey(KEY_INPUT_D) != 0;
					//�C��
					key.get[11] = CheckHitKey(KEY_INPUT_LEFT) != 0;
					key.get[12] = CheckHitKey(KEY_INPUT_RIGHT) != 0;
					key.get[13] = CheckHitKey(KEY_INPUT_UP) != 0;
					key.get[14] = CheckHitKey(KEY_INPUT_DOWN) != 0;
					key.get[15] = CheckHitKey(KEY_INPUT_LCONTROL) != 0;
					//�ˌ�
					key.get[16] = CheckHitKey(KEY_INPUT_SPACE) != 0;
					key.get[17] = CheckHitKey(KEY_INPUT_B) != 0;
					//���_
					key.get[18] = CheckHitKey(KEY_INPUT_RCONTROL) != 0;
				}
				/*�w��*/
				if (map.flug)
					SetMouseDispFlag(TRUE);
			}
			else {
				SetMouseDispFlag(TRUE);
			}
		}
		threadparts->thead_stop();
	} while (ProcessMessage() == 0 && !out.ends);

	return 0; // �\�t�g�̏I��
}
