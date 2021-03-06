#pragma once

//リサイズ
#define x_r(p1) (int(p1) * deskx / 1920)
#define y_r(p1) (int(p1) * desky / 1080)

class UI {
private:
	//
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle aim_if;
	GraphHandle dmg;
	GraphHandle hit;
	//
	float ber = 0;
	//font
	FontHandle font36;
	FontHandle font24;
	FontHandle font18;
	FontHandle font12;
	//
	int out_disp_x = deskx;
	int out_disp_y = desky;
	int disp_x = deskx;
	int disp_y = desky;
	//
	VECTOR_ref aimpos;
	VECTOR_ref aimpos_2;
	//
	size_t id_near = 0;
public:
	UI(int d_x, int d_y) {
		disp_x = d_x;
		disp_y = d_y;
		out_disp_x = deskx;
		out_disp_y = desky;

		circle = GraphHandle::Load("data/UI/battle_circle.bmp");
		aim = GraphHandle::Load("data/UI/battle_aim.bmp");
		aim_if = GraphHandle::Load("data/UI/battle_if.bmp");
		dmg = GraphHandle::Load("data/UI/damage.png");
		hit = GraphHandle::Load("data/UI/battle_hit.bmp");


		font36 = FontHandle::Create(y_r(36), DX_FONTTYPE_EDGE);
		font24 = FontHandle::Create(y_r(24), DX_FONTTYPE_EDGE);
		font18 = FontHandle::Create(y_r(18), DX_FONTTYPE_EDGE);
		font12 = FontHandle::Create(y_r(12), DX_FONTTYPE_EDGE);
	}
	~UI() {
	}
	void load_window(const char* mes) {
		SetUseASyncLoadFlag(FALSE);
		float bar = 0.f, cnt = 0.f;
		auto all = GetASyncLoadNum();
		while (ProcessMessage() == 0) {
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			{
				font18.DrawStringFormat(0, out_disp_y - y_r(70), GetColor(0, 255, 0), " loading... : %04d/%04d  ", all - GetASyncLoadNum(), all);
				font12.DrawStringFormat(out_disp_x - font12.GetDrawWidthFormat("%s 読み込み中 ", mes), out_disp_y - y_r(70), GetColor(0, 255, 0), "%s 読み込み中 ", mes);
				DrawBox(0, out_disp_y - y_r(50), int(float(out_disp_x) * bar / float(all)), out_disp_y - y_r(40), GetColor(0, 255, 0), TRUE);
				easing_set(&bar, float(all - GetASyncLoadNum()), 0.95f);
			}
			ScreenFlip();
			if (GetASyncLoadNum() == 0) {
				cnt += 1.f / GetFPS();
				if (cnt > 1 && bar > float(all - GetASyncLoadNum()) * 0.95f) {
					break;
				}
			}
		}
	}
	//
	void draw_menu(float ber_r, float rad, Mainclass::Vehcs& vehs) {
		//
		{
			int xp = disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
			int yp = disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(90)));
			int xa = disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(90)));
			int ya = disp_y * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(90)));
			DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
			{
				DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 0, 0), TRUE);
				font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3), GetColor(0, 255, 0), "Name     :%s", vehs.name.c_str());
				font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 20), GetColor(0, 255, 0), "MaxSpeed :%03.0f km/h", vehs.max_speed_limit*3.6f);
				font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 40), GetColor(0, 255, 0), "MidSpeed :%03.0f km/h", vehs.mid_speed_limit*3.6f);
				font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 60), GetColor(0, 255, 0), "MinSpeed :%03.0f km/h", vehs.min_speed_limit*3.6f);
				font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(60 - 3 - 80), GetColor(0, 255, 0), "Turn     :%03.0f °/s", vehs.body_rad_limit);
				DrawBox(xp - y_r(120), yp - y_r(60), xp + y_r(120), yp + y_r(60), GetColor(0, 255, 0), FALSE);
				font12.DrawString(xp - y_r(120), yp - y_r(60 + 15), "Spec", GetColor(0, 255, 0));
			}
		}
		//
		{
			int xp = disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
			int yp = disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
			int xa = disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150)) * sin(rad + deg2rad(180)));
			int ya = disp_y * 2 / 3 + int((ber_r - y_r(150)) * cos(rad + deg2rad(180)));
			DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
			{
				int ys = 20 * int(vehs.gunframe.size()) / 2 + 1;
				DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 0, 0), TRUE);
				if (vehs.gunframe.size() == 0) {
					font18.DrawString(xp - y_r(120 - 3), yp - y_r(ys - 3), "N/A", GetColor(0, 255, 0));
				}
				else {
					for (int z = 0; z < vehs.gunframe.size(); z++) {
						font18.DrawStringFormat(xp - y_r(120 - 3), yp - y_r(ys - 3 - 20 * z), GetColor(0, 255, 0), "No.%d  :%s", z, vehs.gunframe[z].name.c_str());
					}
				}
				DrawBox(xp - y_r(120), yp - y_r(ys), xp + y_r(120), yp + y_r(ys), GetColor(0, 255, 0), FALSE);
				font12.DrawString(xp - y_r(120), yp - y_r(ys + 15), "Weapon", GetColor(0, 255, 0));
			}
		}
	}
	//
	void draw_edge(Mainclass::Chara& chara, bool uses_vr = true) {
		int xs = 0, xp = 0, ys = 0, yp = 0;
		auto font_hight = (!uses_vr) ? y_r(18) : y_r(24);
		//死亡時に非表示
		if (!chara.death) {
			//速度計
			{
				if (!uses_vr) {
					xs = 0;
					xp = disp_x / 2 - disp_y / 7;
					ys = disp_y / 14;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 - disp_y / 6 + y_r(120);
					ys = disp_y / 6 - y_r(120);
					yp = disp_y / 2;
				}
				//
				DXDraw::Line2D(xp + y_r(1), yp, xp + y_r(10), yp, GetColor(100, 255, 100), 5);
				//
				DXDraw::Line2D(xp - y_r(17), yp, xp - y_r(25), yp - font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp - y_r(17), yp, xp - y_r(25), yp + font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp - y_r(25), yp - font_hight / 2, xp - y_r(35), yp - font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp - y_r(25), yp + font_hight / 2, xp - y_r(35), yp + font_hight / 2, GetColor(100, 255, 100), 5);
				//
				for (int i = -ys; i < ys; i += (y_r(10))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(10));
					if (p <= ys) {
						DXDraw::Line2D(xp - y_r(10), yp + p, xp, yp + p, GetColor(100, 255, 100), 5);
					}
					else {
						break;
					}
				}
				//
				for (int i = -ys; i < ys; i += (y_r(100))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(100));
					if (p <= ys) {
						DXDraw::Line2D(xp - y_r(15), yp + p, xp, yp + p, GetColor(100, 255, 100), 6);
					}
					else {
						break;
					}
				}
			}
			//高度計
			{
				if (!uses_vr) {
					xs = 0;
					xp = disp_x / 2 + disp_y / 7;
					ys = disp_y / 14;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 + disp_y / 6 - y_r(120);
					ys = disp_y / 6 - y_r(120);
					yp = disp_y / 2;
				}
				//
				DXDraw::Line2D(xp - y_r(10), yp, xp - y_r(1), yp, GetColor(100, 255, 100), 5);
				//
				DXDraw::Line2D(xp + y_r(17), yp, xp + y_r(25), yp - font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp + y_r(17), yp, xp + y_r(25), yp + font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp + y_r(25), yp - font_hight / 2, xp + y_r(35), yp - font_hight / 2, GetColor(100, 255, 100), 5);
				DXDraw::Line2D(xp + y_r(25), yp + font_hight / 2, xp + y_r(35), yp + font_hight / 2, GetColor(100, 255, 100), 5);
				//
				for (int i = -ys; i < ys; i += y_r(10)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(10);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + y_r(10), yp + p, GetColor(100, 255, 100), 5);
					}
					else {
						break;
					}
				}
				//
				for (int i = -ys; i < ys; i += y_r(100)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(100);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + y_r(15), yp + p, GetColor(100, 255, 100), 6);
					}
					else {
						break;
					}
				}
			}
			//コンパス
			{
				VECTOR_ref tmp = chara.vehicle.mat.zvec();
				tmp = VGet(tmp.x(), 0.f, tmp.z());
				tmp = tmp.Norm();
				float rad = -std::atan2f(tmp.x(), -tmp.z())*180.f / DX_PI_F;
				rad = (rad < 0) ? (360.f + rad) : rad;
				if (!uses_vr) {
					xs = disp_y / 14;
					xp = disp_x / 2;
					ys = 0;
					yp = disp_y / 2 + disp_y / 7;
				}
				else {
					xs = disp_y / 6 - y_r(120);
					xp = disp_x / 2;
					ys = 0;
					yp = disp_y / 2 + disp_y / 6 - y_r(120);
				}
				//
				DXDraw::Line2D(xp, yp - y_r(10), xp, yp - y_r(1), GetColor(100, 255, 100), 5);
				//
				for (int i = -xs; i < xs; i += y_r(10)) {
					int p = (i + int(rad + xs) % y_r(10));
					if (p <= xs) {
						DXDraw::Line2D(xp + p, yp, xp + p, yp + y_r(10), GetColor(100, 255, 100), 5);
					}
					else {
						break;
					}
				}
				//
				for (int i = -xs; i < xs; i += y_r(60)) {
					int p = (i + int(rad + xs) % y_r(60));
					if (p <= xs) {
						DXDraw::Line2D(xp + p, yp, xp + p, yp + y_r(15), GetColor(100, 255, 100), 6);
					}
					else {
						break;
					}
				}
			}
		}
	}
	//
	void draw(Mainclass::Chara& chara, const bool& adss, const DXDraw::system_VR& vr_sys, float danger_height, bool uses_vr = true) {
		int xs = 0, xp = 0, ys = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font18 : &font24;
		auto font_hight = (!uses_vr) ? y_r(18) : y_r(24);
		auto& veh = chara.vehicle;
		auto time = GetNowHiPerformanceCount();
		//サイズ変更
		if (uses_vr) {
			GetScreenState(&disp_x, &disp_y, nullptr);
		}
		//HP表示
		{
			if (uses_vr) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(1.f - (float(veh.HP) / veh.use_veh.HP))*0.6f), 0, 255));

				DrawBox(0, 0, disp_x, disp_y, GetColor(128, 0, 0), TRUE);
			}
			else {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(1.f - float(veh.HP) / veh.use_veh.HP)), 0, 255));
				dmg.DrawExtendGraph(0, 0, disp_x, disp_y, true);
			}
			//ダメージ
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(float(int(veh.HP_r) - veh.HP) / float(veh.use_veh.HP))), 0, 255));
			DrawBox(0, 0, disp_x, disp_y, GetColor(128, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
		//死亡時に非表示
		if (!chara.death) {
			//弾薬、残弾計
			{
				if (!uses_vr) {
					xs = x_r(200);
					xp = x_r(20);
					ys = font_hight;
					yp = disp_y - y_r(20) - int(veh.Gun_.size()) * (ys * 2 + y_r(7));
				}
				else {
					xs = x_r(200);
					xp = disp_x / 2 - x_r(20) - xs;
					ys = font_hight;
					yp = disp_y / 2 + disp_y / 6 + y_r(20);
				}

				for (auto& g : veh.Gun_) {
					size_t i = (&g - &veh.Gun_[0]);
					if (i == 0 || i == (veh.sel_weapon + 1)) {
						if (g.get_loadcnt() != 0.f) {
							DrawBox(xp, yp + ys * 2 / 3, xp + (xs - int(float(xs) * g.get_loadcnt() / g.get_gun_info().load_time)), yp + ys, GetColor(255, 0, 0), TRUE);
						}
						else {
							DrawBox(xp, yp + ys * 2 / 3, xp + xs, yp + ys, GetColor(0, 255, 0), TRUE);
						}

						if (g.get_rounds_() != 0.f) {
							DrawBox(xp, yp + ys * 2 - y_r(2), xp + x_r(int(float(xs) * g.get_rounds_() / g.get_gun_info().rounds)), yp + ys * 2 + y_r(2), GetColor(255, 192, 0), TRUE);
						}

						font->DrawString(xp, yp - ys, g.get_gun_info().name, GetColor(255, 255, 255));
						font->DrawString(xp, yp, g.getbullet_use().spec.name_a, GetColor(255, 255, 255));
						font->DrawStringFormat_RIGHT(xp + xs, yp + ys + y_r(2), GetColor(255, 255, 255), "%04d / %04d", g.get_rounds_(), g.get_gun_info().rounds);

						xp += -x_r(30 / int(veh.Gun_.size()));
						yp += ys * 3 + y_r(4);
					}
				}
			}
			//アラート
			if (!adss) {
				if (!uses_vr) {
					xp = disp_x / 2;
					yp = disp_y / 3;
				}
				else {
					xp = disp_x / 2;
					yp = disp_y / 2 - disp_y / 6 + y_r(60);
				}

				int ccc = 0;
				if (veh.speed < veh.use_veh.min_speed_limit) {
					font->DrawString_MID(xp, yp + y_r(36) * ccc, "STALL", GetColor(255, 0, 0));
					ccc++;
				}
				if (veh.pos.y() <= danger_height) {
					font->DrawString_MID(xp, yp + y_r(36) * ccc, "GPWS", GetColor(255, 255, 0));
					ccc++;
				}
				if (chara.p_anime_geardown.second > 0.5f) {
					font->DrawString_MID(xp, yp + y_r(36) * ccc, "GEAR DOWN", GetColor(255, 255, 0));
					ccc++;
				}
			}
			//HP
			{
				if (!uses_vr) {
					xs = x_r(200);
					xp = disp_x - x_r(20 + 30) - xs;
					ys = y_r(42);
					yp = disp_y - y_r(20) - ys;
				}
				else {
					xs = x_r(200);
					xp = disp_x / 2 + x_r(20);
					ys = y_r(36);
					yp = disp_y / 2 + disp_y / 6 + y_r(20) - ys;
				}

				{
					auto per = 255;
					DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4)), xp + int(ber), yp + ys / 2 + ys * 2 / 3, GetColor(per, 0, 0), TRUE);
					easing_set(&ber, float(xs * int(veh.HP) / int(veh.use_veh.HP)), 0.975f);
					DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4)), xp + xs * int(veh.HP) / int(veh.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, per, 0), TRUE);
					SetDrawBright(per, per, per);
					font->DrawStringFormat(
						xp,
						yp + ys + (ys * 2 / 3 - y_r(12)) / 2,
						GetColor(255, 255, 255), "%d / %d", int(veh.HP), int(veh.use_veh.HP));

					font->DrawStringFormat(
						xp + (xs - font->GetDrawWidthFormat("%s", veh.use_veh.name.c_str())),
						yp + (ys * 2 / 3 - y_r(12)) / 2,
						GetColor(255, 255, 255), "%s", veh.use_veh.name.c_str());

					SetDrawBright(255, 255, 255);
					yp += ys;
					xp += x_r(30);
				}
			}
			//モジュールHP
			{
				if (!uses_vr) {
					xs = x_r(230);
					xp = disp_x - x_r(20 + 30) - xs;
					ys = y_r(304);
					yp = disp_y - y_r(20 + 50) - ys;
				}
				else {
					xs = x_r(230);
					xp = disp_x / 2 + x_r(20);
					ys = y_r(304);
					yp = disp_y / 2 + disp_y / 6 + y_r(20) - ys;
				}

				{
					auto tmp = deg2rad(90 * veh.HP / veh.use_veh.HP);
					SetDrawBright(std::min(int(255.f*cos(tmp)*2.f), 255), std::min(int(255.f*sin(tmp)*2.f), 255), 0);
					veh.graph_HP_m_all.DrawExtendGraph(xp, yp, xp + xs, yp + ys, true);
				}
				for (auto& p : veh.graph_HP_m) {
					auto tmp = deg2rad(90 * veh.HP_m[veh.use_veh.module_mesh[&p - &veh.graph_HP_m[0]].first] / veh.use_veh.HP);
					SetDrawBright(std::min(int(255.f*cos(tmp)*2.f), 255), std::min(int(255.f*sin(tmp)*2.f), 255), 0);
					p.DrawExtendGraph(xp, yp, xp + xs, yp + ys, true);
				}
				SetDrawBright(255, 255, 255);
			}
			//VR用オプション
			if (uses_vr && false) {
				const float vr_sys_yvec_y = vr_sys.yvec.y();
				const float vr_sys_yvec_x = vr_sys.yvec.x();
				const float vr_sys_touch_y = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.y() : 0.f;
				const float vr_sys_touch_x = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.x() : 0.f;
				//ピッチ、ロール表示
				{
					int size = y_r(10);
					ys = disp_y / 3 - y_r(240);
					xp = disp_x / 2 + ys / 2;
					yp = disp_y / 2 + ys / 2;


					for (int i = 0; i < 2; i++) {
						DrawCircle(xp, yp, size * 2, i ? GetColor(0, 255, 100) : GetColor(0, 0, 0), FALSE, 1 + i);
						DrawCircle(xp, yp, size, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), FALSE, 1 + i);
						DrawLine(xp - size / 2, yp, xp - size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp, yp - size / 2, xp, yp - size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp + size / 2, yp, xp + size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DrawLine(xp, yp + size / 2, xp, yp + size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
					}

					int y_pos = int(float(size) * std::clamp(vr_sys_yvec_y / sin(deg2rad(20)), -2.f, 2.f));//ピッチ
					int x_pos = int(float(size) * std::clamp(vr_sys_yvec_x / sin(deg2rad(20)), -2.f, 2.f));//ロール
					DrawCircle(xp + x_pos, yp + y_pos, size / 5, GetColor(255, 100, 50), FALSE, 2);
				}
				//ヨー
				{
					xs = disp_y / 5 - y_r(144);
					xp = disp_x / 2 - xs / 2;
					yp = disp_y / 2 + disp_y / 6 - y_r(220 / 2);
					int z_pos = int(float(xs / 4) * std::clamp(vr_sys_touch_x / 0.5f, -1.5f, 1.5f));//ヨー

					for (int i = 0; i < 2; i++) {
						DXDraw::Line2D(xp + xs / 2 - xs / 3, yp, xp + xs / 2 + xs / 3, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DXDraw::Line2D(xp + xs / 4, yp - 5, xp + xs / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						DXDraw::Line2D(xp + xs * 3 / 4, yp - 5, xp + xs * 3 / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
					}
					DXDraw::Line2D(xp + xs / 2 + z_pos, yp - 5, xp + xs / 2 + z_pos, yp + 5, GetColor(255, 100, 50), 2);
				}
			}
			//ピッチ
			/*
			{
				int ys = disp_y / 3 - y_r(240);
				int xp = disp_x / 2 - ys / 2;
				int yp = disp_y / 2 - ys / 2;
				int y_pos = int(float(ys / 4) * std::clamp(vr_sys_touch_y / 0.5f, -2.f, 2.f));

				DXDraw::Line2D(xp, yp, xp, yp + ys, GetColor(0, 0, 0), 5);
				DXDraw::Line2D(xp, yp + ys / 2 - (ys / 4), xp, yp + ys / 2 + (ys / 4), GetColor(255, 255, 255), 2);
				DXDraw::Line2D(xp, yp, xp, yp + ys / 2 - (ys / 4), GetColor(255, 0, 0), 2);
				DXDraw::Line2D(xp, yp + ys / 2 + (ys / 4), xp, yp + ys, GetColor(255, 0, 0), 2);

				DXDraw::Line2D(xp - 5, yp + ys / 2 + y_pos, xp + 5, yp + ys / 2 + y_pos, GetColor(255, 255, 0), 2);
				DXDraw::Line2D(xp - 5, yp + ys / 2 - (ys / 4), xp + 5, yp + ys / 2 - (ys / 4), GetColor(0, 255, 0), 2);
				DXDraw::Line2D(xp - 5, yp + ys / 2 + (ys / 4), xp + 5, yp + ys / 2 + (ys / 4), GetColor(0, 255, 0), 2);
			}
			*/
			//速度計
			{
				if (!uses_vr) {
					xs = 0;
					xp = disp_x / 2 - disp_y / 7;
					ys = disp_y / 14;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 - disp_y / 6 + y_r(120);
					ys = disp_y / 6 - y_r(120);
					yp = disp_y / 2;
				}
				//
				DXDraw::Line2D(xp + y_r(1), yp, xp + y_r(10), yp, GetColor(0, 255, 0), 1);
				//
				DXDraw::Line2D(xp - y_r(17), yp, xp - y_r(25), yp - font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp - y_r(17), yp, xp - y_r(25), yp + font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp - y_r(25), yp - font_hight / 2, xp - y_r(35), yp - font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp - y_r(25), yp + font_hight / 2, xp - y_r(35), yp + font_hight / 2, GetColor(0, 255, 0), 1);
				//
				for (int i = -ys; i < ys; i += (y_r(10))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(10));
					if (p <= ys) {
						DXDraw::Line2D(xp - y_r(10), yp + p, xp, yp + p, GetColor(0, 255, 0), 1);
					}
					else {
						break;
					}
				}
				//
				for (int i = -ys; i < ys; i += (y_r(100))) {
					int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(100));
					if (p <= ys) {
						DXDraw::Line2D(xp - y_r(15), yp + p, xp, yp + p, GetColor(0, 255, 0), 2);
					}
					else {
						break;
					}
				}

				if (!adss) {
					xp = xp - y_r(35);
					yp = yp - font_hight / 2;
					font->DrawStringFormat_RIGHT(xp, yp, GetColor(0, 255, 0), "km/h %4.0f", veh.speed * 3.6f);

					if (chara.use_auto_thrust) {
						//power
						font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(0, 255, 0), "POWER %03.0f%%", veh.accel);
						//オートスラスト使用
						if ((time / 100000) % 10 <= 5) {
							font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, (veh.over_heat) ? GetColor(255, 0, 0) : GetColor(128, 255, 0), "AUTO THRUST %4.0f km/h", chara.speed_auto_thrust);
						}
					}
					else {
						//power
						if (veh.over_heat) {
							font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
						}
						else if (veh.accel >= 100.f) {
							font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
						}
						else {
							font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(0, 255, 0), "POWER %03.0f%%", veh.accel);
						}
						//overheat
						if (veh.over_heat) {
							if ((time / 100000) % 5 <= 2) {
								font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
							}
						}
						else if (veh.accel >= 100.f) {
							if ((time / 100000) % 10 <= 5) {
								font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
							}
						}
					}
				}
			}
			//高度計
			{
				if (!uses_vr) {
					xs = 0;
					xp = disp_x / 2 + disp_y / 7;
					ys = disp_y / 14;
					yp = disp_y / 2;
				}
				else {
					xs = 0;
					xp = disp_x / 2 + disp_y / 6 - y_r(120);
					ys = disp_y / 6 - y_r(120);
					yp = disp_y / 2;
				}
				//
				DXDraw::Line2D(xp - y_r(10), yp, xp - y_r(1), yp, GetColor(0, 255, 0), 1);
				//
				DXDraw::Line2D(xp + y_r(17), yp, xp + y_r(25), yp - font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp + y_r(17), yp, xp + y_r(25), yp + font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp + y_r(25), yp - font_hight / 2, xp + y_r(35), yp - font_hight / 2, GetColor(0, 255, 0), 1);
				DXDraw::Line2D(xp + y_r(25), yp + font_hight / 2, xp + y_r(35), yp + font_hight / 2, GetColor(0, 255, 0), 1);
				//
				for (int i = -ys; i < ys; i += y_r(10)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(10);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + y_r(10), yp + p, GetColor(0, 255, 0), 1);
					}
					else {
						break;
					}
				}
				//
				for (int i = -ys; i < ys; i += y_r(100)) {
					int p = i + int(chara.vehicle.pos.y() + ys) % y_r(100);
					if (p <= ys) {
						DXDraw::Line2D(xp, yp + p, xp + y_r(15), yp + p, GetColor(0, 255, 0), 2);
					}
					else {
						break;
					}
				}

				if (!adss) {
					font->DrawStringFormat(xp + y_r(35), yp - font_hight / 2, GetColor(0, 255, 0), "%05d m", int(veh.pos.y()));
				}
			}
			//コンパス
			{
				VECTOR_ref tmp = chara.vehicle.mat.zvec();
				tmp = VGet(tmp.x(), 0.f, tmp.z());
				tmp = tmp.Norm();
				float rad = -std::atan2f(tmp.x(), -tmp.z())*180.f / DX_PI_F;
				rad = (rad < 0) ? (360.f + rad) : rad;
				if (!uses_vr) {
					xs = disp_y / 14;
					xp = disp_x / 2;
					ys = 0;
					yp = disp_y / 2 + disp_y / 7;
				}
				else {
					xs = disp_y / 6 - y_r(120);
					xp = disp_x / 2;
					ys = 0;
					yp = disp_y / 2 + disp_y / 6 - y_r(120);
				}
				//
				DXDraw::Line2D(xp, yp - y_r(10), xp, yp - y_r(1), GetColor(0, 255, 0), 1);
				//
				for (int i = -xs; i < xs; i += y_r(10)) {
					int p = (i + int(rad + xs) % y_r(10));
					if (p <= xs) {
						DXDraw::Line2D(xp + p, yp, xp + p, yp + y_r(10), GetColor(0, 255, 0), 1);
					}
					else {
						break;
					}
				}
				//
				for (int i = -xs; i < xs; i += y_r(60)) {
					int p = (i + int(rad + xs) % y_r(60));
					if (p <= xs) {
						DXDraw::Line2D(xp + p, yp, xp + p, yp + y_r(15), GetColor(0, 255, 0), 2);
					}
					else {
						break;
					}
				}
				if (!adss) {
					font->DrawStringFormat_MID(xp, yp + y_r(20), GetColor(0, 255, 0), "%03d °", int(rad));
				}
			}
			//キル
			if (chara.vehicle.KILL_ID != -1) {
				font->DrawStringFormat(disp_x / 4, disp_y / 3, GetColor(255, 0, 0), "KILL : %d", chara.vehicle.KILL_COUNT);
				font->DrawStringFormat(disp_x / 4, disp_y / 3 + y_r(18), GetColor(255, 0, 0), "KILL ID : %d", chara.vehicle.KILL_ID);
			}
			if (chara.vehicle.kill_f) {
				if (uses_vr) {
					xp = disp_x / 2;
					yp = disp_y / 2 - disp_y / 6 + font_hight * 3;
				}
				else {
					xp = disp_x / 2;
					yp = disp_y / 2 - disp_y / 6 + font_hight * 3;
				}
				int per = std::clamp(int(255.f*((chara.vehicle.kill_time * 7) / 7.f)) - 255 * 6, 0, 255);

				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
				DrawBox(xp - int(pow(per, 2)) * disp_x / 2 / int(pow(255, 2)), yp, xp + int(pow(per, 2)) * disp_x / 2 / int(pow(255, 2)), yp + font_hight + 2, GetColor(255, 255, 255), TRUE);
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*((chara.vehicle.kill_time * 2) / 7.f)), 0, 255));
				font->DrawStringFormat_MID(xp, yp, GetColor(255, 0, 0), "プレイヤー%d をキルしました", chara.vehicle.KILL_ID);	//キル
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			}
		}
	}

	//
	void box_p(Mainclass::Chara& c, int col) {
		if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f) {
			float siz_per = std::clamp((1.f - sqrtf(powf(c.winpos.x() - disp_x / 2, 2) + powf(c.winpos.y() - disp_y / 2, 2)) / float(disp_x / 2)), 0.f, 1.f);
			int siz = y_r(32.f*siz_per);
			DrawBox(int(c.winpos.x()) - siz, int(c.winpos.y()) - siz, int(c.winpos.x()) + siz, int(c.winpos.y()) + siz, col, FALSE);
			siz = y_r(42.f*siz_per);
			DrawBox(int(c.winpos.x()) - siz, int(c.winpos.y()) - siz, int(c.winpos.x()) + siz, int(c.winpos.y()) + siz, col, FALSE);
		}
	}
	void tri_p(Mainclass::Chara& c, Mainclass::Chara& chara, int col) {
		if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f && c.type != chara.type) {
			auto rad = atan2f(float(int(c.winpos.x()) - disp_x / 2), float(int(c.winpos.y()) - disp_y / 2));
			auto dis2 = std::clamp(sqrtf(powf(float(int(c.winpos.x()) - disp_x / 2), 2.f) + powf(float(int(c.winpos.y()) - disp_y / 2), 2.f)), 0.f, 200.f);
			auto dis = std::clamp((c.vehicle.pos - chara.vehicle.pos).size() / 10.f, 0.f, dis2);
			DrawLine(disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (dis2 - dis) / dis2))), disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (dis2 - dis) / dis2))), disp_x / 2 + int(dis2*sin(rad)), disp_y / 2 + int(dis2*cos(rad)), col, 2);
			DrawLine(disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (dis2 - dis) / dis2))), disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (dis2 - dis) / dis2))), disp_x / 2 + int(dis2*sin(rad)), disp_y / 2 + int(dis2*cos(rad)), col, 2);
			DrawLine(disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (dis2 - dis) / dis2))), disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (dis2 - dis) / dis2))), disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (dis2 - dis) / dis2))), disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (dis2 - dis) / dis2))), col, 2);
		}
	}
	void life_ver(Mainclass::Chara& c, Mainclass::Chara& chara, int col, bool uses_vr = true) {
		if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f) {
			FontHandle* font = (!uses_vr) ? &font18 : &font24;
			int xs = 0, xp = 0, ys = 0, yp = 0;
			int siz = y_r(42.f*std::clamp((1.f - sqrtf(powf(c.winpos.x() - disp_x / 2, 2) + powf(c.winpos.y() - disp_y / 2, 2)) / float(disp_x / 2)), 0.f, 1.f));
			xp = int(c.winpos.x()) - siz;
			yp = int(c.winpos.y()) + siz;
			xs = siz * 2;
			ys = y_r(8);
			DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(255, 0, 0), TRUE);
			DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4)), xp + xs * int(c.vehicle.HP) / int(c.vehicle.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, 255, 0), TRUE);
			if (c.p_anime_geardown.second >= 0.5f) {
				DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(127, 127, 127), TRUE);
			}
			/*
			for (auto& h : c.vehicle.HP_m) {
				int p = &h - &c.vehicle.HP_m[0];

				DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 + y_r(4 + p * 5)), xp + xs, yp + ys / 2 + (ys * 2 / 3 + y_r(8 + p * 5)), GetColor(255, 0, 0), TRUE);
				DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 + y_r(4 + p * 5)), xp + xs * int(h) / int(c.vehicle.use_veh.HP), yp + ys / 2 + (ys * 2 / 3 + y_r(8 + p * 5)), GetColor(0, 255, 0), TRUE);
			}
			*/
			font->DrawStringFormat(xp, yp + 18, col, "%d m", int((VECTOR_ref(c.vehicle.pos) - chara.vehicle.pos).size()));
			font->DrawStringFormat(xp, yp + 36, col, "%d km/h", int(chara.vehicle.speed*3.6f));
		}
	}

	void reset_lock() {
		id_near = 0;
	}

	void item_draw(std::vector<Mainclass::Chara>& charas, Mainclass::Chara& chara, const bool& adss, float danger_height, bool uses_vr = true) {
		int xp = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font18 : &font24;
		auto font_hight = (!uses_vr) ? y_r(18) : y_r(24);
		auto time = GetNowHiPerformanceCount();
		auto& veh = chara.vehicle;
		//取得
		{
			SetCameraNearFar(0.01f, veh.use_veh.canlook_dist*3.f);
			auto gunpos = (veh.obj.frame(veh.Gun_[0].get_gun_info().frame1.first) - veh.pos);
			auto ammo_ = (veh.Gun_[0].getbullet_use().spec.speed_a + veh.speed) / GetFPS();
			for (auto& c : charas) {
				c.winpos_if = ConvWorldPosToScreenPos((c.vehicle.pos + gunpos + c.vehicle.mat.zvec() *(-1.f*c.vehicle.speed / GetFPS())*((c.vehicle.pos - veh.pos).size() / ammo_)).get());
				c.winpos = ConvWorldPosToScreenPos(c.vehicle.pos.get());
			}
			aimpos_2 = ConvWorldPosToScreenPos((veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(0.f, 0.58f, -1.f), veh.mat)).get());
			aimpos = ConvWorldPosToScreenPos((VECTOR_ref(veh.pos) + veh.mat.zvec() * (-veh.use_veh.canlook_dist*0.8f)).get());
		}
		//描画
		if (!chara.death) {
			//照準
			if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
				int siz = y_r(96);
				circle.DrawExtendGraph(int(aimpos.x()) - siz, int(aimpos.y()) - siz, int(aimpos.x()) + siz, int(aimpos.y()) + siz, true);
			}
			//主観
			if (adss) {
				VECTOR_ref tmp_get;
				//警告
				tmp_get = ConvWorldPosToScreenPos((veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(-0.15f, 0.58f, -1.f), veh.mat)).get());
				if (tmp_get.z() >= 0.f && tmp_get.z() <= 1.f) {
					xp = (int)(tmp_get.x());
					yp = (int)(tmp_get.y());
					int ccc = 0;
					if (veh.speed < veh.use_veh.min_speed_limit) {
						if ((time / 100000) % 4 <= 2) {
							font->DrawString(xp, yp + ccc, "STALL", GetColor(255, 0, 0));
						}
						ccc += font_hight;
					}
					if (veh.pos.y() <= danger_height) {
						font->DrawString(xp, yp + ccc, "GPWS", GetColor(255, 255, 0));
						ccc += font_hight;
					}
					if (chara.p_anime_geardown.second > 0.5f) {
						font->DrawString(xp, yp + ccc, "GEAR DOWN", GetColor(255, 255, 0));
						ccc += font_hight;
					}
					if (chara.aim_cnt > 0) {
						if ((time / 80000) % 4 <= 2) {
							font->DrawString(xp, yp + ccc, "ENEMY ALERT", GetColor(255, 225, 0));
						}
						ccc += font_hight;
					}
					if (chara.missile_cnt > 0) {
						font->DrawString(xp, yp + ccc, "MISSILE ALERT", GetColor(255, 0, 0));
						ccc += font_hight;
					}
				}
				//速度計
				tmp_get = ConvWorldPosToScreenPos((chara.cocks.obj.frame(chara.cocks.speed_f.first) - (chara.cocks.obj.frame(chara.cocks.speed2_f.first) - chara.cocks.obj.frame(chara.cocks.speed_f.first)).Norm()*0.05f).get());
				if (tmp_get.z() >= 0.f && tmp_get.z() <= 1.f) {
					xp = (int)(tmp_get.x());
					yp = (int)(tmp_get.y());
					font->DrawStringFormat_RIGHT(xp, yp, GetColor(0, 255, 0), "%4.0f km/h", veh.speed * 3.6f);
					//power
					if (veh.over_heat) {
						font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
					}
					else if (veh.accel >= 100.f) {
						font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
					}
					else {
						font->DrawStringFormat_RIGHT(xp, yp + font_hight * 1, GetColor(0, 255, 0), "POWER %03.0f%%", veh.accel);
					}
					//
					if (chara.use_auto_thrust) {
						//オートスラスト使用
						if ((time / 100000) % 10 <= 5) {
							font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, (veh.over_heat) ? GetColor(255, 0, 0) : GetColor(128, 255, 0), "AUTO THRUST %4.0f km/h", chara.speed_auto_thrust);
						}
					}
					else {
						//overheat
						if (veh.over_heat) {
							if ((time / 100000) % 5 <= 2) {
								font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
							}
						}
						else if (veh.accel >= 100.f) {
							if ((time / 100000) % 10 <= 5) {
								font->DrawStringFormat_RIGHT(xp, yp + font_hight * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
							}
						}
					}
				}
				//高度計
				tmp_get = ConvWorldPosToScreenPos((chara.cocks.obj.frame(chara.cocks.alt_100_f.first) - (chara.cocks.obj.frame(chara.cocks.alt_100_2_f.first) - chara.cocks.obj.frame(chara.cocks.alt_100_f.first)).Norm()*0.05f).get());
				if (tmp_get.z() >= 0.f && tmp_get.z() <= 1.f) {
					xp = (int)(tmp_get.x());
					yp = (int)(tmp_get.y());
					font->DrawStringFormat(xp, yp, GetColor(0, 255, 0), " %4d m", int(veh.pos.y()));
				}
				//コンパス
				tmp_get = ConvWorldPosToScreenPos((chara.cocks.obj.frame(chara.cocks.subcompass_f.first) - (chara.cocks.obj.frame(chara.cocks.subcompass2_f.first) - chara.cocks.obj.frame(chara.cocks.subcompass_f.first)).Norm()*0.05f).get());
				if (tmp_get.z() >= 0.f && tmp_get.z() <= 1.f) {
					xp = (int)(tmp_get.x());
					yp = (int)(tmp_get.y());

					VECTOR_ref tmp = veh.mat.zvec();
					tmp = VGet(tmp.x(), 0.f, tmp.z());
					tmp = tmp.Norm();
					float rad = -std::atan2f(tmp.x(), -tmp.z())*180.f / DX_PI_F;
					rad = (rad < 0) ? (360.f + rad) : rad;
					font->DrawStringFormat(xp, yp, GetColor(0, 255, 0), "%03d °", int(rad));
				}
			}
			//id_near決定
			{
				if (id_near == 0 || charas[id_near].vehicle.HP == 0) {
					float d = 10000.f;
					for (auto&c : charas) {
						if (
							&c != &chara &&
							c.vehicle.HP > 0 &&
							c.type != chara.type
							) {
							auto dt = (c.vehicle.pos - veh.pos).size();
							if (d > dt) {
								id_near = &c - &charas[0];
								d = dt;
							}
						}
					}
				}
			}
			//
			for (auto&c : charas) {
				if (&c != &chara) {
					unsigned int col = (c.type == chara.type) ? GetColor(0, 255, 0) : GetColor(255, 0, 0);
					if (!c.death) {
						//箱
						box_p(c, col);
						//ライフ
						life_ver(c, chara, (&c - &charas[0] == int(id_near)) ? GetColor(255, 255, 0) : col, uses_vr);
						//方向
						if (int(id_near) != &c - &charas[0]) {
							tri_p(c, chara, GetColor(255, 0, 0));
						}
						//
					}
				}
			}
			//ロック
			{
				auto&c = charas[id_near];
				if (!c.death) {
					//箱
					box_p(c, GetColor(255, 255, 0));
					//方向
					tri_p(c, chara, GetColor(255, 255, 0));
					//予測
					if (
						c.winpos.z() >= 0.f && c.winpos.z() <= 1.f &&
						c.type != chara.type &&
						aimpos.z() >= 0.f && aimpos.z() <= 1.f &&
						c.winpos_if.z() >= 0.f && c.winpos_if.z() <= 1.f
						) {
						auto dist = std::clamp(sqrtf(powf((c.winpos.x() - c.winpos_if.x()), 2) + powf((c.winpos.y() - c.winpos_if.y()), 2)), 0.f, 42.f);
						auto rad = atan2f((c.winpos.x() - c.winpos_if.x()), (c.winpos.y() - c.winpos_if.y()));
						DrawLine(int(c.winpos_if.x()) + int(dist*sin(rad)), int(c.winpos_if.y()) + int(dist*cos(rad)), int(c.winpos_if.x()), int(c.winpos_if.y()), GetColor(255, 0, 0));
						if ((sqrtf(powf(c.winpos_if.x() - aimpos.x(), 2) + powf(c.winpos_if.y() - aimpos.y(), 2)) / float(disp_x / 2)) <= 1.f / 3.f) {
							aim_if.DrawRotaGraph(int(c.winpos_if.x()), int(c.winpos_if.y()), 1.f, 0.f, TRUE);
						}
					}
				}
				//
			}
			//
		}
		else {
			reset_lock();
			if (veh.DEATH_ID >= 0 && veh.DEATH_ID < charas.size()) {
				auto& c = charas[veh.DEATH_ID];
				if (!c.death) {
					box_p(c, (c.type == chara.type) ? GetColor(0, 255, 0) : GetColor(255, 0, 0));
				}
			}
		}
		//
				//ヒット
		{
			SetCameraNearFar(0.01f, 100.f);
			for (auto& c : charas) {
				for (auto& g : c.vehicle.Gun_) {
					for (auto& a : g.getbullet()) {
						if (a.hit_r > 0.05f) {
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, int(255.f*a.hit_r));
							this->hit.DrawRotaGraph(a.hit_x, a.hit_y, a.hit_r*(&c == &chara ? 1.f : 0.5f), 0.f, true);
						}
					}
				}
			}
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
		//

		//視点中央
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
			DrawRotaGraph(disp_x / 2, disp_y / 2, float(y_r(64)) / 200.f, 0.f, aim.get(), TRUE);//3
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
	}

	void res_draw(Mainclass::Chara& chara, bool uses_vr = true) {
		int yyy = 0;
		int xp = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font24 : &font36;
		auto ysize = (!uses_vr) ? y_r(24) : y_r(36);
		yyy += ysize;
		if (aimpos_2.z() >= 0.f && aimpos_2.z() <= 1.f) {
			xp = (int)(aimpos_2.x());
			yp = (int)(aimpos_2.y());
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
			DrawBox(xp - disp_x / 6, yp - disp_y / 6, xp + disp_x / 6, yp + disp_y / 6, GetColor(0, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

			font->DrawString(xp - disp_x / 6, yp - disp_y / 6 + yyy, "KILL", GetColor(255, 255, 255));
			font->DrawStringFormat_RIGHT(xp + disp_x / 6, yp - disp_y / 6 + yyy, GetColor(255, 255, 255), "%02d", chara.vehicle.KILL_COUNT);
			yyy += ysize * 2;
			font->DrawString(xp - disp_x / 6, yp - disp_y / 6 + yyy, "DEATH", GetColor(255, 255, 255));
			font->DrawStringFormat_RIGHT(xp + disp_x / 6, yp - disp_y / 6 + yyy, GetColor(255, 255, 255), "%02d", chara.vehicle.DEATH_COUNT);
			yyy += ysize * 2;

			font->DrawString(xp - disp_x / 6, yp + disp_y / 6 - ysize, "CLASS", GetColor(255, 255, 255));
			font->DrawStringFormat_RIGHT(xp + disp_x / 6, yp + disp_y / 6 - ysize,
				(chara.vehicle.KILL_COUNT < 3) ? GetColor(255, 255, 255) :
				((chara.vehicle.KILL_COUNT < 5) ? GetColor(0, 255, 0) :
				((chara.vehicle.KILL_COUNT < 7) ? GetColor(50, 50, 255) :
					((chara.vehicle.KILL_COUNT < 10) ? GetColor(255, 50, 255) : GetColor(255, 255, 0))))
				, "%s",
				(chara.vehicle.KILL_COUNT < 3) ? "Airman" :
				((chara.vehicle.KILL_COUNT < 5) ? "Senior Master" :
				((chara.vehicle.KILL_COUNT < 7) ? "Captain" :
					((chara.vehicle.KILL_COUNT < 10) ? "Major" : "General"))));
		}
	}
};
