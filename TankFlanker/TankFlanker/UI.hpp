#pragma once

//リサイズ
#define x_r(p1 , p2x) (int(p1) * p2x / deskx)
#define y_r(p1 , p2y) (int(p1) * p2y / desky)

class UI : Mainclass {
private:
	//
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle aim_if;
	GraphHandle dmg;
	//
	float ber = 0;
	GraphHandle bufScreen;
	//font
	FontHandle font36;
	FontHandle font24;
	FontHandle font18;
	FontHandle font12;
	//
	VECTOR_ref HMDpos;
	MATRIX_ref HMDmat;
	VECTOR_ref rec_HMD;
	//描画
	MV1 garage;
	//
	int out_disp_x = deskx;
	int out_disp_y = desky;
	int disp_x = deskx;
	int disp_y = desky;
	//
	VECTOR_ref aimpos;
	VECTOR_ref aimpos_2;
	VECTOR_ref aimpos_3;
	VECTOR_ref altpos;
	VECTOR_ref spdpos;
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

		bufScreen = GraphHandle::Make(disp_x, disp_y, true);

		font36 = FontHandle::Create(y_r(36, out_disp_y), DX_FONTTYPE_EDGE);
		font24 = FontHandle::Create(y_r(24, out_disp_y), DX_FONTTYPE_EDGE);
		font18 = FontHandle::Create(y_r(18, out_disp_y), DX_FONTTYPE_EDGE);
		font12 = FontHandle::Create(y_r(12, out_disp_y), DX_FONTTYPE_EDGE);
		MV1::Load("data/model/garage/model.mv1", &garage, false);
	}
	~UI() {
	}
	bool select_window(Mainclass::Chara* chara, std::vector<Mainclass::Vehcs>* vehcs, std::unique_ptr<DXDraw, std::default_delete<DXDraw>>& Drawparts) {
		auto& veh = chara->vehicle;
		if (1) {
			uint8_t rtct = 0, ltct = 0;
			float fov = 45.f;
			veh.use_id %= (*vehcs).size(); //飛行機
			MV1AttachAnim((*vehcs)[veh.use_id].obj.get(), 1);
			float speed = 0.f;
			VECTOR_ref pos;
			pos.y(1.8f);

			VECTOR_ref pos_mine = VGet(15.f, 0, 0);

			bool endp = false;
			bool startp = false;
			float rad = 0.f;
			float xrad_m = 0.f;
			float yrad_m = 0.f;
			float yrad_im = 0.f, xrad_im = 0.f;
			int m_x = 0, m_y = 0;
			float ber_r = 0.f;
			bool oldv = false;
			bool start_c = true;
			GetMousePoint(&m_x, &m_y);
			Mainclass::CAMS cam_s;
			cam_s.cam.campos = VGet(0.f, 0.f, -15.f);
			cam_s.cam.camvec = VGet(0.f, 3.f, 0.f);
			while (ProcessMessage() == 0) {
				const auto waits = GetNowHiPerformanceCount();
				if (!startp) {
					if (Drawparts->use_vr) {
						auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
						if (&ptr_LEFTHAND != nullptr) {
							if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
								//
								{
									ltct = std::clamp<uint8_t>(ltct + 1, 0, ((((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() < -0.5f) ? 2 : 0));
									rtct = std::clamp<uint8_t>(rtct + 1, 0, ((((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() > 0.5f) ? 2 : 0));
									if (ltct == 1) {
										++veh.use_id %= (*vehcs).size();
										MV1AttachAnim((*vehcs)[veh.use_id].obj.get(), 1);
									}
									if (rtct == 1) {
										if (veh.use_id == 0) {
											veh.use_id = (*vehcs).size() - 1;
										}
										else {
											--veh.use_id;
										}
										MV1AttachAnim((*vehcs)[veh.use_id].obj.get(), 1);
									}
								}
								//
								if ((ptr_LEFTHAND.on[0] & BUTTON_TRIGGER) != 0) {
									break;
								}
								//
							}
						}
					}
					else {
						{
							int x, y;
							GetMousePoint(&x, &y);
							yrad_im = std::clamp(yrad_im + float(m_x - x) / 5.f, -120.f, -30.f);
							xrad_im = std::clamp(xrad_im + float(m_y - y), -0.f, 45.f);
							m_x = x;
							m_y = y;
							easing_set(&yrad_m, deg2rad(yrad_im), 0.9f);
							easing_set(&xrad_m, deg2rad(xrad_im), 0.9f);
						}
						//
						{
							ltct = std::clamp<uint8_t>(ltct + 1, 0, ((CheckHitKey(KEY_INPUT_A) != 0) ? 2 : 0));
							rtct = std::clamp<uint8_t>(rtct + 1, 0, ((CheckHitKey(KEY_INPUT_D) != 0) ? 2 : 0));

							if (ltct == 1) {
								++veh.use_id %= (*vehcs).size();
								MV1AttachAnim((*vehcs)[veh.use_id].obj.get(), 1);
							}
							if (rtct == 1) {
								if (veh.use_id == 0) {
									veh.use_id = (*vehcs).size() - 1;
								}
								else {
									--veh.use_id;
								}
								MV1AttachAnim((*vehcs)[veh.use_id].obj.get(), 1);
							}
						}
						//
						if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
							startp = true;
						}
						//
					}
				}
				else {
					speed = std::clamp(speed + 1.5f / 3.6f / GetFPS(), 0.f, 20.f / 3.6f / GetFPS());
					pos.zadd(-speed);

					if (pos.z() <= -15.f) {
						easing_set(&fov, 45.f / 1.75f, 0.95f);
					}
					if (pos.z() < -20.f) {
						endp = true;
					}
				}
				//視点取得
				if (Drawparts->use_vr) {
					auto& ptr_ = *Drawparts->get_device_hmd();
					Drawparts->GetDevicePositionVR(Drawparts->get_hmd_num(), &HMDpos, &HMDmat);
					if (start_c && (ptr_.turn && ptr_.now) != oldv) {
						rec_HMD = VGet(HMDpos.x(), 0.f, HMDpos.z());
						start_c = false;
					}
					if (!start_c && !(ptr_.turn && ptr_.now)) {
						start_c = true;
					}
					oldv = ptr_.turn && ptr_.now;
					HMDpos = HMDpos - rec_HMD;
					HMDmat = MATRIX_ref::Axis1(HMDmat.xvec()*-1.f, HMDmat.yvec(), HMDmat.zvec()*-1.f);
					cam_s.cam.campos = pos_mine + HMDpos;
					cam_s.cam.camvec = cam_s.cam.campos - HMDmat.zvec();
					cam_s.cam.camup = HMDmat.yvec();
				}
				else {
					if (!startp) {
						easing_set(&cam_s.cam.campos, (MATRIX_ref::RotX(xrad_m) * MATRIX_ref::RotY(yrad_m)).zvec() * (-15.f) + VGet(0.f, 3.f, 0.f), 0.95f);
						cam_s.cam.camvec = pos + VGet(0.f, 3.f, 0.f);
					}
					else {
						easing_set(&cam_s.cam.campos, VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 3.f, (1.f - (pos.z() / -120.f)) + 10.f), 0.95f);
						easing_set(&cam_s.cam.camvec, pos + VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 1.f, (1.f - (pos.z() / -120.f))), 0.95f);
					}
					cam_s.cam.camup = VGet(0.f, 1.f, 0.f);
				}
				//UI_buf
				{
					bufScreen.SetDraw_Screen();
					{
						{
							int xp = out_disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(90)));
							int yp = out_disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(90)));
							int xa = out_disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150, out_disp_y)) * sin(rad + deg2rad(90)));
							int ya = out_disp_y * 2 / 3 + int((ber_r - y_r(150, out_disp_y)) * cos(rad + deg2rad(90)));
							DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
							{
								DrawBox(xp - y_r(120, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(60, out_disp_y), GetColor(0, 0, 0), TRUE);
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3, out_disp_y), GetColor(0, 255, 0), "Name     :%s", (*vehcs)[veh.use_id].name.c_str());
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 20, out_disp_y), GetColor(0, 255, 0), "MaxSpeed :%03.0f km/h", (*vehcs)[veh.use_id].max_speed_limit*3.6f);
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 40, out_disp_y), GetColor(0, 255, 0), "MidSpeed :%03.0f km/h", (*vehcs)[veh.use_id].mid_speed_limit*3.6f);
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 60, out_disp_y), GetColor(0, 255, 0), "MinSpeed :%03.0f km/h", (*vehcs)[veh.use_id].min_speed_limit*3.6f);
								font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(60 - 3 - 80, out_disp_y), GetColor(0, 255, 0), "Turn     :%03.0f °/s", (*vehcs)[veh.use_id].body_rad_limit);
								DrawBox(xp - y_r(120, out_disp_y), yp - y_r(60, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(60, out_disp_y), GetColor(0, 255, 0), FALSE);
								font12.DrawString(xp - y_r(120, out_disp_y), yp - y_r(60 + 15, out_disp_y), "Spec", GetColor(0, 255, 0));
							}
						}

						{
							int xp = out_disp_x / 2 + int((ber_r * 16.f / 9.f) * sin(rad + deg2rad(180)));
							int yp = out_disp_y * 2 / 3 + int(ber_r * cos(rad + deg2rad(180)));
							int xa = out_disp_x / 2 + int(((ber_r * 16.f / 9.f) - y_r(150, out_disp_y)) * sin(rad + deg2rad(180)));
							int ya = out_disp_y * 2 / 3 + int((ber_r - y_r(150, out_disp_y)) * cos(rad + deg2rad(180)));
							DXDraw::Line2D(xa, ya, xp, yp, GetColor(0, 255, 0), 2);
							{
								int ys = 20 * int((*vehcs)[veh.use_id].gunframe.size()) / 2 + 1;
								DrawBox(xp - y_r(120, out_disp_y), yp - y_r(ys, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(ys, out_disp_y), GetColor(0, 0, 0), TRUE);
								if ((*vehcs)[veh.use_id].gunframe.size() == 0) {
									font18.DrawString(xp - y_r(120 - 3, out_disp_y), yp - y_r(ys - 3, out_disp_y), "N/A", GetColor(0, 255, 0));
								}
								else {
									for (int z = 0; z < (*vehcs)[veh.use_id].gunframe.size(); z++) {
										font18.DrawStringFormat(xp - y_r(120 - 3, out_disp_y), yp - y_r(ys - 3 - 20 * z, out_disp_y), GetColor(0, 255, 0), "No.%d  :%s", z, (*vehcs)[veh.use_id].gunframe[z].name.c_str());
									}
								}
								DrawBox(xp - y_r(120, out_disp_y), yp - y_r(ys, out_disp_y), xp + y_r(120, out_disp_y), yp + y_r(ys, out_disp_y), GetColor(0, 255, 0), FALSE);
								font12.DrawString(xp - y_r(120, out_disp_y), yp - y_r(ys + 15, out_disp_y), "Weapon", GetColor(0, 255, 0));
							}
						}
					}
					easing_set(&ber_r, float(out_disp_y / 4), 0.95f);
					easing_set(&rad, deg2rad(yrad_im), 0.9f);
				}
				Drawparts->Move_Player();
				//自機描画
				{
					//cam_s.cam
					{
						//far取得
						cam_s.cam.far_ = 300.f;
						//near取得
						cam_s.cam.near_ = 0.1f;
						//fov
						cam_s.cam.fov = deg2rad(Drawparts->use_vr ? 90 : fov);
					}
					//
					//VRに移す
					Drawparts->draw_VR(
						[&] {
						auto tmp = GetDrawScreen();
						auto tmp_cams = cam_s;
						//*
						auto camtmp = VECTOR_ref(GetCameraPosition()) - cam_s.cam.campos;

						auto tvec = (VECTOR_ref(cam_s.cam.camvec) - cam_s.cam.campos);

						camtmp = MATRIX_ref::Vtrans(camtmp, MATRIX_ref::Axis1(
							tvec.cross(cam_s.cam.camup),
							cam_s.cam.camup,
							tvec));

						tmp_cams.cam.campos = camtmp + cam_s.cam.campos;
						tmp_cams.cam.camvec = camtmp + cam_s.cam.camvec;
						//*/
						GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
						{
							garage.DrawModel();
							(*vehcs)[veh.use_id].obj.SetMatrix(MATRIX_ref::Mtrans(pos));
							(*vehcs)[veh.use_id].obj.get_anime();
							MV1SetAttachAnimBlendRate((*vehcs)[veh.use_id].obj.get(), 1, 1.f);
							(*vehcs)[veh.use_id].obj.DrawModel();
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
							bufScreen.DrawGraph(0, 0, true);
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * (pos.z() + 60.f) / -60.f), 0, 255));
							DrawBox(0, 0, out_disp_x, out_disp_y, GetColor(255, 255, 255), TRUE);
							SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
							//UIparts->item_draw(chara, mine, tmp_cams);
						}
					}, cam_s.cam);
				}
				//draw
				GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
				{
					if (Drawparts->use_vr) {
						Drawparts->outScreen[0].DrawRotaGraph(960, 540, 0.5f, 0, false);
					}
					else {
						Drawparts->outScreen[0].DrawGraph(0, 0, false);
					}
				}
				Drawparts->Screen_Flip(waits);
				if (endp) {
					WaitTimer(100);
					break;
				}
				if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
					break;
				}
			}
		}
		else {
			veh.use_id = 1; //飛行機
		}
		return (CheckHitKey(KEY_INPUT_ESCAPE) == 0);
	}

	void load_window(const char* mes) {
		SetUseASyncLoadFlag(FALSE);
		float bar = 0.f, cnt = 0.f;
		auto all = GetASyncLoadNum();
		while (ProcessMessage() == 0) {
			SetDrawScreen(DX_SCREEN_BACK);
			ClearDrawScreen();
			{
				font18.DrawStringFormat(0, out_disp_y - y_r(70, out_disp_y), GetColor(0, 255, 0), " loading... : %04d/%04d  ", all - GetASyncLoadNum(), all);
				font12.DrawStringFormat(out_disp_x - font12.GetDrawWidthFormat("%s 読み込み中 ", mes), out_disp_y - y_r(70, out_disp_y), GetColor(0, 255, 0), "%s 読み込み中 ", mes);
				DrawBox(0, out_disp_y - y_r(50, out_disp_y), int(float(out_disp_x) * bar / float(all)), out_disp_y - y_r(40, out_disp_y), GetColor(0, 255, 0), TRUE);
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

	void draw(
		std::vector<Mainclass::Chara>& charas,
		Mainclass::Chara& chara,
		const bool& adss,
		const DXDraw::system_VR& vr_sys,
		bool uses_vr = true
	) {
		int xs = 0, xp = 0, ys = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font18 : &font36;
		if (uses_vr) {
			GetScreenState(&disp_x, &disp_y, nullptr);
		}
		//HP表示
		{
			if (uses_vr) {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(1.f - (float(chara.vehicle.HP) / chara.vehicle.use_veh.HP))*0.5f), 0, 255));
				DrawBox(0, 0, disp_x, disp_y, GetColor(128, 0, 0), TRUE);
			}
			else {
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(1.f - float(chara.vehicle.HP) / chara.vehicle.use_veh.HP)), 0, 255));
				dmg.DrawExtendGraph(0, 0, disp_x, disp_y, true);
			}
			//ダメージ
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f*(float(int(chara.vehicle.HP_r) - chara.vehicle.HP) / float(chara.vehicle.use_veh.HP))), 0, 255));
			DrawBox(0, 0, disp_x, disp_y, GetColor(128, 0, 0), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		}
		if (!chara.death) {
			//照準
			{
				int siz = int(64.f);
				DrawRotaGraph(int(disp_x / 2), int(disp_y / 2), y_r(siz, out_disp_y) / 200.f, 0.f, aim.get(), TRUE);//3
			}
			//
			{
				{
					//弾薬
					{
						if (!uses_vr) {
							xs = x_r(200, out_disp_x);
							xp = x_r(20, out_disp_x);
							ys = y_r(18, out_disp_y);
							yp = disp_y - y_r(20, out_disp_y) - int(chara.vehicle.Gun_.size()) * (ys * 2 + y_r(7, out_disp_y));
						}
						else {
							xs = x_r(200, out_disp_x);
							xp = disp_x / 2 - x_r(20, out_disp_x) - xs;
							ys = y_r(36, out_disp_y);
							yp = disp_y / 2 + disp_y / 6 + y_r(20, out_disp_y)// - int(chara.vehicle.Gun_.size()) * (ys * 2 + y_r(3, out_disp_y))
								;
						}
						int i = 0;
						for (auto& veh : chara.vehicle.Gun_) {
							if (veh.loadcnt != 0.f) {
								DrawBox(
									xp,
									yp + ys * 2 / 3,
									xp + x_r(200 - int(200.f * veh.loadcnt / veh.gun_info.load_time), out_disp_x),
									yp + ys,
									GetColor(255, 0, 0), TRUE);
							}
							else {
								DrawBox(
									xp,
									yp + ys * 2 / 3,
									xp + xs,
									yp + ys,
									GetColor(0, 255, 0), TRUE);
							}

							if (veh.rounds != 0.f) {
								DrawBox(
									xp,
									yp + ys * 2 - y_r(2, out_disp_y),
									xp + x_r(int(200.f * veh.rounds / veh.gun_info.rounds), out_disp_x),
									yp + ys * 2 + y_r(2, out_disp_y),
									GetColor(255, 192, 0), TRUE);
							}

							font->DrawString(xp, yp, veh.bullet[veh.usebullet].spec.name_a, GetColor(255, 255, 255));
							font->DrawStringFormat(xp + xs - font->GetDrawWidthFormat("%04d / %04d", veh.rounds, veh.gun_info.rounds), yp + ys + y_r(2, out_disp_y), GetColor(255, 255, 255), "%04d / %04d", veh.rounds, veh.gun_info.rounds);
							i++;

							xp += x_r(-(30 / int(chara.vehicle.Gun_.size())), out_disp_x);
							yp += (ys * 2 + y_r(4, out_disp_y));
						}
					}
					//飛行機モード用
					{
						auto& veh = chara.vehicle;
						//速度計
						{
							if (!adss) {
								if (!uses_vr) {
									xp = disp_x / 3;
									yp = disp_y / 2;
								}
								else {
									xp = disp_x / 2 - disp_y / 6 + y_r(240 / 2, out_disp_y);
									yp = disp_y / 2;
								}
								font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 0, GetColor(255, 255, 255), "%4.0f km/h", veh.speed * 3.6f);

								if (veh.over_heat) {
									font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
									if ((GetNowHiPerformanceCount() / 100000) % 5 <= 2) {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
									}
								}
								else {
									if (veh.accel >= 100.f) {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
									}
									else {
										font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 255), "POWER %03.0f%%", veh.accel);
									}
								}
							}
						}
						//高度計
						{
							//
							if (!adss) {
								//
								if (!uses_vr) {
									xp = disp_x * 2 / 3;
									yp = disp_y / 2;
								}
								else {
									xp = disp_x / 2 + (disp_y / 3 - y_r(240, out_disp_y)) / 2;
									yp = disp_y / 2;
								}
								font->DrawStringFormat(xp, yp, GetColor(255, 255, 255), " %4d m", int(veh.pos.y()));
							}
						}
						//アラート
						{
							//
							if (!adss) {
								if (!uses_vr) {
									xp = disp_x / 2;
									yp = disp_y / 3;
								}
								else {
									xp = disp_x / 2;
									yp = disp_y / 2 - disp_y / 6 + y_r(60, out_disp_y);
								}
								int ccc = 0;
								if (veh.speed < veh.use_veh.min_speed_limit) {
									font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "STALL", GetColor(255, 0, 0));
									ccc++;
								}
								if (veh.pos.y() <= 30.f) {
									font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "GPWS", GetColor(255, 255, 0));
									ccc++;
								}
								if (chara.p_anime_geardown.second > 0.5f) {
									font->DrawString_MID(xp, yp + y_r(36, out_disp_y) * ccc, "GEAR DOWN", GetColor(255, 255, 0));
									ccc++;
								}
							}

						}
						//HP
						{
							if (!uses_vr) {
								xs = x_r(200, out_disp_x);
								xp = disp_x - x_r(20 + 30, out_disp_x) - xs;
								ys = y_r(42, out_disp_y);
								yp = disp_y - y_r(20, out_disp_y) - ys;
							}
							else {
								xs = x_r(200, out_disp_x);
								xp = disp_x / 2 + x_r(20, out_disp_x);
								ys = y_r(36, out_disp_y);
								yp = disp_y / 2 + disp_y / 6 + y_r(20, out_disp_y) - ys;
							}

							//
							{
								auto per = 255;
								DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + int(ber), yp + ys / 2 + ys * 2 / 3, GetColor(per, 0, 0), TRUE);
								easing_set(&ber, float(xs * int(veh.HP) / int(veh.use_veh.HP)), 0.975f);
								DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs * int(veh.HP) / int(veh.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, per, 0), TRUE);
								SetDrawBright(per, per, per);
								font->DrawStringFormat(
									xp,
									yp + ys + (ys * 2 / 3 - y_r(12, out_disp_y)) / 2,
									GetColor(255, 255, 255), "%d / %d", int(veh.HP), int(veh.use_veh.HP));

								font->DrawStringFormat(
									xp + (xs - font->GetDrawWidthFormat("%s", veh.use_veh.name.c_str())),
									yp + (ys * 2 / 3 - y_r(12, out_disp_y)) / 2,
									GetColor(255, 255, 255), "%s", veh.use_veh.name.c_str());

								SetDrawBright(255, 255, 255);
								yp += ys;
								xp += x_r(30, out_disp_x);
							}
						}
						{
							if (!uses_vr) {
								xs = x_r(230, out_disp_x);
								xp = disp_x - x_r(20 + 30, out_disp_x) - xs;
								ys = y_r(304, out_disp_y);
								yp = disp_y - y_r(20 + 50, out_disp_y) - ys;
							}
							else {
								xs = x_r(230, out_disp_x);
								xp = disp_x / 2 + x_r(20, out_disp_x);
								ys = y_r(304, out_disp_y);
								yp = disp_y / 2 + disp_y / 6 + y_r(20, out_disp_y) - ys;
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
					}
					//
				}
			}
			//VR用オプション
			if (uses_vr) {
				const float vr_sys_yvec_y = vr_sys.yvec.y();
				const float vr_sys_yvec_x = vr_sys.yvec.x();
				const float vr_sys_touch_y = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.y() : 0.f;
				const float vr_sys_touch_x = ((vr_sys.on[1] & BUTTON_TOUCHPAD) != 0) ? vr_sys.touch.x() : 0.f;
				//
				{
					//ピッチ、ロール
					{
						int size = y_r(10, disp_y);
						ys = disp_y / 3 - y_r(240, out_disp_y);
						xp = disp_x / 2 + ys / 2;
						yp = disp_y / 2 + ys / 2;

						int y_pos = int(float(size) * std::clamp(vr_sys_yvec_y / sin(deg2rad(20)), -2.f, 2.f));//ピッチ
						int x_pos = int(float(size) * std::clamp(vr_sys_yvec_x / sin(deg2rad(20)), -2.f, 2.f));//ロール

						for (int i = 0; i < 2; i++) {
							DrawCircle(xp, yp, size * 2, i ? GetColor(0, 255, 100) : GetColor(0, 0, 0), FALSE, 1 + i);
							DrawCircle(xp, yp, size, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), FALSE, 1 + i);
							DrawLine(xp - size / 2, yp, xp - size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
							DrawLine(xp, yp - size / 2, xp, yp - size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
							DrawLine(xp + size / 2, yp, xp + size / 4, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
							DrawLine(xp, yp + size / 2, xp, yp + size / 4, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						}

						DrawCircle(xp + x_pos, yp + y_pos, size / 5, GetColor(255, 100, 50), FALSE, 2);
					}
					//ヨー
					{
						xs = disp_y / 5 - y_r(144, out_disp_y);
						xp = disp_x / 2 - xs / 2;
						yp = disp_y / 2 + disp_y / 6 - y_r(220 / 2, out_disp_y);
						int z_pos = int(float(xs / 4) * std::clamp(vr_sys_touch_x / 0.5f, -1.5f, 1.5f));//ヨー

						for (int i = 0; i < 2; i++) {
							DXDraw::Line2D(xp + xs / 2 - xs / 3, yp, xp + xs / 2 + xs / 3, yp, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
							DXDraw::Line2D(xp + xs / 4, yp - 5, xp + xs / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
							DXDraw::Line2D(xp + xs * 3 / 4, yp - 5, xp + xs * 3 / 4, yp + 5, i ? GetColor(0, 255, 0) : GetColor(0, 0, 0), 1 + i);
						}
						DXDraw::Line2D(xp + xs / 2 + z_pos, yp - 5, xp + xs / 2 + z_pos, yp + 5, GetColor(255, 100, 50), 2);
					}
				}

			}
			{
				//ピッチ
				/*
				{
					int ys = disp_y / 3 - y_r(240, out_disp_y);
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
						xs = x_r(200, out_disp_x);
						xp = disp_x / 3;
						ys = disp_y / 4 / 3;
						yp = disp_y / 2;
					}
					else {
						xs = 0;
						xp = disp_x / 2 - (disp_y / 3 - y_r(240, out_disp_y)) / 2;
						ys = (disp_y / 3 - y_r(240, out_disp_y)) / 3;
						yp = disp_y / 2;
					}

					DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 0, 0), 2);
					DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 255, 0), 1);

					for (int i = -ys; i < ys; i += (y_r(10, out_disp_y))) {
						int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(10, out_disp_y));
						if (p <= ys) {
							DXDraw::Line2D(xp, yp + p, xp + 10, yp + p, GetColor(0, 255, 0), 1);
						}
						else {
							break;
						}
					}

					for (int i = -ys; i < ys; i += (y_r(100, out_disp_y))) {
						int p = i + int(chara.vehicle.speed * 3.6f + ys) % (y_r(100, out_disp_y));
						if (p <= ys) {
							DXDraw::Line2D(xp, yp + p, xp + 15, yp + p, GetColor(0, 255, 0), 2);
						}
						else {
							break;
						}
					}
				}
				//高度計
				{
					if (!uses_vr) {
						xs = x_r(200, out_disp_x);
						xp = disp_x * 2 / 3;
						ys = (disp_y / 4) / 3;
						yp = disp_y / 2;
					}
					else {
						xs = 0;
						xp = disp_x / 2 + (disp_y / 3 - y_r(240, out_disp_y)) / 2;
						ys = (disp_y / 3 - y_r(240, out_disp_y)) / 3;
						yp = disp_y / 2;
					}

					DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 0, 0), 2);
					DXDraw::Line2D(xp, yp - ys, xp, yp + ys, GetColor(0, 255, 0), 1);

					for (int i = -ys; i < ys; i += y_r(5, out_disp_y)) {
						int p = i + int(chara.vehicle.pos.y() + ys) % y_r(5, out_disp_y);
						if (p <= ys) {
							DXDraw::Line2D(xp, yp + p, xp - 10, yp + p, GetColor(0, 255, 0), 1);
						}
						else {
							break;
						}
					}

					for (int i = -ys; i < ys; i += y_r(50, out_disp_y)) {
						int p = i + int(chara.vehicle.pos.y() + ys) % y_r(50, out_disp_y);
						if (p <= ys) {
							DXDraw::Line2D(xp, yp + p, xp - 15, yp + p, GetColor(0, 255, 0), 2);
						}
						else {
							break;
						}
					}
				}
			}

			{
				float d = 10000.f;
				for (auto&c : charas) {
					if (&c != &chara && c.vehicle.HP > 0 && c.id != chara.id && (chara.vehicle.mat.zvec()).dot((c.vehicle.pos - chara.vehicle.pos).Norm()) < -cos(15)) {
						auto dt = (c.vehicle.pos - chara.vehicle.pos).size();
						if (d > dt) {
							id_near = &c - &charas[0];
							d = dt;
						}
					}
				}
			}
			if (chara.vehicle.KILL_ID != -1) {
				font->DrawStringFormat(disp_x / 4, disp_y / 3, GetColor(255, 0, 0), "KILL : %d", chara.vehicle.KILL_COUNT);
				font->DrawStringFormat(disp_x / 4, disp_y / 3 + y_r(18, out_disp_y), GetColor(255, 0, 0), "KILL ID : %d", chara.vehicle.KILL_ID);
			}
		}
	}

	void item_draw(std::vector<Mainclass::Chara>& charas, Mainclass::Chara& chara, const Mainclass::CAMS& cam_s, const bool& adss, bool uses_vr = true) {
		int xs = 0, xp = 0, ys = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font18 : &font36;

		SetCameraNearFar(0.01f, chara.vehicle.use_veh.canlook_dist*3.f);

		VECTOR_ref aimp = (VECTOR_ref)(chara.vehicle.pos) + chara.vehicle.mat.zvec() * (-chara.vehicle.use_veh.canlook_dist*0.8f);
		//mapparts->map_col_line_nearest(chara.vehicle.pos, &aimp);				//地形
		aimpos = ConvWorldPosToScreenPos(aimp.get());
		for (auto& c : charas) {
			c.winpos_if = ConvWorldPosToScreenPos((c.vehicle.pos + (c.vehicle.mat.zvec() * (-c.vehicle.speed / GetFPS()))*((chara.vehicle.pos - c.vehicle.pos).size() / (600.f / GetFPS()))).get());
			//pos = c.vehicle.pos + (c.vehicle.mat.zvec() * (-c.vehicle.speed / GetFPS()))*((chara.vehicle.pos - pos).size() / (a.spec.speed_a));
			c.winpos = ConvWorldPosToScreenPos(c.vehicle.pos.get());
		}
		if (cam_s.Rot >= ADS) {
			altpos = ConvWorldPosToScreenPos((chara.cocks.obj.frame(chara.cocks.alt_100_f.first) - (chara.cocks.obj.frame(chara.cocks.alt_100_2_f.first) - chara.cocks.obj.frame(chara.cocks.alt_100_f.first)).Norm()*0.05f).get());
			spdpos = ConvWorldPosToScreenPos((chara.cocks.obj.frame(chara.cocks.speed_f.first) - (chara.cocks.obj.frame(chara.cocks.speed2_f.first) - chara.cocks.obj.frame(chara.cocks.speed_f.first)).Norm()*0.05f).get());
			auto& veh = chara.vehicle;
			aimpos_2 = ConvWorldPosToScreenPos((veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(-0.15f, 0.58f, -1.f), veh.mat)).get());
		}
		{
			auto& veh = chara.vehicle;
			aimpos_3 = ConvWorldPosToScreenPos((veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(0.f, 0.58f, -1.f), veh.mat)).get());
		}

		if (!chara.death) {
			auto& veh = chara.vehicle;
			//照準
			{
				int siz = int(64.f);
				if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
					circle.DrawExtendGraph(int(aimpos.x()) - y_r(siz, out_disp_y), int(aimpos.y()) - y_r(siz, out_disp_y), int(aimpos.x()) + y_r(siz, out_disp_y), int(aimpos.y()) + y_r(siz, out_disp_y), TRUE);
				}
			}
			//
			if (adss) {
				if (aimpos_2.z() >= 0.f && aimpos_2.z() <= 1.f) {
					xp = (int)(aimpos_2.x());
					yp = (int)(aimpos_2.y());
					int ccc = 0;
					if (veh.speed < veh.use_veh.min_speed_limit) {
						if ((GetNowHiPerformanceCount() / 100000) % 4 <= 2) {
							font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "STALL", GetColor(255, 0, 0));
						}
						ccc++;
					}
					if (veh.pos.y() <= 30.f) {
						font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "GPWS", GetColor(255, 255, 0));
						ccc++;
					}
					if (chara.p_anime_geardown.second > 0.5f) {
						font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "GEAR DOWN", GetColor(255, 255, 0));
						ccc++;
					}
					if (chara.aim_cnt > 0) {
						if ((GetNowHiPerformanceCount() / 80000) % 4 <= 2) {
							font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "ENEMY ALERT", GetColor(255, 225, 0));
						}
						ccc++;
					}
					if (chara.missile_cnt > 0) {
						font->DrawString(xp, yp + y_r(36, out_disp_y) * ccc, "MISSILE ALERT", GetColor(255, 0, 0));
						ccc++;
					}
				}
			}
			//
			if (adss) {
				if (spdpos.z() >= 0.f && spdpos.z() <= 1.f) {
					xp = (int)(spdpos.x());
					yp = (int)(spdpos.y());
					font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 0, GetColor(0, 255, 0), "%4.0f km/h", veh.speed * 3.6f);

					if (veh.over_heat) {
						font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 0, 0), "POWER %03.0f%%", veh.accel);
						if ((GetNowHiPerformanceCount() / 100000) % 5 <= 2) {
							font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 0, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
						}
					}
					else {
						if (veh.accel >= 100.f) {
							font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(255, 255, 0), "POWER %03.0f%%", veh.accel);
							font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 2, GetColor(255, 255, 0), "OVER HEAT %05.2fs / %05.2fs", veh.WIP_timer_limit - veh.WIP_timer, veh.WIP_timer_limit);
						}
						else {
							font->DrawStringFormat_RIGHT(xp, yp + y_r(36, out_disp_y) * 1, GetColor(0, 255, 0), "POWER %03.0f%%", veh.accel);
						}
					}
				}
			}
			//
			if (adss) {
				if (altpos.z() >= 0.f && altpos.z() <= 1.f) {
					xp = (int)(altpos.x());
					yp = (int)(altpos.y());
					font->DrawStringFormat(xp, yp, GetColor(0, 255, 0), " %4d m", int(veh.pos.y()));
				}
			}
			for (auto&c : charas) {
				if (&c != &chara) {
					int siz = int(32.f);
					float siz_per = 1.f;

					unsigned int col = 0;

					if (c.id == chara.id) {
						col = GetColor(0, 255, 0);
					}
					else {
						col = GetColor(255, 0, 0);
					}

					if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f && !c.death) {
						siz_per = std::clamp((1.f - sqrtf(powf(c.winpos.x() - disp_x / 2, 2) + powf(c.winpos.y() - disp_y / 2, 2)) / float(disp_x / 2)), 0.f, 1.f);

						siz = int(32.f*siz_per);
						DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), (&c - &charas[0] == int(id_near)) ? GetColor(255, 255, 0) : col, FALSE);
						siz = int(42.f*siz_per);
						DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), (&c - &charas[0] == int(id_near)) ? GetColor(255, 255, 0) : col, FALSE);

						xp = int(c.winpos.x()) - y_r(siz, out_disp_y);
						yp = int(c.winpos.y()) + y_r(siz, out_disp_y);
						xs = y_r(siz * 2, out_disp_y);
						ys = y_r(8, out_disp_y);
						DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(255, 0, 0), TRUE);
						DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs * int(c.vehicle.HP) / int(c.vehicle.use_veh.HP), yp + ys / 2 + ys * 2 / 3, GetColor(0, 255, 0), TRUE);
						if (c.p_anime_geardown.second >= 0.5f) {
							DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 - y_r(4, out_disp_y)), xp + xs, yp + ys / 2 + ys * 2 / 3, GetColor(127, 127, 127), TRUE);
						}
						/*
						for (auto& h : c.vehicle.HP_m) {
							int p = &h - &c.vehicle.HP_m[0];

							DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 + y_r(4 + p * 5, out_disp_y)), xp + xs, yp + ys / 2 + (ys * 2 / 3 + y_r(8 + p * 5, out_disp_y)), GetColor(255, 0, 0), TRUE);
							DrawBox(xp, yp + ys / 2 + (ys * 2 / 3 + y_r(4 + p * 5, out_disp_y)), xp + xs * int(h) / int(c.vehicle.use_veh.HP), yp + ys / 2 + (ys * 2 / 3 + y_r(8 + p * 5, out_disp_y)), GetColor(0, 255, 0), TRUE);
						}
						*/
						font->DrawStringFormat(xp, yp + 18, col, "%d m", int((VECTOR_ref(c.vehicle.pos) - chara.vehicle.pos).size()));

						if (c.id != chara.id) {
							if ((int(c.winpos.x()) < 0 || int(c.winpos.x()) > disp_x || int(c.winpos.y()) < 0 || int(c.winpos.y()) > disp_y) || (&c - &charas[0] != int(id_near))) {
								auto rad = atan2f(float(int(c.winpos.x()) - disp_x / 2), float(int(c.winpos.y()) - disp_y / 2));
								auto dis = std::clamp((c.vehicle.pos - chara.vehicle.pos).size() / 10.f, 0.f, 200.f);
								DrawLine(
									disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (200 - dis) / 200))),
									disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (200 - dis) / 200))),
									disp_x / 2 + int(200.f*sin(rad)),
									disp_y / 2 + int(200.f*cos(rad)),
									col, 2);
								DrawLine(
									disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (200 - dis) / 200))),
									disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (200 - dis) / 200))),
									disp_x / 2 + int(200.f*sin(rad)),
									disp_y / 2 + int(200.f*cos(rad)),
									col, 2);
								DrawLine(
									disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (200 - dis) / 200))),
									disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (200 - dis) / 200))),
									disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (200 - dis) / 200))),
									disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (200 - dis) / 200))),
									col, 2);
							}
							if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
								if (c.winpos_if.z() >= 0.f && c.winpos_if.z() <= 1.f) {
									siz_per = sqrtf(powf(c.winpos_if.x() - aimpos.x(), 2) + powf(c.winpos_if.y() - aimpos.y(), 2)) / float(disp_x / 2);


									//DrawLine(int(c.winpos.x()), int(c.winpos.y()), int(c.winpos_if.x()), int(c.winpos_if.y()), col);
									auto dist = std::clamp(sqrtf(powf((c.winpos.x() - c.winpos_if.x()), 2) + powf((c.winpos.y() - c.winpos_if.y()), 2)), 0.f, 42.f);
									auto rad = atan2f((c.winpos.x() - c.winpos_if.x()), (c.winpos.y() - c.winpos_if.y()));
									DrawLine(
										int(c.winpos_if.x()) + int(dist*sin(rad)),
										int(c.winpos_if.y()) + int(dist*cos(rad)),
										int(c.winpos_if.x()),// + int(200.f*sin(rad)),
										int(c.winpos_if.y()),// + int(200.f*cos(rad)),
										col);

									if (siz_per <= 1.f / 3.f) {
										aim_if.DrawRotaGraph(int(c.winpos_if.x()), int(c.winpos_if.y()), 1.f, 0.f, TRUE);
									}
								}
							}
						}
					}
				}
			}

			if (int(id_near) != &chara - &charas[0]) {
				auto&c = charas[id_near];
				if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f) {
					auto rad = atan2f(float(int(c.winpos.x()) - disp_x / 2), float(int(c.winpos.y()) - disp_y / 2));
					auto dis2 = std::clamp(sqrtf(powf(float(int(c.winpos.x()) - disp_x / 2), 2.f) + powf(float(int(c.winpos.y()) - disp_y / 2), 2.f)), 0.f, 200.f);
					auto dis = std::clamp((c.vehicle.pos - chara.vehicle.pos).size() / 10.f, 0.f, dis2);
					DrawLine(
						disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (200 - dis) / 200))),
						disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (200 - dis) / 200))),
						disp_x / 2 + int(dis2*sin(rad)),
						disp_y / 2 + int(dis2*cos(rad)),
						GetColor(255, 255, 0), 2);
					DrawLine(
						disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (200 - dis) / 200))),
						disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (200 - dis) / 200))),
						disp_x / 2 + int(dis2*sin(rad)),
						disp_y / 2 + int(dis2*cos(rad)),
						GetColor(255, 255, 0), 2);
					DrawLine(
						disp_x / 2 + int(dis*sin(rad + deg2rad(10 * (200 - dis) / 200))),
						disp_y / 2 + int(dis*cos(rad + deg2rad(10 * (200 - dis) / 200))),
						disp_x / 2 + int(dis*sin(rad - deg2rad(10 * (200 - dis) / 200))),
						disp_y / 2 + int(dis*cos(rad - deg2rad(10 * (200 - dis) / 200))),
						GetColor(255, 255, 0), 2);
				}
			}
		}
		else {
			auto& c = charas[chara.vehicle.DEATH_ID];
			int siz = int(32.f);
			float siz_per = 1.f;

			unsigned int col = 0;

			if (c.id == chara.id) {
				col = GetColor(0, 255, 0);
			}
			else {
				col = GetColor(255, 0, 0);
			}

			if (c.winpos.z() >= 0.f && c.winpos.z() <= 1.f && !c.death) {
				siz_per = std::clamp((1.f - sqrtf(powf(c.winpos.x() - disp_x / 2, 2) + powf(c.winpos.y() - disp_y / 2, 2)) / float(disp_x / 2)), 0.f, 1.f);
				siz = int(32.f*siz_per);
				DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), col, FALSE);
				siz = int(42.f*siz_per);
				DrawBox(int(c.winpos.x()) - y_r(siz, out_disp_y), int(c.winpos.y()) - y_r(siz, out_disp_y), int(c.winpos.x()) + y_r(siz, out_disp_y), int(c.winpos.y()) + y_r(siz, out_disp_y), col, FALSE);
			}
		}

	}

	void res_draw(Mainclass::Chara& chara, bool uses_vr = true) {
		int yyy = 0;
		int xs = 0, xp = 0, ys = 0, yp = 0;
		FontHandle* font = (!uses_vr) ? &font24 : &font36;
		auto ysize = (!uses_vr) ? y_r(24, out_disp_y) : y_r(36, out_disp_y);
		yyy += ysize;
		if (aimpos_3.z() >= 0.f && aimpos_3.z() <= 1.f) {
			xp = (int)(aimpos_3.x());
			yp = (int)(aimpos_3.y());
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
			font->DrawStringFormat_RIGHT(xp + disp_x / 6, yp + disp_y / 6 - ysize, GetColor(255, 255, 255), "%s",
				(chara.vehicle.KILL_COUNT < 3) ? "Airman" :
				((chara.vehicle.KILL_COUNT < 5) ? "Senior Master" :
				((chara.vehicle.KILL_COUNT < 7) ? "Captain" :
					((chara.vehicle.KILL_COUNT < 10) ? "Major" : "General"))));
		}
	}
};
