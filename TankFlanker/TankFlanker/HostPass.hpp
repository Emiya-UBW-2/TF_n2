#pragma once
#include "DXLib_ref/DXLib_ref.h"

class HostPassEffect {
private:
	//GraphHandle FarScreen;
	//GraphHandle MainScreen;
	//GraphHandle NearScreen;
	//GraphHandle GaussScreen;

	GraphHandle *FarScreen_ptr;
	GraphHandle *NearFarScreen_ptr;
	GraphHandle *NearScreen_ptr;
	GraphHandle *GaussScreen_ptr;

	int EXTEND = 4;
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
		//FarScreen = GraphHandle::Make(disp_x, disp_y, true);
		//MainScreen = GraphHandle::Make(disp_x, disp_y, true);
		//NearScreen = GraphHandle::Make(disp_x, disp_y, true);
		//GaussScreen = GraphHandle::Make(disp_x / EXTEND, disp_y / EXTEND); /*エフェクト*/
	}
	~HostPassEffect() {
	}
	//被写体深度描画
	template <typename T2>
	void dof(
		GraphHandle& buf, GraphHandle& skyhandle, T2 doing,
		DXDraw::cam_info& cams, 
		GraphHandle& farsc, GraphHandle& nearfarsc, GraphHandle& nearsc
	) {
		if (dof_flag) {
			//
			farsc.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.far_, 1000000.f);
			skyhandle.DrawGraph(0, 0, FALSE);
			doing();
			//
			nearfarsc.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, cams.near_, cams.far_ + 50.f);
			Effekseer_Sync3DSetting();
			GraphFilter(farsc.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
			farsc.DrawGraph(0, 0, false);
			UpdateEffekseer3D();
			doing();
			DrawEffekseer3D();
			//
			nearsc.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, 0.5f, cams.near_ + 1.f);
			nearfarsc.DrawGraph(0, 0, false);
			doing();
		}
		else {
			//
			nearsc.SetDraw_Screen(cams.campos, cams.camvec, cams.camup, cams.fov, std::clamp(cams.near_, 0.1f, 2000.f), 2000.f);
			Effekseer_Sync3DSetting();
			skyhandle.DrawGraph(0, 0, FALSE);
			UpdateEffekseer3D(); //2.0ms
			doing();
			DrawEffekseer3D();
		}
		buf.SetDraw_Screen();
		nearsc.DrawGraph(0, 0, false);
	}
	//ブルームエフェクト
	void bloom(GraphHandle& BufScreen, GraphHandle& gausssc, const int& EXTE, const int& level = 255) {
		if (bloom_flag) {
			GraphFilter(BufScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 245, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
			GraphFilterBlt(BufScreen.get(), gausssc.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTE);
			GraphFilter(gausssc.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
			SetDrawMode(DX_DRAWMODE_BILINEAR);
			SetDrawBlendMode(DX_BLENDMODE_ADD, level);
			int x=0, y=0;
			GetScreenState(&x, &y, nullptr);
			gausssc.DrawExtendGraph(0, 0, x, y, false);
			gausssc.DrawExtendGraph(0, 0, x, y, false);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}

public:
};
