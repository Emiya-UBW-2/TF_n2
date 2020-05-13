#pragma once
#include "DxLib.h"
#include"EffekseerForDXLib.h"

#include"DXLib_mat.hpp"

#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"

#include "useful.hpp"

#include <array>
#include <list>
#include <vector>
enum Effect {
	ef_fire = 0, //���C��
	ef_reco = 1, //����a���e
	ef_reco2 = 2, //�����a���e
	ef_gndhit = 3, //����a���e
	ef_gndhit2 = 4, //�����a���e
	ef_bomb = 5, //���j����
	ef_smoke1 = 6, //�~�T�C����
	ef_smoke2 = 7, //�e�̋O��
	effects = 8, //�ǂݍ���
	efs_user = 8  //
};
struct EffectS {
	bool flug{ false };		 /**/
	size_t id = 0;
	Effekseer3DPlayingHandle handle; /**/
	VECTOR_ref pos;			 /**/
	VECTOR_ref nor;			 /**/
	float scale = 1.f;		 /**/
};

class DXDraw {
private:
	const bool use_shadow = true;			     /*�e�`��*/
	int shadow_near = 0;			     /*�߉e*/
	int shadow_far = 0;			     /*���e*/
	bool use_pixellighting = false;			     /**/
	bool use_vsync = false;				     /*��������*/
	float frate = 60.f;				     /*�t���[�����[�g*/
	std::array<EffekseerEffectHandle, effects> effHndle; /*�G�t�F�N�g���\�[�X*/
	EffekseerEffectHandle gndsmkHndle;		     /*�G�t�F�N�g���\�[�X*/
	int disp_x = 1920;
	int disp_y = 1080;
public:
	EffekseerEffectHandle& get_effHandle(int p1) noexcept { return effHndle[p1]; }
	const EffekseerEffectHandle& get_effHandle(int p1) const noexcept { return effHndle[p1]; }
	EffekseerEffectHandle& get_gndhitHandle() noexcept { return gndsmkHndle; }
	const EffekseerEffectHandle& get_gndhitHandle() const noexcept { return gndsmkHndle; }

	DXDraw(const char* title, const int& xd, const int& yd, const float& fps = 60.f) {
		disp_x = xd;
		disp_y = yd;

		frate = fps;
		SetOutApplicationLogValidFlag(FALSE);		       /*log*/
		SetMainWindowText(title);			       /*�^�C�g��*/
		ChangeWindowMode(TRUE);				       /*���\��*/
		SetUseDirect3DVersion(DX_DIRECT3D_11);		       /*directX ver*/
		SetGraphMode(disp_x, disp_y, 32);		       /*�𑜓x*/
		SetUseDirectInputFlag(TRUE);			       /**/
		SetDirectInputMouseMode(TRUE);			       /**/
		SetWindowSizeChangeEnableFlag(FALSE, FALSE);	       /*�E�C���h�E�T�C�Y���蓮�s�A�E�C���h�E�T�C�Y�ɍ��킹�Ċg������Ȃ��悤�ɂ���*/
		SetUsePixelLighting(use_pixellighting ? TRUE : FALSE); /*�s�N�Z���V�F�[�_�̎g�p*/
		//SetFullSceneAntiAliasingMode(4, 2);		       /*�A���`�G�C���A�X*/
		SetWaitVSyncFlag(use_vsync ? TRUE : FALSE);	       /*��������*/
		DxLib_Init();					       /**/
		Effekseer_Init(8000);				       /*Effekseer*/
		SetChangeScreenModeGraphicsSystemResetFlag(FALSE);     /*Effekseer*/
		Effekseer_SetGraphicsDeviceLostCallbackFunctions();    /*Effekseer*/
		SetAlwaysRunFlag(TRUE);				       /*background*/
		SetUseZBuffer3D(TRUE);				       /*zbufuse*/
		SetWriteZBuffer3D(TRUE);			       /*zbufwrite*/
		SetDrawMode(DX_DRAWMODE_BILINEAR);		       /**/
		//�G�t�F�N�g
		{
			size_t j = 0;
			for (auto& e : effHndle)
				e = EffekseerEffectHandle::load("data/effect/" + std::to_string(j++) + ".efk");
			gndsmkHndle = EffekseerEffectHandle::load("data/effect/gndsmk.efk");
		}

	}
	~DXDraw(void) {
		Effkseer_End();
		DxLib_End();
	}
	template <typename T>
	bool Set_Shadow(const size_t& scale, const VECTOR_ref& farsize, const VECTOR_ref& Light_dir, T doing) {
		shadow_near = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		shadow_far = MakeShadowMap(int(pow(2, scale)), int(pow(2, scale)));
		SetShadowMapAdjustDepth(shadow_near, 0.0005f);
		SetShadowMapLightDirection(shadow_near, Light_dir.get());
		SetShadowMapLightDirection(shadow_far, Light_dir.get());
		SetShadowMapDrawArea(shadow_far, (farsize*-1.f).get(), farsize.get());
		ShadowMap_DrawSetup(shadow_far);
		doing();
		ShadowMap_DrawEnd();
		return true;
	}
	bool Delete_Shadow() {
		DeleteShadowMap(shadow_near);
		DeleteShadowMap(shadow_far);
		return true;
	}

	bool Set_light(const VECTOR_ref& Light_dir) {
		SetGlobalAmbientLight(GetColorF(0.80f, 0.75f, 0.70f, 0.0f));
		SetLightDirection(Light_dir.get());
		return true;
	}


	template <typename T>
	bool Ready_Shadow(const VECTOR_ref& pos, T doing, const VECTOR_ref& nearsize) {
		if (use_shadow) {
			SetShadowMapDrawArea(shadow_near, (nearsize*(-1.f) + pos).get(), (VECTOR_ref(nearsize) + pos).get());
			ShadowMap_DrawSetup(shadow_near);
			doing();
			ShadowMap_DrawEnd();
			return true;
		}
		return false;
	}
	template <typename T>
	bool Draw_by_Shadow(T doing) {
		if (use_shadow) {
			SetUseShadowMap(0, shadow_near);
			SetUseShadowMap(1, shadow_far);
		}
		doing();
		if (use_shadow) {
			SetUseShadowMap(0, -1);
			SetUseShadowMap(1, -1);
		}
		return true;
	}
	bool Screen_Flip(const LONGLONG& waits) {
		/*
		{
			int i = 0;
			for (auto& c : colors) {
				DrawFormatString(200, i * 20, c.col, "%06x", c.buf);
				i++;
			}
		}
		*/
		Screen_Flip();
		if (!use_vsync) {
			while (GetNowHiPerformanceCount() - waits < 1000000.0f / frate) {}
		}
		return true;
	}
	bool Screen_Flip(void) {
		ScreenFlip();
		return true;
	}


	static bool Capsule3D(const VECTOR_ref& p1, const VECTOR_ref& p2, const float& range, const unsigned int& color, const unsigned int& speccolor) {
		return DxLib::DrawCapsule3D(p1.get(), p2.get(), range, 8, color, speccolor, TRUE) == TRUE;
	}

	static bool Line2D(const int& p1x, const int& p1y, const int& p2x, const int& p2y, const unsigned int& color, const int& thickness=1) {
		return DxLib::DrawLine(p1x,p1y,p2x,p2y,color,thickness) == TRUE;
	}

};
