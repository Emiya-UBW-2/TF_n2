#pragma once
class main_c {
protected:
	cam_info cam_mine, cam_view;
	VECTOR_ref eyezvec, eyeyvec;	//視点
	//描画スクリーン
	GraphHandle UI_Screen;			//
	MV1 cockpit;					//コックピット
	MV1 human;						//コックピット
	MV1 garage;
	//操作
	Mainclass::CAMS cam_s;
	float range = 0.f, range_p = 30.f;
	float fovs = 1.f, fovs_p = 1.f;
	VECTOR_ref eye_pos_ads;
	//データ
	std::vector<Mainclass::Chara> chara;	//キャラ
	std::vector<Mainclass::Ammos> Ammo;		//弾薬
	std::vector<Mainclass::Vehcs> Vehicles;	//車輛データ
	//サウンド
	Mainclass::ses_ se;
	Mainclass::bgms_ bgm;
	Mainclass::voices_ voice;

	SoundHandle se_alert;
	SoundHandle se_alert2;
	SoundHandle se_change;
	SoundHandle se_timer;
	SoundHandle bgm_title;
	SoundHandle bgm_win;
	//timer
	float timer = 0.f;
	float ready_timer = 0.f;
	bool ending_win = true;
	//
	bool start_c2 = true;
	bool ending = true;
	float danger_height = 300.f;
	//設定
	float fov_pc = 45.f;
	float bgm_vol = 0.5f;
	float se_vol = 0.35f;
	float vc_vol = 1.f;
	//キー
	Mainclass::key_bind k_;
	Mainclass::views_ view_;
	//sumapo
	std::unique_ptr<DXDraw, std::default_delete<DXDraw>> Drawparts;
	std::unique_ptr<UI, std::default_delete<UI>> UIparts;
	std::unique_ptr<DeBuG, std::default_delete<DeBuG>> Debugparts;
	std::unique_ptr<HostPassEffect, std::default_delete<HostPassEffect>> Hostpassparts;
	std::unique_ptr<Mapclass, std::default_delete<Mapclass>> mapparts;
public:
	main_c() {
		//設定読み込み
		{
			bool dof_e = false;
			bool bloom_e = false;
			bool shadow_e = false;
			bool useVR_e = true;

			SetOutApplicationLogValidFlag(FALSE);	//log
			int mdata = FileRead_open("data/setting.txt", FALSE);
			dof_e = getparams::_bool(mdata);
			bloom_e = getparams::_bool(mdata);
			shadow_e = getparams::_bool(mdata);
			useVR_e = getparams::_bool(mdata);
			fov_pc = getparams::_float(mdata);
			FileRead_close(mdata);
			SetOutApplicationLogValidFlag(TRUE);	//log
			Drawparts = std::make_unique<DXDraw>("TankFlanker", FRAME_RATE, useVR_e, shadow_e);						//汎用クラス
			UIparts = std::make_unique<UI>(Drawparts->disp_x, Drawparts->disp_y);										//UI
			Debugparts = std::make_unique<DeBuG>(FRAME_RATE);															//デバッグ
			Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);	//ホストパスエフェクト
			mapparts = std::make_unique<Mapclass>();																	//map
			UI_Screen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);										//VR、フルスクリーン共用UI
		}

		FontHandle font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);

		//サウンド読み込み
		voice.load();
		se.Load();
		bgm.Load();

		se_alert = SoundHandle::Load("data/audio/alert.wav");
		se_alert2 = SoundHandle::Load("data/audio/alert2.wav");
		se_change = SoundHandle::Load("data/audio/change.wav");
		se_timer = SoundHandle::Load("data/audio/timer.wav");
		bgm_title = SoundHandle::Load("data/audio/BGM/title.wav");
		bgm_win = SoundHandle::Load("data/audio/BGM/win.wav");
		//キー読み込み
		k_.load_keyg();
		//機体モデル読み込み
		Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles, true);
		MV1::Load("data/model/cockpit/model.mv1", &cockpit, true);
		MV1::Load("data/model/human/model.mv1", &human, true);
		MV1::Load("data/model/garage/model.mv1", &garage, true);
		UIparts->load_window("車両モデル");					//ロード画面
		//機体データ読み込み
		Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
		Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
		//ラムダ
		auto shadow_draw_menu = [&]() {
			Vehicles[chara[0].vehicle.use_id].obj.DrawModel();
		};
		auto ram_draw_menu = [&]() {
			//マップ
			garage.DrawModel();
			//機体
			Vehicles[chara[0].vehicle.use_id].obj.DrawModel();
		};
		//ラムダ2
		auto shadow_draw = [&]() {
			for (auto& c : chara) {
				c.vehicle.draw();
			}
		};
		auto ram_draw = [&]() {
			//マップ
			{
				//地形
				mapparts->map_draw();
				//海
				mapparts->sea_draw();
				//雲
				mapparts->cloud_draw();
			}
			//機体
			{
				SetFogStartEnd(0.0f, 3000.f);
				SetFogColor(128, 128, 128);
				for (auto& c : chara) {
					MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY);
					c.vehicle.draw();
					MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_SEMITRANS_ONLY);
					c.vehicle.draw();
				}
				MV1SetSemiTransDrawMode(DX_SEMITRANSDRAWMODE_ALWAYS);
				//弾
				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				for (auto& c : chara) {
					for (auto& cg : c.vehicle.Gun_) {
						cg.draw();
					}
				}
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);
			}
		};
		//
		auto mouse_aim = [&](VECTOR_ref& eyevec_tmp) {
			int mousex, mousey;
			GetMousePoint(&mousex, &mousey);
			SetMousePoint(deskx / 2, desky / 2);

			float y = atan2f(eyevec_tmp.x(), eyevec_tmp.z()) + deg2rad(float(mousex - deskx / 2) * 0.1f / fovs);
			float x = atan2f(eyevec_tmp.y(), std::hypotf(eyevec_tmp.x(), eyevec_tmp.z())) + deg2rad(float(mousey - desky / 2) * 0.1f / fovs);
			x = std::clamp(x, deg2rad(-80), deg2rad(45));
			eyevec_tmp = VGet(cos(x) * std::sin(y), std::sin(x), std::cos(x) * std::cos(y));
		};
		//ココから繰り返し読み込み//-------------------------------------------------------------------
		do {
			//読み出し
			chara.resize(24);
			auto& mine = chara[0];
			//キャラ選択
			Drawparts->reset_HMD();
			start_c2 = false;
			{
				//
				float speed = 0.f;
				VECTOR_ref pos;
				pos = VGet(0, 1.8f, 0);
				bool endp = false;
				float rad = 0.f, ber_r = 0.f;
				float yrad_m = 0.f, xrad_m = 0.f;
				float yrad_im = 0.f, xrad_im = 0.f;
				int anime = 0;
				//VR
				switchs rt, lt;
				//
				int m_x = 0, m_y = 0;
				{
					//マップ読み込み
					SetGlobalAmbientLight(GetColorF(0.5f, 0.475f, 0.45f, 1.f));
					//光、影
					Drawparts->Set_Light_Shadow(VGet(10.f, 10.f, 10.f), VGet(-10.f, -10.f, -10.f), VGet(0.0f, -0.5f, 0.5f), [&] {});
					//飛行機
					mine.vehicle.use_id %= Vehicles.size();
					auto&veh_t = Vehicles[mine.vehicle.use_id];
					anime = MV1AttachAnim(veh_t.obj.get(), 1);
					MV1SetAttachAnimBlendRate(veh_t.obj.get(), anime, 1.f);
					{
						veh_t.obj.SetMatrix(MGetIdent());
						pos.y(0.f);
						for (auto& w : veh_t.wheelframe) {
							auto p = -veh_t.obj.frame(w.frame.first + 1).y() + 0.2f;
							if (p >= pos.y()) {
								pos.y(p);
							}
						}
					}
					veh_t.obj.SetMatrix(MATRIX_ref::Mtrans(pos));
					GetMousePoint(&m_x, &m_y);
					cam_s.cam.campos = VGet(0.f, 0.f, -15.f);
					cam_s.cam.camvec = VGet(0.f, 3.f, 0.f);
					fovs = 1.f;
					bgm_title.play(DX_PLAYTYPE_LOOP, TRUE);
					bgm_title.vol(int(float(255)*bgm_vol));
				}
				//
				while (ProcessMessage() == 0) {
					{
						auto& veh = mine.vehicle;
						if (!start_c2) {
							if (Drawparts->use_vr) {
								auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
								if (&ptr_LEFTHAND != nullptr) {
									if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
										//
										{
											lt.get_in(((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() < -0.5f);
											rt.get_in(((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) && ptr_LEFTHAND.touch.x() > 0.5f);
											if (lt.push()) {
												MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
												++veh.use_id %= Vehicles.size();
												anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
												MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
												{
													Vehicles[veh.use_id].obj.SetMatrix(MGetIdent());
													pos.y(0.f);
													for (auto& w : Vehicles[veh.use_id].wheelframe) {
														auto p = -Vehicles[veh.use_id].obj.frame(w.frame.first + 1).y() + 0.2f;
														if (p >= pos.y()) {
															pos.y(p);
														}
													}
												}
											}
											if (rt.push()) {
												MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
												if (veh.use_id == 0) {
													veh.use_id = Vehicles.size() - 1;
												}
												else {
													--veh.use_id;
												}
												anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
												MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
												{
													Vehicles[veh.use_id].obj.SetMatrix(MGetIdent());
													pos.y(0.f);
													for (auto& w : Vehicles[veh.use_id].wheelframe) {
														auto p = -Vehicles[veh.use_id].obj.frame(w.frame.first + 1).y() + 0.2f;
														if (p >= pos.y()) {
															pos.y(p);
														}
													}
												}
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
									if (k_.key_use_ID[3].get_key(2)) {
										MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
										++veh.use_id %= Vehicles.size();
										anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
										MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
										{
											Vehicles[veh.use_id].obj.SetMatrix(MGetIdent());
											pos.y(0.f);
											for (auto& w : Vehicles[veh.use_id].wheelframe) {
												auto p = -Vehicles[veh.use_id].obj.frame(w.frame.first + 1).y() + 0.2f;
												if (p >= pos.y()) {
													pos.y(p);
												}
											}
										}
									}
									if (k_.key_use_ID[2].get_key(2)) {
										MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
										if (veh.use_id == 0) {
											veh.use_id = Vehicles.size() - 1;
										}
										else {
											--veh.use_id;
										}
										anime = MV1AttachAnim(Vehicles[veh.use_id].obj.get(), 1);
										MV1SetAttachAnimBlendRate(Vehicles[veh.use_id].obj.get(), anime, 1.f);
										{
											Vehicles[veh.use_id].obj.SetMatrix(MGetIdent());
											pos.y(0.f);
											for (auto& w : Vehicles[veh.use_id].wheelframe) {
												auto p = -Vehicles[veh.use_id].obj.frame(w.frame.first + 1).y() + 0.2f;
												if (p >= pos.y()) {
													pos.y(p);
												}
											}
										}
									}
								}
								//
								if (CheckHitKey(KEY_INPUT_SPACE) != 0) {
									start_c2 = true;
								}
								//
							}
						}
						else {
							speed = std::clamp(speed + 1.5f / 3.6f / GetFPS(), 0.f, 20.f / 3.6f / GetFPS());
							pos.zadd(-speed);

							if (pos.z() <= -15.f) {
								easing_set(&fovs, 1.75f, 0.95f);
							}
							if (pos.z() < -20.f) {
								endp = true;
							}
						}
						for (auto& b : Vehicles[veh.use_id].burner) {
							Vehicles[veh.use_id].obj.SetFrameLocalMatrix(b.first, MATRIX_ref::Scale(VGet(1.f, 1.f, 0.f)) * MATRIX_ref::Mtrans(b.second));
						}
						Vehicles[veh.use_id].obj.SetMatrix(MATRIX_ref::Mtrans(pos));
					}
					//視点取得
					if (Drawparts->use_vr) {
						VECTOR_ref HMDpos;
						MATRIX_ref HMDmat;
						Drawparts->GetHMDPositionVR(&HMDpos, &HMDmat);
						cam_s.cam.campos = VECTOR_ref(VGet(15.f, 0, 0)) + HMDpos;
						cam_s.cam.camvec = cam_s.cam.campos - HMDmat.zvec();
						cam_s.cam.camup = HMDmat.yvec();
					}
					else {
						if (!start_c2) {
							easing_set(&cam_s.cam.campos, (MATRIX_ref::RotX(xrad_m) * MATRIX_ref::RotY(yrad_m)).zvec() * (-15.f) + VGet(0.f, 3.f, 0.f), 0.95f);
							cam_s.cam.camvec = pos + VGet(0.f, 3.f, 0.f);
						}
						else {
							easing_set(&cam_s.cam.campos, VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 3.f, (1.f - (pos.z() / -120.f)) + 10.f), 0.95f);
							easing_set(&cam_s.cam.camvec, pos + VGet((1.f - (pos.z() / -120.f)), (1.f - (pos.z() / -120.f)) + 1.f, (1.f - (pos.z() / -120.f))), 0.95f);
						}
						cam_s.cam.camup = VGet(0.f, 1.f, 0.f);
					}
					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos, shadow_draw_menu, VGet(100.f, 100.f, 100.f), VGet(100.f, 100.f, 100.f));
					//VR更新
					Drawparts->Move_Player();
					//自機描画
					{
						auto& veh = mine.vehicle;
						//cam_s.cam
						{
							cam_s.cam.set_cam_info(deg2rad(Drawparts->use_vr ? 90 : fov_pc / fovs), 0.1f, 300.f);
						}
						//UI
						UI_Screen.SetDraw_Screen();
						{
							UIparts->draw_menu(ber_r, rad, Vehicles[veh.use_id]);
							easing_set(&ber_r, float(Drawparts->disp_y / 4), 0.95f);
							easing_set(&rad, deg2rad(yrad_im), 0.9f);
						}
						//VRに移す
						Drawparts->draw_VR(
							[&] {
							auto tmp = GetDrawScreen();
							auto tmp_cams = cam_s;
							{
								auto camtmp = VECTOR_ref(GetCameraPosition()) - cam_s.cam.campos;
								auto vectmp = (VECTOR_ref(cam_s.cam.camvec) - cam_s.cam.campos);
								camtmp = MATRIX_ref::Vtrans(camtmp, MATRIX_ref::Axis1(vectmp.cross(cam_s.cam.camup), cam_s.cam.camup, vectmp));
								tmp_cams.cam.campos = camtmp + cam_s.cam.campos;
								tmp_cams.cam.camvec = camtmp + cam_s.cam.camvec;
							}
							//被写体深度描画
							Hostpassparts->BUF_draw([&]() {}, [&]() { Drawparts->Draw_by_Shadow(ram_draw_menu); }, tmp_cams.cam);
							//最終描画
							Hostpassparts->MAIN_draw();

							GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
							{
								Hostpassparts->get_main().DrawGraph(0, 0, false);

								SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(255 - int(255.f * pos.z() / -10.f), 0, 255));
								UI_Screen.DrawGraph(0, 0, true);
								SetDrawBlendMode(DX_BLENDMODE_ALPHA, std::clamp(int(255.f * (pos.z() + 15.f) / -5.f), 0, 255));
								DrawBox(0, 0, Drawparts->disp_x, Drawparts->disp_y, GetColor(255, 255, 255), TRUE);
								SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
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
					Drawparts->Screen_Flip();
					//
					if (k_.key_use_ID[11].get_key(0)) {
						break;
					}
					if (endp) {
						WaitTimer(100);
						break;
					}
				}
				if (k_.key_use_ID[11].get_key(0)) {
					break;
				}
				//終了
				{
					auto& veh = mine.vehicle;
					MV1DetachAnim(Vehicles[veh.use_id].obj.get(), anime);
					bgm_title.stop();
					Drawparts->Delete_Shadow();
				}
			}
			//開始
			Drawparts->reset_HMD();
			start_c2 = true;
			{
				{
					//マップ読み込み
					mapparts->set_pre();
					UIparts->load_window("マップモデル");			   //ロード画面
					mapparts->set("data/grassput.bmp", VGet(0.0f, -0.5f, 0.5f), GetColorF(0.5f, 0.475f, 0.45f, 1.f), 35000.f, 35000.f, -35000.f, -35000.f);
					//光、影
					Drawparts->Set_Light_Shadow(mapparts->mesh_maxpos(0), mapparts->mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f),
						[&] {
						mapparts->map_draw();
						mapparts->cloud_draw();
					});
					//共通
					for (auto& c : chara) {
						size_t i = &c - &chara[0];
						//敵味方
						c.type = (i >= (chara.size() / 2)) ? 1 : 0;
						//機体選択
						c.vehicle.use_id = (i != 0) ? GetRand(int(Vehicles.size()) - 1) : c.vehicle.use_id;
						//set
						c.set_human(Vehicles, Ammo);
						//コックピット
						c.cocks.set_(cockpit,human);
						//se
						c.se.Duplicate(se);
						//spawn
						/*
						{
							float rad = deg2rad(-130);
							c.vehicle.spawn(VGet(float(-3200)*sin(rad) + float(0)*cos(rad), 10.f, float(-3200)*cos(rad) - float(0)*sin(rad)), MATRIX_ref::RotY(deg2rad(((c.type == 0) ? 180 : 0) - 130)), 0.f, 0.f);
						}
						//*/
						{
							float rad = deg2rad(-130);
							auto x_ = int(i / (chara.size() / 2));
							auto y_ = int(i % (chara.size() / 2));
							c.vehicle.spawn(VGet(float(4000 * x_ - 2000)*sin(rad) + float(100 * y_)*cos(rad), 1500.f, float(4000 * x_ - 2000)*cos(rad) - float(100 * y_)*sin(rad)), MATRIX_ref::RotY(deg2rad(((c.type == 0) ? 180 : 0) - 130)), 25.f, c.vehicle.use_veh.min_speed_limit*3.6f);
						}
						//
					}
					//音量調整
					se_alert.vol(int(float(192)*se_vol));
					se_alert2.vol(int(float(192)*se_vol));
					se_change.vol(int(float(728)*se_vol));
					for (auto& c : chara) {
						c.se.setinfo(se_vol);
					}
					bgm_win.vol(int(float(255)*bgm_vol));
					//se
					for (auto& c : chara) {
						c.se.cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
						c.se.engine.play(DX_PLAYTYPE_LOOP, TRUE);
					}
					//bgm
					bgm.play(bgm_vol, -1);
					//voice
					voice.play(0, 0.f, vc_vol);
					//mouse
					SetMouseDispFlag(FALSE);
					SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
					//timer
					timer = 60.f*5.f;
					ready_timer = 5.f;
					//cam
					eyezvec = mine.vehicle.mat.zvec() * -1.f;
					cam_s.cam.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyezvec * range;
					cam_s.Rot = ADS;
					view_.init();
					//
					ending_win = true;
					//
				}
				//
				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					Debugparts->put_way();
					//
					for (auto& c : chara) {
						c.set_alive(se_vol);
					}
					Debugparts->end_way();
					auto old_sel = mine.vehicle.sel_weapon;
					//プレイヤー操作+CPU
					{
						//スコープ
						{
							cam_s.Rot = std::clamp(cam_s.Rot + GetMouseWheelRotVol(), 0, 3);
							if (Drawparts->use_vr) {
								switch (cam_s.Rot) {
								case 2:
									fovs_p = 1.f;
									break;
								case 3:
									fovs_p = 2.f;
									break;
								default:
									cam_s.Rot = ADS;
									fovs_p = 1.f;
									break;
								}
							}
							else {
								switch (cam_s.Rot) {
								case 1:
									range_p = 15.f;
									break;
								case 0:
									range_p = 30.f;
									break;
								case 2:
									fovs_p = 1.f;
									break;
								case 3:
									fovs_p = 2.f;
									break;
								default:
									cam_s.Rot = ADS;
									fovs_p = 1.f;
									break;
								}
							}
							easing_set(&range, range_p, 0.9f);
							easing_set(&fovs, fovs_p, 0.9f);
						}
						//見回し
						if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
							if (Drawparts->use_vr) {
								chara[1].view_yrad = 0.f;
								chara[1].view_xrad = 0.f;
							}
							else {
								mine.view_yrad = 0.f;
								mine.view_xrad = 0.f;
							}
						}
						//キー
						{
							//cpu
							for (auto& c : chara) {
								if (&c - &chara[0] >= (Drawparts->use_vr ? 1 : 1)) {
									c.cpu_doing(&chara);
								}
							}
							//VR専用
							if (Drawparts->use_vr) {
								std::for_each(mine.key.begin(), mine.key.end(), [](bool& g) {g = false; });

								auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
								if (&ptr_LEFTHAND != nullptr) {
									if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
										//メイン武器
										mine.key[0] = mine.key[0] || ((ptr_LEFTHAND.on[0] & BUTTON_TRIGGER) != 0);
										//サブ武器
										mine.key[1] = mine.key[1] || ((ptr_LEFTHAND.on[1] & BUTTON_TOPBUTTON_B) != 0);
										//ピッチ
										mine.key[2] = mine.key[2] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(24)));
										mine.key[3] = mine.key[3] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-18)));
										//ロール
										mine.key[4] = mine.key[4] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(20)));
										mine.key[5] = mine.key[5] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-20)));
										if ((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) {
											//ヨー
											mine.key[6] = mine.key[6] || (ptr_LEFTHAND.touch.x() > 0.5f);
											mine.key[7] = mine.key[7] || (ptr_LEFTHAND.touch.x() < -0.5f);
											//スロットル
											mine.key[8] = mine.key[8] || (ptr_LEFTHAND.touch.y() > 0.5f);
											mine.key[9] = mine.key[9] || (ptr_LEFTHAND.touch.y() < -0.5f);
											//ブレーキ
											if ((ptr_LEFTHAND.touch.x() >= -0.5f) && (ptr_LEFTHAND.touch.x() <= 0.5f) && (ptr_LEFTHAND.touch.y() >= -0.5f) && (ptr_LEFTHAND.touch.y() <= 0.5f)) {
												mine.key[11] = mine.key[11] || true;
											}
										}
										//脚
										mine.key[10] = false;
										//精密操作
										{
											//ピッチ
											mine.key[12] = mine.key[12] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(14)));
											mine.key[13] = mine.key[13] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-8)));
											//ロール
											mine.key[14] = mine.key[14] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(12)));
											mine.key[15] = mine.key[15] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-12)));
											if ((ptr_LEFTHAND.on[0] & BUTTON_TOUCHPAD) != 0) {
												//ヨー
												mine.key[16] = mine.key[16] || (ptr_LEFTHAND.touch.x() > 0.45f);
												mine.key[17] = mine.key[17] || (ptr_LEFTHAND.touch.x() < -0.45f);
											}
										}
										//
									}
								}
							}
							//通常
							else {
								k_.set(mine);
								//ロックオン外し
								if (k_.key_use_ID[12].get_key(0)) {
									UIparts->reset_lock();
								}
								//武装切り替え
								if (k_.key_use_ID[14].get_key(2)) {
									++mine.vehicle.sel_weapon;
									mine.vehicle.sel_weapon %= (mine.vehicle.Gun_.size() - 1);
								}
							}
							for (auto& c : chara) {
								if (c.death) {
									std::for_each(c.key.begin(), c.key.end(), [](bool& g) {g = false; });
								}
								//
								if (ready_timer >= 0.f || timer <= 0.f) {
									c.key[0] = false;
									c.key[1] = false;
								}
								//
								if (&c != &mine && (c.type == mine.type)) {
									if (c.vehicle.HP > 0) {
										if (c.key[0] && GetRand(100) < 10) {
											voice.play(1, float(GetRand(170) + 30) / 10.f, vc_vol);
										}
										if (c.key[1] && GetRand(100) < 10) {
											voice.play(2, float(GetRand(170) + 30) / 10.f, vc_vol);
										}
										if (c.vehicle.hitf) {
											voice.play(3, float(GetRand(170) + 30) / 10.f, vc_vol);
											c.vehicle.hitf = false;
										}
										if (c.vehicle.killf) {
											voice.play(4, 0.f, vc_vol);
											c.vehicle.killf = false;
										}
										if (c.aim_cnt > 0 && GetRand(100) < 10) {
											voice.play(5, float(GetRand(170) + 30) / 10.f, vc_vol);
										}
										if (c.missile_cnt > 0 && GetRand(100) < 10) {
											voice.play(6, float(GetRand(170) + 30) / 10.f, vc_vol);
										}
										if (c.vehicle.dmgf) {
											voice.play(7, float(GetRand(170) + 30) / 10.f, vc_vol);
											c.vehicle.dmgf = false;
										}
									}
									if (c.vehicle.deathf) {
										voice.play(8, 0.f, vc_vol);
										c.vehicle.deathf = false;
									}
								}
								//
							}
							//
							voice.update();
							//
						}
						//マウスと視点角度をリンク
						if (Drawparts->use_vr) {
							//+視点取得
							VECTOR_ref HMDpos;
							MATRIX_ref HMDmat;
							Drawparts->GetHMDPositionVR(&HMDpos, &HMDmat);
							eye_pos_ads = VGet(std::clamp(HMDpos.x(), -0.18f, 0.18f), std::clamp(HMDpos.y() - 0.42f, 0.f, 0.8f), std::clamp(HMDpos.z(), -0.36f, 0.1f));
							eyezvec = HMDmat.zvec();
							eyeyvec = HMDmat.yvec();
						}
						else {
							eye_pos_ads = VGet(0, 0.68f, -0.1f);
							mouse_aim(eyezvec);
						}
					}
					//反映
					Debugparts->end_way();
					for (auto& c : chara) {
						//反映
						c.update(mapparts, Drawparts, &chara, start_c2);
						//サウンド
						c.se.setpos(c.vehicle.pos);
					}
					start_c2 = false;
					Debugparts->end_way();
					//換装音
					if (old_sel != mine.vehicle.sel_weapon) {
						se_change.play(DX_PLAYTYPE_BACK, TRUE);
					}
					//アラート
					{
						bool ttttt = false;
						if (mine.vehicle.speed < mine.vehicle.use_veh.min_speed_limit) {
							if (!se_alert2.check()) {
								se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (mine.missile_cnt > 0) {
							if (!se_alert2.check()) {
								se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (!ttttt) {
							se_alert2.stop();
						}
						ttttt = false;
						if (mine.vehicle.pos.y() <= danger_height) {
							if (!se_alert.check()) {
								se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (mine.aim_cnt > 0) {
							if (!se_alert.check()) {
								se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
							}
							ttttt = true;
						}
						if (!ttttt) {
							se_alert.stop();
						}
					}
					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos, shadow_draw, VGet(100.f, 100.f, 100.f), VGet(1000.f, 1000.f, 1000.f));
					//VR更新
					Drawparts->Move_Player();
					//描画用意
					mapparts->sea_draw_set();
					//自機描画
					{
						auto& veh = mine.vehicle;
						//cam_s.cam
						{
							//プレイヤー視点
							if (k_.key_use_ID[15].get_key(2)) {
								view_.use ^= 1;
							}
							{
								//campos,camvec,camup取得
								if (cam_s.Rot >= ADS) {
									cam_mine.camvec -= cam_mine.campos;
									cam_mine.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
									cam_mine.campos.y(std::max(cam_mine.campos.y(), 0.f));
									if (Drawparts->use_vr) {
										cam_mine.camvec = cam_mine.campos - MATRIX_ref::Vtrans(eyezvec, veh.mat);
										cam_mine.camup = MATRIX_ref::Vtrans(eyeyvec, veh.mat);//veh.mat.yvec();
									}
									else {
										if (!((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)) {
											eyezvec = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
										}
										easing_set(&cam_mine.camvec, MATRIX_ref::Vtrans(eyezvec, veh.mat)*-1.f, 0.75f);
										cam_mine.camvec += cam_mine.campos;
										cam_mine.camup = veh.mat.yvec();
									}
								}
								else {
									cam_mine.campos -= cam_mine.camvec;

									cam_mine.camvec = veh.pos + veh.mat.yvec() * (6.f);
									cam_mine.camvec.y(std::max(cam_mine.camvec.y(), 0.f));

									if (((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)) {
										easing_set(&cam_mine.campos, eyezvec * range, 0.9f);
										cam_mine.campos += cam_mine.camvec;
										cam_mine.campos.y(std::max(cam_mine.campos.y(), 0.f));
										mapparts->col_line_nearest(cam_mine.camvec, &cam_mine.campos);

										easing_set(&cam_mine.camup, VGet(0.f, 1.f, 0.f), 0.9f);
									}
									else {
										cam_mine.camvec = veh.pos + veh.mat.yvec() * (6.f);
										cam_mine.camvec.y(std::max(cam_mine.camvec.y(), 0.f));

										eyezvec = (cam_mine.camvec - (mine.vehicle.pos + mine.vehicle.mat.zvec() * (-1000.f))).Norm();
										cam_mine.campos = cam_mine.camvec + eyezvec * range;

										easing_set(&cam_mine.camup, veh.mat.yvec(), 0.9f);
									}
								}
								//
								cam_mine.set_cam_info(deg2rad(Drawparts->use_vr ? 90 : fov_pc / fovs), (cam_s.Rot >= ADS) ? (3.f) : (range_p - 5.f), (cam_s.Rot >= ADS) ? (1000.f) : (60.f*(range_p - 5.f)));
							}
							view_.update(cam_mine, cam_view, deg2rad(Drawparts->use_vr ? 90 : fov_pc), mine, ready_timer);
							cam_s.cam = view_.use ? cam_view : cam_mine;
						}
						//
						Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
						//コックピット演算
						if (cam_s.Rot >= ADS) {
							mine.cocks.ready_(mine, cam_mine.campos);
						}
						//UI
						if (!view_.use && view_.on) {
							UI_Screen.SetDraw_Screen();
							{
								UIparts->draw(mine, cam_s.Rot >= ADS, *Drawparts->get_device_hand1(), danger_height, Drawparts->use_vr);
							}
						}
						//VRに移す
						Drawparts->draw_VR(
							[&] {
							auto tmp = GetDrawScreen();
							auto tmp_cams = cam_s;
							{
								auto camtmp = VECTOR_ref(GetCameraPosition()) - cam_s.cam.campos;
								auto tvec = (VECTOR_ref(cam_s.cam.camvec) - cam_s.cam.campos);
								camtmp = MATRIX_ref::Vtrans(camtmp, MATRIX_ref::Axis1(tvec.cross(cam_s.cam.camup), cam_s.cam.camup, tvec));
								tmp_cams.cam.campos = camtmp + cam_s.cam.campos;
								tmp_cams.cam.camvec = camtmp + cam_s.cam.camvec;
							}
							//被写体深度描画
							Hostpassparts->BUF_draw([&]() { mapparts->sky_draw(); }, [&]() { Drawparts->Draw_by_Shadow(ram_draw); }, tmp_cams.cam);
							//最終描画
							Hostpassparts->MAIN_draw();

							GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
							{
								Hostpassparts->get_main().DrawGraph(0, 0, false);
								SetCameraNearFar(0.01f, 2.f);
								//コックピット
								if (tmp_cams.Rot >= ADS) {
									mine.cocks.obj.DrawModel();
									mine.cocks.humen.DrawModel();
								}
								//UI
								if (!view_.use && view_.on) {
									//弾情報
									for (auto& c : chara) {
										for (auto& g : c.vehicle.Gun_) {
											g.update_bullet();
										}
									}
									//画面依存系描画
									UIparts->item_draw(chara, mine, tmp_cams.Rot >= ADS, danger_height, Drawparts->use_vr);
									//
									if (Drawparts->use_vr) {
										SetUseZBuffer3D(FALSE);												//zbufuse
										SetWriteZBuffer3D(FALSE);											//zbufwrite
										DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, 1.8f, 0.f, UI_Screen.get(), TRUE);
										SetUseZBuffer3D(TRUE);												//zbufuse
										SetWriteZBuffer3D(TRUE);											//zbufwrite
									}
									else {
										this->UI_Screen.DrawGraph(0, 0, TRUE);
									}
									//タイマー
									UIparts->timer_draw(timer, ready_timer, Drawparts->use_vr);
									//
									{
										int yyy = Drawparts->disp_y / 36;
										for (auto& v : voice.get_voice_str()) {
											if (v.handle->check()) {
												auto wide = font18.GetDrawWidth(v.str);
												SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
												DrawBox(Drawparts->disp_x / 2 - wide / 2, yyy - 2, Drawparts->disp_x / 2 + wide / 2, yyy + 18 + 2, GetColor(0, 0, 0), TRUE);
												SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

												font18.DrawString_MID(Drawparts->disp_x / 2, yyy, v.str, GetColor(100, 150, 255));
												yyy += 24;
											}
										}
									}
									//
									if (timer <= 0.f) {
										//リザルト
										UIparts->res_draw(mine, Drawparts->use_vr);
									}
								}
								//開始暗転
								{
									SetDrawBlendMode(DX_BLENDMODE_ALPHA, int(view_.black*255.f));
									DrawBox(0, 0, Drawparts->disp_x, Drawparts->disp_y, GetColor(0, 0, 0), TRUE);
									SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
								}
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
						//キー
						k_.draw();
						//デバック
						Debugparts->end_way();
						Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
					}
					Drawparts->Screen_Flip();
					//
					if (ready_timer <= 0.f) {
						timer -= 1.f / GetFPS();
						if (timer <= 0.f && ending_win) {
							bgm.stop();
							bgm_win.play(DX_PLAYTYPE_BACK, TRUE);
							ending_win = false;
						}
						if (timer <= 0.f) {
							timer = 0.f;
						}
					}
					else {
						ready_timer -= 1.f / GetFPS();

						view_.use = true;
						if (ready_timer <= 0.f) {
							view_.use = false;
						}
						if ((GetNowHiPerformanceCount() / 100000) % 10 == 0) {
							if (!se_timer.check()) {
								se_timer.play(DX_PLAYTYPE_BACK, TRUE);
							}
						}
					}
					//
					if (k_.key_use_ID[11].get_key(0)) {
						ending = false;
						break;
					}
					if (k_.key_use_ID[10].get_key(0)) {
						break;
					}
				}
				//終了
				{
					SetMouseDispFlag(TRUE);
					SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
					//音声ストップ
					bgm.stop();
					bgm_win.stop();
					for (auto& c : chara) {
						c.se.stop();
					}
					se_alert.stop();
					se_alert2.stop();
					//解放
					for (auto& c : chara) {
						for (auto& t : c.effcs_missile) {
							t.first.handle.Dispose();
						}
						for (auto& t : c.effcs) {
							t.handle.Dispose();
						}
						c.vehicle.Dispose();
					}
					chara.clear();
					mapparts->Dispose();
					Drawparts->Delete_Shadow();
				}
			}
			//
		} while (ProcessMessage() == 0 && ending);
	}
};