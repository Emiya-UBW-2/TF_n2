#pragma once
#include "DXLib_ref/DXLib_ref.h"

class HostPassEffect {
private:
	GraphHandle FarScreen;
	GraphHandle MainScreen;
	GraphHandle NearScreen;
	GraphHandle GaussScreen;
	const int EXTEND = 4;
	bool dof_flag = true;
	bool bloom_flag = true;
	int disp_x = 1920;
	int disp_y = 1080;
public:
	HostPassEffect(const bool& dof_, const bool& bloom_, const int& xd, const int& yd) {
		disp_x = xd;
		disp_y = yd;
		dof_flag = dof_;
		bloom_flag = bloom_;
		FarScreen = GraphHandle::Make(disp_x, disp_y, true);
		MainScreen = GraphHandle::Make(disp_x, disp_y, true);
		NearScreen = GraphHandle::Make(disp_x, disp_y, true);
		GaussScreen = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND); /*�G�t�F�N�g*/
	}
	~HostPassEffect() {
	}
	//��ʑ̐[�x�`��
	template <typename T2>
	void dof(
		GraphHandle* buf, GraphHandle& skyhandle, T2 doing,
		const VECTOR_ref& campos, const VECTOR_ref& camvec, const VECTOR_ref& camup, const float& fov,
		const float& far_distance = 1000.f, const float& near_distance = 100.f) {
		if (dof_flag) {
			//
			FarScreen.SetDraw_Screen(campos, camvec, camup,fov, far_distance, 30000.f);
			skyhandle.DrawGraph(0, 0, FALSE);
			doing();
			//
			MainScreen.SetDraw_Screen(campos, camvec, camup,fov, near_distance, far_distance + 50.f);
			Effekseer_Sync3DSetting();
			GraphFilter(FarScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
			FarScreen.DrawGraph(0, 0, false);
			UpdateEffekseer3D();
			doing();
			DrawEffekseer3D();
			//
			NearScreen.SetDraw_Screen(campos, camvec, camup,fov, 1.f, near_distance + 1.f);
			MainScreen.DrawGraph(0, 0, false);
			doing();
		}
		else {
			//
			NearScreen.SetDraw_Screen(campos, camvec, camup,fov, std::clamp(near_distance, 0.1f, 2000.f), 2000.f);
			Effekseer_Sync3DSetting();
			skyhandle.DrawGraph(0, 0, FALSE);
			UpdateEffekseer3D(); //2.0ms
			doing();
			DrawEffekseer3D();
		}
		buf->SetDraw_Screen();
		NearScreen.DrawGraph(0, 0, false);
	}
	//�u���[���G�t�F�N�g
	void bloom(GraphHandle& BufScreen, const int& level = 255) {
		if (bloom_flag) {
			GraphFilter(BufScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 245, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
			GraphFilterBlt(BufScreen.get(), GaussScreen.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
			GraphFilter(GaussScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
			SetDrawMode(DX_DRAWMODE_BILINEAR);
			SetDrawBlendMode(DX_BLENDMODE_ADD, level);
			GaussScreen.DrawExtendGraph(0, 0, disp_x, disp_y, false);
			GaussScreen.DrawExtendGraph(0, 0, disp_x, disp_y, false);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}

public:
};
