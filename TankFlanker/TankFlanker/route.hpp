#pragma once
class main_c : Mainclass {
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;

	DXDraw::cam_info cam_easy;
	std::list<Mainclass::CAMS> rep_cam;
	VECTOR_ref eyevec, eyevec2;																	//視点
	VECTOR_ref aimposout;																		//UIに出力
	FontHandle font12;
	GraphHandle outScreen2;
	MV1 hit_pic;	//弾痕
	Mainclass::cockpits cocks;	//コックピット
	//操作
	Mainclass::CAMS cam_s;
	float range = 0.f, range_p = 30.f;
	VECTOR_ref eye_pos_ads = VGet(0, 0.58f, 0);
	VECTOR_ref HMDpos;
	MATRIX_ref HMDmat;
	VECTOR_ref rec_HMD;
	//データ
	std::vector<Mainclass::Chara> chara;	/*キャラ*/
	std::vector<Mainclass::Ammos> Ammo;		/*弾薬*/
	std::vector<Mainclass::Vehcs> Vehicles;	/*車輛データ*/
	SoundHandle se_cockpit;
	SoundHandle se_gun;
	SoundHandle se_hit;
	//
	bool oldv = false;
	bool start_c = true;
	bool start_c2 = true;
	bool ending = true;
public:
	main_c() {
		//設定読み込み
		{
			SetOutApplicationLogValidFlag(FALSE);	/*log*/
			int mdata = FileRead_open("data/setting.txt", FALSE);
			dof_e = getparams::_bool(mdata);
			bloom_e = getparams::_bool(mdata);
			shadow_e = getparams::_bool(mdata);
			useVR_e = getparams::_bool(mdata);
			FileRead_close(mdata);
		}
		//
		auto Drawparts = std::make_unique<DXDraw>("TankFlanker", FRAME_RATE, useVR_e, shadow_e);													/*汎用クラス*/
		auto UIparts = std::make_unique<UI>(Drawparts->out_disp_x, Drawparts->out_disp_y, Drawparts->disp_x, Drawparts->disp_y, Drawparts->use_vr);	/*UI*/
		auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);																						/*デバッグ*/
		auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);								/*ホストパスエフェクト*/
		auto Hostpass2parts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->out_disp_x, Drawparts->out_disp_y);						/*ホストパスエフェクト*/
		auto mapparts = std::make_unique<Mapclass>();																								/*map*/
		//
		font12 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
		outScreen2 = GraphHandle::Make(Drawparts->out_disp_x, Drawparts->out_disp_y, true);	//描画スクリーン
	//その他
		//
		SetCreate3DSoundFlag(TRUE);
		se_cockpit = SoundHandle::Load("data/audio/fighter-cockpit1.wav");
		se_gun = SoundHandle::Load("data/audio/hit.wav");
		se_hit = SoundHandle::Load("data/audio/hit.wav");
		SetCreate3DSoundFlag(FALSE);
		//
		MV1::Load("data/model/hit/model.mv1", &hit_pic, true);			//弾痕
		Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles, true);
		//
		UIparts->load_window("車両モデル");					//ロード画面
		//
		cocks.set_();			//コックピット
		Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
		Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
		//ココから繰り返し読み込み//-------------------------------------------------------------------
		do {
			oldv = false;
			start_c = true;
			start_c2 = true;
			//
			chara.resize(2);
			//
			for (auto& c : chara) {
				c.se_cockpit = se_cockpit.Duplicate();
				c.se_gun = se_gun.Duplicate();
				c.se_hit = se_hit.Duplicate();
			}
			//キャラ選択
			if (!UIparts->select_window(&chara[0], &Vehicles)) {
				break;
			}
			//マップ読み込み
			mapparts->set_map_pre();
			UIparts->load_window("マップモデル");			   //ロード画面
			//壁
			mapparts->set_map();
			//光、影
			Drawparts->Set_Light_Shadow(mapparts->map_get().mesh_maxpos(0), mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&mapparts] { mapparts->map_get().DrawModel(); });
			//キャラ選択
			{
				for (auto& c : chara) {
					size_t i = &c - &chara[0];
					auto& veh = c.vehicle;
					veh.spawn(
						VGet(
							float(30 * (i / 3))*sin(deg2rad(-130)),
							10.f,
							float(30 * (i / 3))*cos(deg2rad(-130)) + float(30 * (i % 3))
						),
						MATRIX_ref::RotY(deg2rad(-130)));
				}
			}
			//キャラ設定
			for (auto& c : chara) {
				c.set_human(Vehicles, Ammo, hit_pic);//
			}
			//ラムダ
			auto ram_draw = [&]() {
				Drawparts->Draw_by_Shadow(
					[&]() {
					//マップ
					SetFogStartEnd(30000.0f, 60000.f);
					SetFogColor(128, 128, 128);
					{
						mapparts->map_get().DrawModel();
					}
					//海
					mapparts->sea_draw(cam_s.cam.campos);
					//機体
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 128, 128);
					{
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							//戦闘機
							veh.obj.DrawModel();
							//弾痕
							for (auto& h : veh.hit_obj) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);
								}
							}
						}
						mapparts->map_drawtree();
						//

					}
					SetFogEnable(FALSE);
					SetUseLighting(FALSE);
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						for (auto& cg : veh.Gun_) {
							for (auto& a : cg.bullet) {
								if (a.flug) {
									DXDraw::Capsule3D(a.pos, a.repos, (((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - cam_s.cam.campos).size() / 24.f))*4.5f, a.color, GetColor(255, 255, 255));
								}
							}
						}
					}
					SetUseLighting(TRUE);
					SetFogEnable(TRUE);
				}
				);
			};
			//木セット
			mapparts->map_settree();
			//
			{
				//通信開始
				{
				}
				//開始
				auto& mine = chara[0];
				cam_s.Rot = ADS;
				eyevec = mine.vehicle.mat.zvec() * -1.f;
				cam_s.cam.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
				eyevec2 = chara[1].vehicle.mat.zvec() * -1.f;
				for (auto& c : chara) {
					auto& veh = c.vehicle;
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.scale = 0.1f;
					}
				}
				for (auto& c : chara) {
					Set3DRadiusSoundMem(600.0f, c.se_cockpit.get()); //gun
					Set3DRadiusSoundMem(300.0f, c.se_gun.get()); //gun
					Set3DRadiusSoundMem(900.0f, c.se_hit.get()); //gun
					c.se_cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
					c.se_cockpit.vol(64);
				}
				SetMouseDispFlag(FALSE);
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
				while (ProcessMessage() == 0) {
					const auto fps = GetFPS();
					const auto waits = GetNowHiPerformanceCount();
					Debugparts->put_way();
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						//当たり判定リフレッシュ
						if (veh.hit_check) {
							veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
							for (int i = 0; i < veh.col.mesh_num(); i++) {
								veh.col.RefreshCollInfo(-1, i);
							}
							veh.hit_check = false;
						}
					}
					//プレイヤー操作
					{
						//スコープ
						if (Drawparts->use_vr) {
							cam_s.Rot = ADS;
						}
						else {
							cam_s.Rot = std::clamp(cam_s.Rot + GetMouseWheelRotVol(), 0, ADS);
							switch (cam_s.Rot) {
							case 1:
								range_p = 15.f;
								break;
							case 0:
								range_p = 30.f;
								break;
							default:
								cam_s.Rot = ADS;
								break;
							}
							easing_set(&range, range_p, 0.9f);
						}
						//見回し
						if (Drawparts->use_vr) {
							if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
								chara[1].view_yrad = 0.f;
								chara[1].view_xrad = 0.f;
							}
						}
						else {
							if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
								mine.view_yrad = 0.f;
								mine.view_xrad = 0.f;
							}
						}
						//キー
						{
							if (Drawparts->use_vr) {
								chara[1].key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
								chara[1].key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
								chara[1].key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
								chara[1].key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
								chara[1].key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
								chara[1].key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
								//ヨー
								chara[1].key[6] = (CheckHitKey(KEY_INPUT_Q) != 0);
								chara[1].key[7] = (CheckHitKey(KEY_INPUT_E) != 0);
								//スロットル
								chara[1].key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
								chara[1].key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
								//脚
								chara[1].key[10] = (CheckHitKey(KEY_INPUT_C) != 0);
								//ブレーキ
								chara[1].key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
								//精密操作
								chara[1].key[12] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[13] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[14] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[15] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[16] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[17] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
							}
							//通常、VR共通
							if (!Drawparts->use_vr) {
								mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
								mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
								mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
								mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
								mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
								mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
								//ヨー
								mine.key[6] = (CheckHitKey(KEY_INPUT_Q) != 0);
								mine.key[7] = (CheckHitKey(KEY_INPUT_E) != 0);
								//スロットル
								mine.key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
								mine.key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
								//脚
								mine.key[10] = (CheckHitKey(KEY_INPUT_C) != 0);
								//ブレーキ
								mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
								//精密操作
								mine.key[12] = false;
								mine.key[13] = false;
								mine.key[14] = false;
								mine.key[15] = false;
								mine.key[16] = false;
								mine.key[17] = false;
							}
							//VR専用
							if (Drawparts->use_vr) {
								mine.key[0] = false;   //射撃
								mine.key[1] = false; //マシンガン
								mine.key[2] = false;
								mine.key[3] = false;
								mine.key[4] = false;
								mine.key[5] = false;
								//ヨー
								mine.key[6] = false;
								mine.key[7] = false;
								//スロットル
								mine.key[8] = false;
								mine.key[9] = false;
								//脚
								mine.key[10] = false;
								//ブレーキ
								mine.key[11] = false;
								//精密操作
								mine.key[12] = false;
								mine.key[13] = false;
								mine.key[14] = false;
								mine.key[15] = false;
								mine.key[16] = false;
								mine.key[17] = false;

								auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
								if (&ptr_LEFTHAND != nullptr) {
									if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
										//メイン武器
										mine.key[0] = mine.key[0] || ((ptr_LEFTHAND.on[0] & BUTTON_TRIGGER) != 0);
										//サブ武器
										mine.key[1] = mine.key[1] || ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B)) != 0);
										//ピッチ
										mine.key[2] = mine.key[2] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(28)));
										mine.key[3] = mine.key[3] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-28)));
										//ロール
										mine.key[4] = mine.key[4] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(24)));
										mine.key[5] = mine.key[5] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-24)));
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

										//精密操作
										{
											//ピッチ
											mine.key[12] = mine.key[12] || (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(22)));
											mine.key[13] = mine.key[13] || (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-22)));
											//ロール
											mine.key[14] = mine.key[14] || (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(14)));
											mine.key[15] = mine.key[15] || (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-14)));
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
						}
						//マウスと視点角度をリンク
						if (Drawparts->use_vr) {
							//+視点取得
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
							eye_pos_ads = HMDpos + VGet(0, -0.8f, 0);
							eye_pos_ads = VGet(
								std::clamp(eye_pos_ads.x(), -0.18f, 0.18f),
								std::clamp(eye_pos_ads.y(), 0.f, 0.8f),
								std::clamp(eye_pos_ads.z(), -0.36f, 0.1f)
							);
							eyevec = HMDmat.zvec();
						}
						else {
							int mousex, mousey;
							GetMousePoint(&mousex, &mousey);
							SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);

							float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - Drawparts->disp_x / 2) * 0.1f);
							float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - Drawparts->disp_y / 2) * 0.1f);
							x = std::clamp(x, deg2rad(-45), deg2rad(45));
							eyevec = VGet(cos(x) * std::sin(y), std::sin(x), std::cos(x) * std::cos(y));
						}
					}
					{
						//他のキー入力をここで取得(ホスト)
					}
					//反映
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						//飛行機演算
						{
							float rad_spec = deg2rad(veh.use_veh.body_rad_limit * (veh.use_veh.mid_speed_limit / veh.speed));
							if (veh.speed < veh.use_veh.min_speed_limit) {
								rad_spec = deg2rad(veh.use_veh.body_rad_limit * (std::clamp(veh.speed, 0.f, veh.use_veh.min_speed_limit) / veh.use_veh.min_speed_limit));
							}
							//ピッチ
							easing_set(&veh.xradadd_right, (c.key[2] ? -rad_spec / 3.f : c.key[12] ? -rad_spec / 9.f : 0.f), 0.95f);
							easing_set(&veh.xradadd_left, (c.key[3] ? rad_spec / 3.f : c.key[13] ? rad_spec / 9.f : 0.f), 0.95f);
							//ロール
							easing_set(&veh.zradadd_right, (c.key[4] ? rad_spec : (c.key[14] ? rad_spec / 3.f : 0.f)), 0.95f);
							easing_set(&veh.zradadd_left, (c.key[5] ? -rad_spec : (c.key[15] ? -rad_spec / 3.f : 0.f)), 0.95f);
							//ヨー
							easing_set(&veh.yradadd_left, (c.key[6] ? -rad_spec / 24.f : (c.key[16] ? -rad_spec / 72.f : 0.f)), 0.95f);
							easing_set(&veh.yradadd_right, (c.key[7] ? rad_spec / 24.f : (c.key[17] ? rad_spec / 72.f : 0.f)), 0.95f);
							//スロットル
							easing_set(&veh.speed_add, ((c.key[8] && veh.speed < veh.use_veh.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f);
							easing_set(&veh.speed_sub, c.key[9] ? ((veh.speed > veh.use_veh.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f);
							//スピード
							veh.speed += (veh.speed_add + veh.speed_sub) * 60.f / fps;
							{
								auto tmp = veh.mat.zvec();
								auto tmp2 = std::sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
								veh.speed += (((std::abs(tmp2) > std::sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //落下
							}
							//座標系反映
							{
								auto t_mat = veh.mat;
								veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / fps);
								veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / fps);
								veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / fps);
							}
							//脚
							c.changegear.get_in(c.key[10]);
							easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f);
							MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
							//舵
							for (int i = 0; i < c.p_animes_rudder.size(); i++) {
								easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2] + c.key[i + 12])*0.5f, 0.95f);
								MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
							}
							//
							{
								//
								if (veh.speed >= veh.use_veh.min_speed_limit) {
									easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f);
								}
								else {
									veh.add.yadd(M_GR / powf(fps, 2.f));
								}
								//
								if (c.p_anime_geardown.second >= 0.5f) {
									bool hit_f = false;
									for (auto& t : veh.use_veh.wheelframe) {
										easing_set(&t.gndsmkeffcs.scale, 0.01f, 0.9f);
										auto tmp = veh.obj.frame(int(t.frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
										//地面
										{
											auto hp = mapparts->map_col_line(tmp + (veh.mat.yvec() * (0.5f)), tmp, 0);
											if (hp.HitFlag == TRUE) {
												veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
												{
													auto normal = veh.mat.yvec();
													easing_set(&normal, hp.Normal, 0.95f);
													veh.mat *= MATRIX_ref::RotVec2(veh.mat.yvec(), normal);
												}
												t.gndsmkeffcs.scale = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
												if (veh.speed >= 0.f && (c.key[11])) {
													veh.speed += -0.5f / 3.6f;
												}
												if (veh.speed <= 0.f) {
													easing_set(&veh.speed, 0.f, 0.9f);
												}
												hit_f = true;
											}
										}
									}

									if (hit_f) {
										easing_set(&veh.wheel_Leftadd, -veh.speed / 20.f, 0.95f);
										easing_set(&veh.wheel_Rightadd, -veh.speed / 20.f, 0.95f);
									}
									else {
										easing_set(&veh.wheel_Leftadd, 0.f, 0.9f);
										easing_set(&veh.wheel_Rightadd, 0.f, 0.9f);
									}
									veh.wheel_Left += veh.wheel_Leftadd;  // -veh.yradadd * 5.f;
									veh.wheel_Right += veh.wheel_Rightadd; // +veh.yradadd * 5.f;


									for (auto& f : veh.use_veh.wheelframe_nospring) {
										veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotAxis(MATRIX_ref::Vtrans(VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)), (f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *MATRIX_ref::Mtrans(f.frame.second));
									}
								}
								else {
									for (auto& t : veh.use_veh.wheelframe) {
										easing_set(&t.gndsmkeffcs.scale, 0.01f, 0.9f);
									}
								}
								veh.pos += veh.add + (veh.mat.zvec() * (-veh.speed / fps));
							}
							//壁の当たり判定
							bool hitb = false;
							{
								VECTOR_ref p_0 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
								VECTOR_ref p_1 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
								VECTOR_ref p_2 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
								VECTOR_ref p_3 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
								if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) {
									hitb = true;
								}
								if (!hitb) {
									for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
										if (mapparts->map_col_line(p_0, p_1, i).HitFlag == TRUE) {
											hitb = true;
											break;
										}
										if (mapparts->map_col_line(p_1, p_2, i).HitFlag == TRUE) {
											hitb = true;
											break;
										}
										if (mapparts->map_col_line(p_2, p_3, i).HitFlag == TRUE) {
											hitb = true;
											break;
										}
										if (mapparts->map_col_line(p_3, p_0, i).HitFlag == TRUE) {
											hitb = true;
											break;
										}
									}
								}
								if (veh.HP == 0) {
									hitb = true;
								}
							}
							if (hitb) {
								size_t index = &c - &chara[0];
								veh.spawn(
									VGet(
										float(30 * (index / 3))*sin(deg2rad(-130)),
										10.f,
										float(30 * (index / 3))*cos(deg2rad(-130)) + float(30 * (index % 3))
									),
									MATRIX_ref::RotY(deg2rad(-130)));
								c.p_anime_geardown.second = 1.f;
								c.changegear.first = true;
							}
							//バーナー
							for (auto& be : c.p_burner) {
								veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(veh.speed / veh.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
							}
						}
						//射撃
						for (auto& cg : veh.Gun_) {
							size_t index = &cg - &veh.Gun_[0];
							if (c.key[(index == 0) ? 0 : 1] && cg.loadcnt == 0 && cg.rounds > 0) {
								auto& u = cg.bullet[cg.usebullet];
								++cg.usebullet %= cg.bullet.size();
								//ココだけ変化
								c.se_gun.play(DX_PLAYTYPE_BACK, TRUE);
								u.spec = cg.Spec[0];
								u.spec.speed_a *= float(75 + GetRand(50)) / 100.f;
								u.pos = veh.obj.frame(cg.gun_info.frame2.first);
								u.vec = (veh.obj.frame(cg.gun_info.frame3.first) - veh.obj.frame(cg.gun_info.frame2.first)).Norm();
								//
								cg.loadcnt = cg.gun_info.load_time;
								cg.rounds = std::max<uint16_t>(cg.rounds - 1, 0);
								if (index == 0) {
									cg.fired = 1.f;
								}
								u.hit = false;
								u.flug = true;
								u.count = 0.f;
								u.yadd = 0.f;
								u.repos = u.pos;
								if (u.spec.type_a != 2) {
									c.effcs[ef_fire].set(veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
									if (u.spec.caliber_a >= 0.017f) {
										c.effcs_gun[c.gun_effcnt].first.set(veh.obj.frame(cg.gun_info.frame3.first), u.vec);
										c.effcs_gun[c.gun_effcnt].second = &u;
										c.effcs_gun[c.gun_effcnt].count = 0.f;
										++c.gun_effcnt %= c.effcs_gun.size();
									}
								}
								else {
									c.effcs_missile[c.missile_effcnt].first.set(veh.obj.frame(cg.gun_info.frame3.first), u.vec);
									c.effcs_missile[c.missile_effcnt].second = &u;
									c.effcs_missile[c.missile_effcnt].count = 0.f;
									++c.missile_effcnt %= c.effcs_missile.size();
								}
							}
							cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
							cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
						}
						//弾関連
						{
							//弾判定
							for (auto& cg : veh.Gun_) {
								for (auto& a : cg.bullet) {
									float size = 3.f;
									for (int z = 0; z < int(size); z++) {
										if (a.flug) {
											a.repos = a.pos;
											a.pos += a.vec * (a.spec.speed_a / fps / size);
											//判定
											{
												bool ground_hit = false;
												VECTOR_ref normal;
												//機体以外に当たる
												for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
													auto hps = mapparts->map_col_line(a.repos, a.pos, i);
													if (hps.HitFlag) {
														a.pos = hps.HitPosition;
														normal = hps.Normal;
														ground_hit = true;
													}
												}
												//必要な時に当たり判定をリフレッシュする
												for (auto& t : chara) {
													auto& veh_t = t.vehicle;
													if (&c == &t || veh_t.hit_check) {
														continue;
													}
													if ((Segment_Point_MinLen(a.pos, a.repos, veh_t.pos) > 5.f)) {
														continue;
													}
													veh_t.col.SetMatrix(veh_t.mat * MATRIX_ref::Mtrans(veh_t.pos));
													for (int i = 0; i < veh_t.col.mesh_num(); i++) {
														veh_t.col.RefreshCollInfo(-1, i);
													}
													veh_t.hit_check = true;
												}
												//飛行機にあたる
												auto hitplane = c.get_reco(chara, a);
												//その後処理
												if (!hitplane) {
													if (ground_hit) {
														if (a.spec.caliber_a >= 0.020f) {
															c.effcs[ef_gndhit].set(a.pos + normal * (0.1f), normal);
														}
														else {
															c.effcs[ef_gndhit2].set(a.pos + normal * (0.1f), normal);
														}
														switch (a.spec.type_a) {
														case 0: //AP
															if ((a.vec.Norm().dot(normal)) <= cos(deg2rad(60))) {
																a.flug = false;
															}
															else {
																a.vec += normal * ((a.vec.dot(normal)) * -2.f);
																a.vec = a.vec.Norm();
																a.pos += a.vec * (0.01f);
																a.spec.pene_a /= 2.f;
															}
															break;
														case 1: //HE
															a.flug = false;
															break;
														case 2: //ミサイル
															a.flug = false;
															break;
														default:
															break;
														}
													}
												}
												if (a.flug) {
													switch (a.spec.type_a) {
													case 0: //AP
													{
														a.spec.pene_a -= 1.0f / fps / size;
														a.spec.speed_a -= 5.f / fps / size;
														a.pos += VGet(0.f, a.yadd / size, 0.f);
													}
													break;
													case 1: //HE
													{
														a.spec.speed_a -= 5.f / fps / size;
														a.pos += VGet(0.f, a.yadd / size, 0.f);
														//
														size_t id = chara.size();
														VECTOR_ref pos;
														float dist = (std::numeric_limits<float>::max)();
														for (auto& t : chara) {
															//弾関連
															if (&c == &t) {
																continue;
															}
															auto p = (t.vehicle.pos - a.pos).size();
															if (dist > p) {
																dist = p;
																id = &t - &chara[0];
																pos = t.vehicle.pos;
															}
														}
														if (id != chara.size()) {
															//反映
															auto vec_a = (a.pos - pos).Norm();
															auto vec_z = a.vec;
															if (vec_a.dot(vec_z) < 0) {
																float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
																float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
																float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
																float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
																float view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
																{
																	float limit = deg2rad(2.5f) / fps;
																	float y = atan2f(a.vec.x(), a.vec.z()) + std::clamp(view_yrad, -limit, limit);
																	float x = atan2f(a.vec.y(), std::hypotf(a.vec.x(), a.vec.z())) + std::clamp(view_xrad, -limit, limit);
																	a.vec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
																}
															}
														}
													}
													break;
													case 2: //ミサイル
													{
														size_t id = chara.size();
														VECTOR_ref pos;
														float dist = (std::numeric_limits<float>::max)();
														for (auto& t : chara) {
															//弾関連
															if (&c == &t) {
																continue;
															}
															auto p = (t.vehicle.pos - a.pos).size();
															if (dist > p) {
																dist = p;
																id = &t - &chara[0];
																pos = t.vehicle.pos;
															}
														}
														if (id != chara.size()) {
															//反映
															auto vec_a = (a.pos - pos).Norm();
															auto vec_z = a.vec;
															float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
															float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
															float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
															float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
															float view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
															{
																float limit = deg2rad(25.f) / fps;
																float y = atan2f(a.vec.x(), a.vec.z()) + std::clamp(view_yrad, -limit, limit);
																float x = atan2f(a.vec.y(), std::hypotf(a.vec.x(), a.vec.z())) + std::clamp(view_xrad, -limit, limit);
																a.vec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
															}
														}
													}
													break;
													default:
														break;
													}
												}
											}

											//消す(2秒たった、スピードが100以下、貫通が0以下)
											if (a.count >= 2.5f || a.spec.speed_a < 100.f || a.spec.pene_a <= 0.f) {
												a.flug = false;
											}
											if (!a.flug) {
												for (auto& b : c.effcs_gun) {
													if (b.second == &a) {
														b.first.handle.SetPos(b.second->pos);
														break;
													}
												}
											}
										}
									}
									a.yadd += M_GR / powf(fps, 2.f);
									a.count += 1.f / fps;
								}
							}
							//弾痕
							for (auto& h : veh.hit_obj) {
								if (h.flug) {
									h.pic.SetMatrix(h.mat* veh.mat*MATRIX_ref::Mtrans(veh.pos + MATRIX_ref::Vtrans(h.pos, veh.mat)));
								}
							}
							//ミサイル
							for (auto& a : c.effcs_missile) {
								if (a.second != nullptr) {
									a.n_l = (a.second != nullptr);
									a.flug = a.second->flug;

									if (a.flug) {
										a.first.pos = a.second->pos;
										a.first.handle.SetPos(a.first.pos);
									}
									if (a.count >= 0.f) {
										a.count += 1.f / fps;
										if (a.count >= 3.f) {
											a.first.handle.Stop();
											a.count = -1.f;
										}
									}
								}
							}
							//銃砲
							for (auto& a : c.effcs_gun) {
								if (a.second != nullptr) {
									a.n_l = (a.second != nullptr);
									a.flug = a.second->flug;

									if (a.flug) {
										a.first.pos = a.second->pos;
										a.first.handle.SetPos(a.first.pos);
									}
									if (a.count >= 0.f) {
										a.count += 1.f / fps;
										if (a.count >= 3.f) {
											a.first.handle.Stop();
											a.count = -1.f;
										}
									}
								}
							}
						}
						//effect
						for (auto& t : c.effcs) {
							const size_t index = &t - &c.effcs[0];
							if (index != ef_smoke1 && index != ef_smoke2) {
								t.put(Drawparts->get_effHandle(int(index)));
							}
						}
						for (auto& t : c.effcs_gun) {
							t.first.put(Drawparts->get_effHandle(ef_smoke2));
						}
						for (auto& t : c.effcs_missile) {
							t.first.put(Drawparts->get_effHandle(ef_smoke1));
						}

						for (auto& t : veh.use_veh.wheelframe) {
							t.gndsmkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), t.gndsmkeffcs.scale);
							if (start_c2) {
								t.gndsmkeffcs.set_loop(Drawparts->get_effHandle(ef_gndsmoke));
							}
						}
					}
					start_c2 = false;
					{
						//他の座標をここで出力(ホスト)
					}
					{
						//ホストからの座標をここで入力
					}
					//モデルに反映
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
					}
					for (auto& c : chara) {
						c.se_cockpit.SetPosition(c.vehicle.pos);
						c.se_hit.SetPosition(c.vehicle.pos);
						c.se_gun.SetPosition(c.vehicle.pos);
					}
					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos,
						[&] {
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							veh.obj.DrawModel();
						}
						mapparts->map_drawtree();
					}
					, VGet(200.f, 200.f, 200.f), VGet(2000.f, 2000.f, 2000.f));
					//VR更新
					Drawparts->Move_Player();
					//描画
					{
						//自機描画
						{
							auto& veh = mine.vehicle;
							//cam_s.cam
							{
								//campos,camvec,camup取得
								if (cam_s.Rot == ADS) {
									cam_easy.camvec -= cam_easy.campos;
									cam_easy.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
									cam_easy.campos.y(std::max(cam_easy.campos.y(), 5.f));
									if (Drawparts->use_vr) {
										cam_easy.camvec = cam_easy.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
										cam_easy.camup = MATRIX_ref::Vtrans(HMDmat.yvec(), veh.mat);//veh.mat.yvec();
									}
									else {
										if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
											easing_set(&cam_easy.camvec, MATRIX_ref::Vtrans(eyevec, veh.mat)*-1.f, 0.75f);
											cam_easy.camvec += cam_easy.campos;

											cam_easy.camup = veh.mat.yvec();
										}
										else {
											eyevec = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
											cam_easy.camvec = cam_easy.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
											cam_easy.camup = veh.mat.yvec();
										}
									}
								}
								else {
									cam_easy.campos -= cam_easy.camvec;

									cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
									cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

									if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {

										easing_set(&cam_easy.campos, eyevec * range, 0.9f);
										cam_easy.campos += cam_easy.camvec;
										cam_easy.campos.y(std::max(cam_easy.campos.y(), 0.f));
										mapparts->map_col_line_nearest(cam_easy.camvec, &cam_easy.campos);

										easing_set(&cam_easy.camup, VGet(0.f, 1.f, 0.f), 0.9f);
									}
									else {
										cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
										cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

										eyevec = (cam_easy.camvec - (mine.vehicle.pos + mine.vehicle.mat.zvec() * (-1000.f))).Norm();
										cam_easy.campos = cam_easy.camvec + eyevec * range;

										cam_easy.camup = veh.mat.yvec();
									}


								}
								//near取得
								cam_easy.near_ = (cam_s.Rot == ADS) ? (5.f + 25.f * (cam_easy.far_ - 300.f) / (3000.f - 300.f)) : (range_p - 5.f);
								//far取得
								cam_easy.far_ = 6000.f;
								//fov
								cam_easy.fov = deg2rad(Drawparts->use_vr ? 90 : 45);

								cam_s.cam = cam_easy;
							}
							//照準座標取得
							{
								Hostpassparts->MAIN_Screen.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, 0.01f, 5000.0f);
								{
									VECTOR_ref aimpos = mine.vehicle.pos + mine.vehicle.mat.zvec() * (-1000.f);
									//地形
									mapparts->map_col_line_nearest(mine.vehicle.pos, &aimpos);
									aimposout = ConvWorldPosToScreenPos(aimpos.get());
									for (auto& c : chara) {
										c.winpos = ConvWorldPosToScreenPos(c.vehicle.pos.get());
									}
								}
							}
							//
							Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
							//UI
							Hostpassparts->UI_Screen.SetDraw_Screen();
							{
								UIparts->draw(chara, aimposout, *Drawparts->get_device_hand1(), mine);
							}
							//sky
							Hostpassparts->SkyScreen.SetDraw_Screen(cam_s.cam.campos - cam_s.cam.camvec, VGet(0, 0, 0), cam_s.cam.camup, cam_s.cam.fov, 1.0f, 50.0f);
							{
								mapparts->sky_draw();
							}
							//被写体深度描画
							Hostpassparts->dof(ram_draw, cam_s.cam);
							//最終描画
							Hostpassparts->MAIN_Screen.SetDraw_Screen();
							{
								Hostpassparts->bloom(255);//ブルーム
							}
							//コックピット演算
							if (cam_s.Rot == ADS) {
								cocks.ready_(mine);
							}
							//VRに移す
							Drawparts->draw_VR(
								[&] {
								SetCameraNearFar(0.01f, 2.f);
								SetUseZBuffer3D(FALSE);												//zbufuse
								SetWriteZBuffer3D(FALSE);											//zbufwrite
								DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, Drawparts->use_vr ? 1.8f : 1.475f, 0.f, Hostpassparts->MAIN_Screen.get(), TRUE);
								SetUseZBuffer3D(TRUE);												//zbufuse
								SetWriteZBuffer3D(TRUE);											//zbufwrite
								//コックピット
								if (cam_s.Rot == ADS) {
									cocks.cockpit.DrawModel();
								}
								//UI
								SetUseZBuffer3D(FALSE);												//zbufuse
								SetWriteZBuffer3D(FALSE);											//zbufwrite
								DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, Drawparts->use_vr ? 1.8f : 1.475f, 0.f, Hostpassparts->UI_Screen.get(), TRUE);
								SetUseZBuffer3D(TRUE);												//zbufuse
								SetWriteZBuffer3D(TRUE);											//zbufwrite
								//Hostpassparts->UI_Screen.DrawGraph(0, 0, true);
							}, cam_s.cam);
						}
						//自機リプレイ
						{
							rep_cam.push_back(cam_s);
						}
						//2P描画
						if (Drawparts->use_vr) {
							auto& ct = chara[1];
							auto& veh = ct.vehicle;
							//cam_s.cam
							{
								//campos,camvec,camup取得
								{
									int mousex, mousey;
									GetMousePoint(&mousex, &mousey);
									SetMousePoint(Drawparts->out_disp_x / 2, Drawparts->out_disp_y / 2);

									float y = atan2f(eyevec2.x(), eyevec2.z()) + deg2rad(float(mousex - Drawparts->out_disp_x / 2) * 0.1f);
									float x = atan2f(eyevec2.y(), std::hypotf(eyevec2.x(), eyevec2.z())) + deg2rad(float(mousey - Drawparts->out_disp_y / 2) * 0.1f);
									x = std::clamp(x, deg2rad(-45), deg2rad(45));
									eyevec2 = VGet(cos(x) * std::sin(y), std::sin(x), std::cos(x) * std::cos(y));
								}
								{
									cam_s.cam.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(0, 0.58f, 0), veh.mat);
									cam_s.cam.campos.y(std::max(cam_s.cam.campos.y(), 5.f));
									if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
										eyevec2 = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
									}
									cam_s.cam.camvec = cam_s.cam.campos - MATRIX_ref::Vtrans(eyevec2, veh.mat);
									cam_s.cam.camup = veh.mat.yvec();
								}
								//near取得
								cam_s.cam.near_ = 5.f + 25.f * (cam_s.cam.far_ - 300.f) / (3000.f - 300.f);
								//far取得
								cam_s.cam.far_ = 4000.f;
								//fov
								cam_s.cam.fov = deg2rad(45);
							}
							//照準座標取得
							{
								Hostpass2parts->MAIN_Screen.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, 0.01f, 5000.0f);
								{
									VECTOR_ref aimpos2 = ct.vehicle.pos + ct.vehicle.mat.zvec() * (-1000.f);
									//地形
									mapparts->map_col_line_nearest(ct.vehicle.pos, &aimpos2);
									aimposout = ConvWorldPosToScreenPos(aimpos2.get());
									for (auto& c : chara) {
										auto& veht = c.vehicle;
										c.winpos = ConvWorldPosToScreenPos(veht.pos.get());
									}
								}
							}
							//UI
							Hostpass2parts->UI_Screen.SetDraw_Screen();
							{
								UIparts->draw(chara, aimposout, *Drawparts->get_device_hand1(), ct, 0);
							}
							//sky
							Hostpass2parts->SkyScreen.SetDraw_Screen(cam_s.cam.campos - cam_s.cam.camvec, VGet(0, 0, 0), cam_s.cam.camup, cam_s.cam.fov, 1.0f, 50.0f);
							{
								mapparts->sky_draw();
							}
							//被写体深度描画
							Hostpass2parts->dof(ram_draw, cam_s.cam);
							//最終描画
							Hostpass2parts->MAIN_Screen.SetDraw_Screen();
							{
								Hostpass2parts->bloom(255);//ブルーム
							}
							//コックピット演算
							{
								cocks.ready_(ct);
							}
							//Screen2に移す
							outScreen2.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, cam_s.cam.near_, cam_s.cam.far_);
							{
								Hostpass2parts->MAIN_Screen.DrawGraph(0, 0, true);
								SetCameraNearFar(0.01f, 2.f);
								//コックピット
								cocks.cockpit.DrawModel();
								//UI
								Hostpass2parts->UI_Screen.DrawGraph(0, 0, true);
							}
						}
					}
					//draw
					GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
					{
						if (Drawparts->use_vr) {
							outScreen2.DrawGraph(0, 0, false);
							//Drawparts->outScreen[0].DrawExtendGraph(0, 0, Drawparts->out_disp_x, Drawparts->out_disp_y, false);
						}
						else {
							Drawparts->outScreen[0].DrawGraph(0, 0, false);
						}
						Debugparts->end_way();
						Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
					}

					Drawparts->Screen_Flip(waits);
					if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
						ending = false;
						break;
					}
					if (CheckHitKey(KEY_INPUT_O) != 0) {
						break;
					}
					//リプレイ
					{
						Mainclass::Chara::sendstat tmp_rep;
						for (auto& c : chara) {
							c.rep.push_back(tmp_rep);
							c.rep.back().get_data(c);
							for (auto& t : c.effcs) {
								t.put_end();
							}
							for (auto& t : c.effcs_gun) {
								t.first.put_end();
							}
							for (auto& t : c.effcs_missile) {
								t.first.put_end();
							}
						}
					}
					//
				}
				SetMouseDispFlag(TRUE);
				SetMousePoint(deskx / 2, desky / 2);
				for (auto& c : chara) {
					c.se_cockpit.stop(); //gun
					c.se_gun.stop(); //gun
					c.se_hit.stop(); //gun
				}
			}
			//保存
			{
				std::ofstream fout;
				fout.open("data/file.txt", std::ios::binary);

				auto tt = chara[0].rep.begin();
				auto tt2 = chara[1].rep.begin();
				auto tcam = rep_cam.begin();
				while (true) {
					{
						tt->write(fout);
						tt2->write(fout);
						fout.write((char *)&tcam->cam, sizeof(tcam->cam));
						fout.write((char *)&tcam->Rot, sizeof(tcam->Rot));
					}
					tt++;
					if (tt == chara[0].rep.end()) {
						break;
					}
					tt2++;
					if (tt2 == chara[1].rep.end()) {
						break;
					}
					tcam++;
					if (tcam == rep_cam.end()) {
						break;
					}
				}
				fout.close();  //ファイルを閉じる
			}
			//読み出し
			//*
			{
				Mainclass::Chara::sendstat tmp_rep;
				Mainclass::CAMS tmp_cam_rep;

				std::ifstream fout;
				fout.open("data/file.txt", std::ios::binary);
				{
					chara[0].rep.clear();
					chara[1].rep.clear();
					rep_cam.clear();
					chara[0].rep.push_back(tmp_rep);
					chara[1].rep.push_back(tmp_rep);
					rep_cam.push_back(tmp_cam_rep);
				}
				auto tt = chara[0].rep.begin();
				auto tt2 = chara[1].rep.begin();
				auto tcam = rep_cam.begin();
				while (true) {
					{
						tt->read(fout);
						tt2->read(fout);
						fout.read((char *)&tcam->cam, sizeof(tcam->cam));
						fout.read((char *)&tcam->Rot, sizeof(tcam->Rot));
					}
					if (fout.eof()) {
						break;
					}
					{
						chara[0].rep.push_back(tmp_rep);
						chara[1].rep.push_back(tmp_rep);
						rep_cam.push_back(tmp_cam_rep);
					}
					tt++;
					tt2++;
					tcam++;
				}
				fout.close();  //ファイルを閉じる
			}
			//*/
			//リプレイ
			{
				//開始
				for (auto& c : chara) {
					//ミサイル
					for (auto& a : c.effcs_missile) {
						a.first.handle.Dispose();
					}
					//銃砲
					for (auto& a : c.effcs_gun) {
						a.first.handle.Dispose();
					}
				}
				size_t sel_l = 0;
				switchs start_stop;
				eyevec2 = chara[sel_l].vehicle.mat.zvec() * -1.f;
				eye_pos_ads = VGet(0, 0.58f, 0);
				for (auto& c : chara) {
					Set3DRadiusSoundMem(600.0f, c.se_cockpit.get()); //gun
					Set3DRadiusSoundMem(300.0f, c.se_gun.get()); //gun
					Set3DRadiusSoundMem(900.0f, c.se_hit.get()); //gun
					c.se_cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
					c.se_cockpit.vol(64);
				}
				SetMouseDispFlag(FALSE);
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
				{
					auto tcam = rep_cam.begin();
					auto tt = chara[0].rep.begin();
					auto tt2 = chara[1].rep.begin();
					while (ProcessMessage() == 0) {
						const auto waits = GetNowHiPerformanceCount();
						Debugparts->put_way();
						//プレイヤー操作
						{
							//スコープ
							{
								cam_s.Rot = std::clamp(cam_s.Rot + GetMouseWheelRotVol(), 0, ADS);
								switch (cam_s.Rot) {
								case 1:
									range_p = 15.f;
									break;
								case 0:
									range_p = 30.f;
									break;
								default:
									cam_s.Rot = ADS;
									break;
								}
								easing_set(&range, range_p, 0.9f);
							}
							//選択
							{
								if (CheckHitKey(KEY_INPUT_0) != 0) {
									sel_l = 0;
								}
								if (CheckHitKey(KEY_INPUT_1) != 0) {
									sel_l = 1;
								}
							}
							//見回し
							if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
								chara[sel_l].view_yrad = 0.f;
								chara[sel_l].view_xrad = 0.f;
							}
							//マウスと視点角度をリンク
							{
								int mousex, mousey;
								GetMousePoint(&mousex, &mousey);
								SetMousePoint(Drawparts->out_disp_x / 2, Drawparts->out_disp_y / 2);

								float y = atan2f(eyevec2.x(), eyevec2.z()) + deg2rad(float(mousex - Drawparts->out_disp_x / 2) * 0.1f);
								float x = atan2f(eyevec2.y(), std::hypotf(eyevec2.x(), eyevec2.z())) + deg2rad(float(mousey - Drawparts->out_disp_y / 2) * 0.1f);
								x = std::clamp(x, deg2rad(-45), deg2rad(45));
								eyevec2 = VGet(cos(x) * std::sin(y), std::sin(x), std::cos(x) * std::cos(y));
							}
						}
						//
						{
							tt->put_data(chara[0]);
							tt2->put_data(chara[1]);
						}
						//cam_s.cam
						{
							if (sel_l == 0 && ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0)) {
								cam_s = *tcam;
								cam_s.cam.fov = deg2rad(45);
							}
							else {
								auto& veh = chara[sel_l].vehicle;
								//cam_s.cam
								{
									//campos,camvec,camup取得
									if (cam_s.Rot == ADS) {
										cam_easy.camvec -= cam_easy.campos;
										cam_easy.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
										cam_easy.campos.y(std::max(cam_easy.campos.y(), 5.f));

										if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
											easing_set(&cam_easy.camvec, MATRIX_ref::Vtrans(eyevec2, veh.mat)*-1.f, 0.75f);
											cam_easy.camvec += cam_easy.campos;

											cam_easy.camup = veh.mat.yvec();
										}
										else {
											eyevec2 = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
											cam_easy.camvec = cam_easy.campos - MATRIX_ref::Vtrans(eyevec2, veh.mat);
											cam_easy.camup = veh.mat.yvec();
										}


									}
									else {
										cam_easy.campos -= cam_easy.camvec;

										cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
										cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

										if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {

											easing_set(&cam_easy.campos, eyevec2 * range, 0.9f);
											cam_easy.campos += cam_easy.camvec;
											cam_easy.campos.y(std::max(cam_easy.campos.y(), 0.f));
											mapparts->map_col_line_nearest(cam_easy.camvec, &cam_easy.campos);

											easing_set(&cam_easy.camup, VGet(0.f, 1.f, 0.f), 0.9f);
										}
										else {
											cam_easy.camvec = veh.pos + veh.mat.yvec() * (6.f);
											cam_easy.camvec.y(std::max(cam_easy.camvec.y(), 5.f));

											eyevec2 = (cam_easy.camvec - (chara[sel_l].vehicle.pos + chara[sel_l].vehicle.mat.zvec() * (-1000.f))).Norm();
											cam_easy.campos = cam_easy.camvec + eyevec2 * range;

											cam_easy.camup = veh.mat.yvec();
										}
									}
									//near取得
									cam_easy.near_ = (cam_s.Rot == ADS) ? (5.f + 25.f * (cam_easy.far_ - 300.f) / (3000.f - 300.f)) : (range_p - 5.f);
									//far取得
									cam_easy.far_ = 6000.f;
									//fov
									cam_easy.fov = deg2rad(45);

									cam_s.cam = cam_easy;
								}
							}
						}
						//
						{
							start_stop.get_in(CheckHitKey(KEY_INPUT_SPACE) != 0);
							if (!start_stop.first) {
								tt++;
								if (tt == chara[0].rep.end()) {
									break;
								}
								tt2++;
								if (tt2 == chara[1].rep.end()) {
									break;
								}
								tcam++;
								if (tcam == rep_cam.end()) {
									break;
								}
							}
						}
						//
						{
							for (auto& c : chara) {
								auto& veh = c.vehicle;

								c.se_cockpit.SetPosition(c.vehicle.pos);
								c.se_hit.SetPosition(c.vehicle.pos);
								c.se_gun.SetPosition(c.vehicle.pos);
								/*effect*/
								{
									int i = 0;
									for (auto& t : c.effcs) {
										t.put(Drawparts->get_effHandle(i));
										i++;
									}
								}
								for (auto& t : veh.use_veh.wheelframe) {
									t.gndsmkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), t.gndsmkeffcs.scale);
								}
								//銃砲
								//*
								for (auto& a : c.effcs_gun) {
									a.first.put(Drawparts->get_effHandle(ef_smoke2));
									if (a.n_l) {
										if (a.flug) {
											a.first.handle.SetPos(a.first.pos);
										}
										if (a.count >= 0.f) {
											if (a.count >= 3.f) {
												a.first.handle.Dispose();
											}
										}
									}
								}
								//ミサイル
								for (auto& a : c.effcs_missile) {
									a.first.put(Drawparts->get_effHandle(ef_smoke1));
									if (a.n_l) {
										if (a.flug) {
											a.first.handle.SetPos(a.first.pos);
										}
										if (a.count >= 0.f) {
											if (a.count >= 3.f) {
												a.first.handle.Dispose();
											}
										}
									}
								}
								//*/
							}
						}
						//描画
						{

							Drawparts->Ready_Shadow(cam_s.cam.campos,
								[&] {
								for (auto& c : chara) {
									auto& veh = c.vehicle;
									veh.obj.DrawModel();
								}
								mapparts->map_drawtree();
							}
							, VGet(200.f, 200.f, 200.f), VGet(2000.f, 2000.f, 2000.f));

							//照準座標取得
							{
								auto& veh = chara[sel_l].vehicle;
								Hostpass2parts->MAIN_Screen.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, 0.01f, 5000.0f);
								{
									VECTOR_ref aimpos = veh.pos + veh.mat.zvec() * (-1000.f);
									//地形
									mapparts->map_col_line_nearest(veh.pos, &aimpos);
									aimposout = ConvWorldPosToScreenPos(aimpos.get());
									for (auto& c : chara) {
										auto& veht = c.vehicle;
										c.winpos = ConvWorldPosToScreenPos(veht.pos.get());
									}
								}
							}
							Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
							//UI
							Hostpass2parts->UI_Screen.SetDraw_Screen();
							{
								UIparts->draw(chara, aimposout, *Drawparts->get_device_hand1(), chara[sel_l], 0);
							}
							//sky
							Hostpass2parts->SkyScreen.SetDraw_Screen(cam_s.cam.campos - cam_s.cam.camvec, VGet(0, 0, 0), cam_s.cam.camup, cam_s.cam.fov, 1.0f, 50.0f);
							{
								mapparts->sky_draw();
							}
							//被写体深度描画
							Hostpass2parts->dof(ram_draw, cam_s.cam, !start_stop.first);
							//最終描画
							Hostpass2parts->MAIN_Screen.SetDraw_Screen();
							{
								Hostpass2parts->bloom(255);//ブルーム
							}
							//コックピット演算
							if (cam_s.Rot == ADS) {
								cocks.ready_(chara[sel_l]);
							}
							//VRに移す
							outScreen2.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, cam_s.cam.near_, cam_s.cam.far_);
							{
								Hostpass2parts->MAIN_Screen.DrawGraph(0, 0, true);
								//コックピット
								if (cam_s.Rot == ADS) {
									SetCameraNearFar(0.01f, 2.f);
									cocks.cockpit.DrawModel();
								}
								//UI
								Hostpass2parts->UI_Screen.DrawGraph(0, 0, true);
							}
							//draw
							GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
							{
								outScreen2.DrawGraph(0, 0, false);
								font12.DrawString(10, 10, "REPLAY", GetColor(255, 0, 0));
								if (start_stop.first) {
									font12.DrawString(10, 10 + 20, "|>", GetColor(255, 0, 0));
								}
								else {
									font12.DrawString(10, 10 + 20, "||", GetColor(255, 0, 0));
								}
								Debugparts->end_way();
								Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
							}
						}

						Drawparts->Screen_Flip(waits);
						if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) {
							ending = false;
							break;
						}
						if (CheckHitKey(KEY_INPUT_O) != 0) {
							break;
						}
					}
				}
			}
			//解放
			{
				for (auto& c : chara) {
					c.rep.clear();
					auto& veh = c.vehicle;
					/*エフェクト*/
					for (auto& t : c.effcs_gun) {
						t.first.handle.Dispose();
					}
					for (auto& t : c.effcs_missile) {
						t.first.handle.Dispose();
					}
					for (auto& t : c.effcs) {
						t.handle.Dispose();
					}
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.handle.Dispose();
					}
					veh.init();
				}
				chara.clear();
				mapparts->delete_map();
				Drawparts->Delete_Shadow();
			}
			//
		} while (ProcessMessage() == 0 && ending);
	}
};