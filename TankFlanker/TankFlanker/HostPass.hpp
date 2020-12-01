#pragma once
#include "DXLib_ref/DXLib_ref.h"

class HostPassEffect {
private:
	GraphHandle FarScreen_;		//描画スクリーン
	GraphHandle NearFarScreen_;	//描画スクリーン
	GraphHandle NearScreen_;	//描画スクリーン
	GraphHandle GaussScreen_;	//描画スクリーン
	GraphHandle BufScreen;		//描画スクリーン

	int EXTEND = 4;
	bool dof_flag = true;
	bool bloom_flag = true;
	int disp_x = 1920;
	int disp_y = 1080;
public:
	GraphHandle MAIN_Screen;	//描画スクリーン
	GraphHandle SkyScreen;		//空描画
	GraphHandle UI_Screen;		//描画スクリーン

	HostPassEffect(const bool& dof_, const bool& bloom_, const int& xd, const int& yd) {
		disp_x = xd;
		disp_y = yd;
		dof_flag = dof_;
		bloom_flag = bloom_;

		FarScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		NearFarScreen_ = GraphHandle::Make(disp_x, disp_y, true);					//描画スクリーン
		NearScreen_ = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		GaussScreen_ = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND, true);	//描画スクリーン
		SkyScreen = GraphHandle::Make(disp_x, disp_y);								//空描画
		BufScreen = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		UI_Screen = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
		MAIN_Screen = GraphHandle::Make(disp_x, disp_y, true);						//描画スクリーン
	}
	~HostPassEffect() {
	}
	//被写体深度描画
	template <typename T2>
	void dof(T2 doing, DXDraw::cam_info& cams,bool update_ef=true) {
		if (dof_flag) {
			//
			FarScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.far_, 1000000.f);
			SkyScreen.DrawGraph(0, 0, FALSE);
			doing();
			//
			NearFarScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.near_, cams.far_ + 50.f);
			Effekseer_Sync3DSetting();
			GraphFilter(FarScreen_.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
			FarScreen_.DrawGraph(0, 0, false);
			if (update_ef) {
				UpdateEffekseer3D();
			}
			doing();
			DrawEffekseer3D();
			//
			NearScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, 0.5f, cams.near_ + 1.f);
			NearFarScreen_.DrawGraph(0, 0, false);
			doing();
		}
		else {
			//
			NearScreen_.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, std::clamp(cams.near_, 0.1f, 2000.f), 2000.f);
			Effekseer_Sync3DSetting();
			SkyScreen.DrawGraph(0, 0, FALSE);
			UpdateEffekseer3D(); //2.0ms
			doing();
			DrawEffekseer3D();
		}
		BufScreen.SetDraw_Screen();
		NearScreen_.DrawGraph(0, 0, false);
	}
	//ブルームエフェクト
	void bloom(const int& level = 255) {
		BufScreen.DrawGraph(0, 0, false);
		if (bloom_flag) {
			GraphFilter(BufScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 245, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
			GraphFilterBlt(BufScreen.get(), GaussScreen_.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
			GraphFilter(GaussScreen_.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
			SetDrawMode(DX_DRAWMODE_BILINEAR);
			SetDrawBlendMode(DX_BLENDMODE_ADD, level);
			int x=0, y=0;
			GetScreenState(&x, &y, nullptr);
			GaussScreen_.DrawExtendGraph(0, 0, x, y, false);
			GaussScreen_.DrawExtendGraph(0, 0, x, y, false);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}

public:
};
