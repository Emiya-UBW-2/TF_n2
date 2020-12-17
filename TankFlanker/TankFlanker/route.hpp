#pragma once
class main_c : Mainclass {
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;

	DXDraw::cam_info cam_easy;
	std::list<Mainclass::CAMS> rep_cam;
	VECTOR_ref eyevec, eyevec2;																	//視点
	FontHandle font18;
	GraphHandle outScreen2;
	MV1 hit_pic;	//弾痕
	MV1 cockpit;	//コックピット
	//操作
	Mainclass::CAMS cam_s;
	float range = 0.f, range_p = 30.f;
	float fovs = 1.f, fovs_p = 1.f;
	VECTOR_ref eye_pos_ads = VGet(0, 0.58f, 0);
	VECTOR_ref HMDpos;
	MATRIX_ref HMDmat;
	VECTOR_ref rec_HMD;
	//データ
	std::vector<Mainclass::Chara> chara;	//キャラ
	std::vector<Mainclass::Ammos> Ammo;		//弾薬
	std::vector<Mainclass::Vehcs> Vehicles;	//車輛データ
	SoundHandle se_cockpit;
	SoundHandle se_gun;
	SoundHandle se_missile;
	SoundHandle se_hit;
	SoundHandle se_alert;
	//設定
	bool oldv = false;
	bool start_c = true;
	bool start_c2 = true;
	bool ending = true;
	float fov_pc = 45.f;

	const bool replay_on = false;
public:
	main_c() {
		//設定読み込み
		{
			SetOutApplicationLogValidFlag(FALSE);	//log
			int mdata = FileRead_open("data/setting.txt", FALSE);
			dof_e = getparams::_bool(mdata);
			bloom_e = getparams::_bool(mdata);
			shadow_e = getparams::_bool(mdata);
			useVR_e = getparams::_bool(mdata);
			if (replay_on) {
				useVR_e = false;
			}
			FileRead_close(mdata);
			SetOutApplicationLogValidFlag(TRUE);	//log
		}
		//
		auto Drawparts = std::make_unique<DXDraw>("TankFlanker", FRAME_RATE, useVR_e, shadow_e);								//汎用クラス
		auto UIparts = std::make_unique<UI>(Drawparts->use_vr);																	//UI
		auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);																	//デバッグ
		auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);			//ホストパスエフェクト
		auto Hostpass2parts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->out_disp_x, Drawparts->out_disp_y);	//ホストパスエフェクト
		auto mapparts = std::make_unique<Mapclass>();																			//map
		//
		font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
		outScreen2 = GraphHandle::Make(Drawparts->out_disp_x, Drawparts->out_disp_y, true);	//描画スクリーン
	//その他
		SetCreate3DSoundFlag(TRUE);
		se_cockpit = SoundHandle::Load("data/audio/fighter-cockpit1.wav");
		se_gun = SoundHandle::Load("data/audio/hit.wav");
		se_missile = SoundHandle::Load("data/audio/rolling_rocket.wav");
		se_hit = SoundHandle::Load("data/audio/destruction.wav");
		SetCreate3DSoundFlag(FALSE);
		se_alert = SoundHandle::Load("data/audio/alert.wav");
		//
		MV1::Load("data/model/hit/model.mv1", &hit_pic, true);			//弾痕
		Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles, true);
		MV1::Load("data/model/cockpit/model.mv1", &cockpit, true);
		//
		UIparts->load_window("車両モデル");					//ロード画面
		//
		Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
		Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
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
				//雲
				mapparts->cloud_draw();
				//機体
				SetFogStartEnd(0.0f, 3000.f);
				SetFogColor(128, 128, 128);
				{
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						//戦闘機
						for (auto& h : veh.HP_m) {
							size_t i = &h - &veh.HP_m[0];
							if (h > 0) {
								veh.obj.DrawMesh(i);
							}
							else {
								if (veh.info_break[i].per > 0.1f) {
									veh.obj_break.SetMatrix(veh.info_break[i].mat * MATRIX_ref::Mtrans(veh.info_break[i].pos));
									veh.obj_break.DrawMesh(i);
								}
							}
						}
						//弾痕
						for (auto& h : veh.hit_obj) {
							h.draw();
						}
					}
					mapparts->map_drawtree();
				}
				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				for (auto& c : chara) {
					auto& veh = c.vehicle;
					for (auto& cg : veh.Gun_) {
						cg.draw(cam_s.cam.campos);
					}
				}
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);

			}
			);
		};
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
			if (!replay_on) {
				chara.resize(20);
			}
			else{
				Mainclass::Chara::sendstat tmp_rep;
				Mainclass::CAMS tmp_cam_rep;
				size_t size_c = 1;
				std::vector< std::list<Mainclass::Chara::sendstat>::iterator> chara_rep;

				std::ifstream fout;
				fout.open("data/file.txt", std::ios::binary);

				fout.read((char *)&size_c, sizeof(size_c));

				chara.resize(size_c);
				{
					for (auto& c : chara) {
						c.rep.clear();
					}
					rep_cam.clear();
					for (auto& c : chara) {
						c.rep.push_back(tmp_rep);
					}
					rep_cam.push_back(tmp_cam_rep);
				}

				for (auto& c : chara) {
					chara_rep.emplace_back(c.rep.begin());
				}

				auto tcam = rep_cam.begin();
				while (true) {
					{
						for (auto& cr : chara_rep) {
							cr->read(fout);
						}
						fout.read((char *)&tcam->cam, sizeof(tcam->cam));
						fout.read((char *)&tcam->Rot, sizeof(tcam->Rot));
					}
					if (fout.eof()) {
						break;
					}
					{
						for (auto& c : chara) {
							c.rep.push_back(tmp_rep);
						}
						rep_cam.push_back(tmp_cam_rep);
					}
					for (auto& cr : chara_rep) {
						cr++;
					}
					tcam++;
				}
				fout.close();  //ファイルを閉じる
			}
			{
				oldv = false;
				start_c = true;
				start_c2 = true;
				for (auto& c : chara) {
				}
				//キャラ選択
				if (!UIparts->select_window(&chara[0], &Vehicles)) {
					break;
				}
				//マップ読み込み
				mapparts->set_map_pre();
				UIparts->load_window("マップモデル");			   //ロード画面
				mapparts->set_map("data/grassput.bmp", 35000.f, 35000.f, -35000.f, -35000.f);
				//木セット
				mapparts->map_settree();
				//光、影
				Drawparts->Set_Light_Shadow(mapparts->map_get().mesh_maxpos(0), mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f),
					[&] {
					mapparts->map_get().DrawModel();
					mapparts->cloud_draw();
				});
				//共通
				for (auto& c : chara) {
					//キャラ選択
					size_t i = &c - &chara[0];
					c.vehicle.spawn(VGet(float(100 * (i / 3))*sin(deg2rad(-130)), 1000.f, float(100 * (i / 3))*cos(deg2rad(-130)) + float(100 * (i % 3))), MATRIX_ref::RotY(deg2rad(-130)));
					//キャラ設定
					c.set_human(Vehicles, Ammo, hit_pic);	//
					c.cocks.set_(cockpit);					//コックピット
					//
					c.se_cockpit = se_cockpit.Duplicate();
					c.se_gun = se_gun.Duplicate();
					c.se_missile = se_missile.Duplicate();
					c.se_hit = se_hit.Duplicate();
					c.se_cockpit.Radius(600.f);
					c.se_gun.Radius(300.f);
					c.se_missile.Radius(300.f);
					c.se_hit.Radius(900.f);
				}
				se_alert.vol(64);
				//開始共通
				eye_pos_ads = VGet(0, 0.58f, 0);
				auto& mine = chara[0];
				size_t sel_l = 0;
				switchs start_stop;
				std::vector< std::list<Mainclass::Chara::sendstat>::iterator> chara_rep;
				//開始
				if (!replay_on) {
					eyevec = mine.vehicle.mat.zvec() * -1.f;
					cam_s.cam.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
					cam_s.Rot = ADS;
					eyevec2 = chara[1].vehicle.mat.zvec() * -1.f;
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						for (auto& t : veh.use_veh.wheelframe) {
							t.init();
						}
					}
				}
				else {
					eyevec2 = chara[sel_l].vehicle.mat.zvec() * -1.f;
				}

				for (auto& c : chara) {
					c.se_cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
					c.se_cockpit.vol(64);
					chara_rep.emplace_back(c.rep.begin());
				}
				auto tcam = rep_cam.begin();

				SetMouseDispFlag(FALSE);
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);

				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					Debugparts->put_way();
					if (!replay_on) {
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							//当たり判定クリア
							if (veh.hit_check) {
								veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
								for (int i = 0; i < veh.col.mesh_num(); i++) {
									veh.col.RefreshCollInfo(-1, i);
								}
								veh.hit_check = false;
							}
						}
					}

					//プレイヤー操作
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
						//選択
						if (replay_on) {
							if (CheckHitKey(KEY_INPUT_0) != 0) {
								sel_l = 0;
							}
							if (CheckHitKey(KEY_INPUT_1) != 0) {
								sel_l = 1;
							}
						}
						//見回し
						if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
							if (!replay_on) {
								if (Drawparts->use_vr) {
									chara[1].view_yrad = 0.f;
									chara[1].view_xrad = 0.f;
								}
								else {
									mine.view_yrad = 0.f;
									mine.view_xrad = 0.f;
								}
							}
							else {
								chara[sel_l].view_yrad = 0.f;
								chara[sel_l].view_xrad = 0.f;
							}
						}
						//キー
						if (!replay_on) {
							//
							for (auto& c : chara) {
								auto& veh = c.vehicle;
								if (&c - &chara[0] >= (Drawparts->use_vr ? 2 : 1)) {

									c.key[0] = false;//GetRand(100) <= 10;   //射撃
									c.key[1] = false;//GetRand(100) <= 1; //マシンガン


									size_t id = chara.size();
									VECTOR_ref tgt_pos;

									float dist = (std::numeric_limits<float>::max)();
									for (auto& t : chara) {
										//弾関連
										if (&c == &t) {
											continue;
										}
										auto p = (t.vehicle.pos - veh.pos).size();
										if (dist > p) {
											dist = p;
											id = &t - &chara[0];
											tgt_pos = t.vehicle.pos;
										}
									}

									VECTOR_ref tgt_vec = (tgt_pos - veh.pos).Norm();
									VECTOR_ref my_xvec = veh.mat.xvec();
									VECTOR_ref my_yvec = veh.mat.yvec();
									VECTOR_ref my_zvec = veh.mat.zvec();
									{
										//ピッチ
										VECTOR_ref tgt_yvec = tgt_vec.cross(my_xvec);
										VECTOR_ref cross_yvec = tgt_vec.cross(tgt_yvec);
										VECTOR_ref cross_vec = tgt_vec.cross(my_zvec);

										if (cross_vec.dot(cross_yvec) >= 0.1f) {
											c.key[2] = GetRand(10) <= 5;
											c.key[3] = false;
											c.key[12] = !c.key[2];
											c.key[13] = false;
										}
										else if (cross_vec.dot(cross_yvec) <= -0.1f) {
											c.key[2] = false;
											c.key[3] = GetRand(10) <= 5;
											c.key[12] = false;
											c.key[13] = !c.key[3];
										}
										else {
											c.key[2] = false;
											c.key[3] = false;
											if (cross_vec.dot(cross_yvec) >= 0.f) {
												c.key[12] = true;
												c.key[13] = false;
											}
											else {
												c.key[12] = false;
												c.key[13] = true;
											}

											c.key[0] = GetRand(100) <= 10;   //射撃
											c.key[1] = GetRand(200) <= 1; //マシンガン
										}
									}
									//ロール
									{
										auto tgt_xvec = tgt_vec.cross(my_zvec).Norm();
										VECTOR_ref tgt_yvec = tgt_xvec.cross(my_zvec);
										VECTOR_ref cross_yvec = tgt_xvec.cross(tgt_yvec);
										VECTOR_ref cross_vec = tgt_xvec.cross(my_xvec);

										if (cross_vec.dot(cross_yvec) >= 0.1f) {
											c.key[4] = GetRand(10) <= 5;
											c.key[5] = false;
											c.key[14] = !c.key[4];
											c.key[15] = false;
										}
										else if (cross_vec.dot(cross_yvec) <= -0.1f) {
											c.key[4] = false;
											c.key[5] = GetRand(10) <= 5;
											c.key[14] = false;
											c.key[15] = !c.key[5];
										}
										else {
											c.key[4] = false;
											c.key[5] = false;
											if (cross_vec.dot(cross_yvec) >= 0.f) {
												c.key[14] = true;
												c.key[15] = false;
											}
											else {
												c.key[14] = false;
												c.key[15] = true;
											}
										}
									}

									//ヨー
									c.key[6] = false;
									c.key[7] = false;
									//スロットル
									if (veh.speed <= veh.use_veh.min_speed_limit) {
										c.key[8] = true;
										c.key[9] = false;
									}
									else if (veh.speed >= veh.use_veh.max_speed_limit*0.8f) {
										c.key[8] = false;
										c.key[9] = true;
									}
									else {
										c.key[8] = false;
										c.key[9] = false;
									}

									c.key[10] = false;
									c.key[11] = false; 

									c.key[16] = false;
									c.key[17] = false; 
								}
							}
							if (Drawparts->use_vr) {
								chara[1].key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
								chara[1].key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
								chara[1].key[2] = (CheckHitKey(KEY_INPUT_W) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								chara[1].key[3] = (CheckHitKey(KEY_INPUT_S) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								chara[1].key[4] = (CheckHitKey(KEY_INPUT_D) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								chara[1].key[5] = (CheckHitKey(KEY_INPUT_A) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								//ヨー
								chara[1].key[6] = (CheckHitKey(KEY_INPUT_Q) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								chara[1].key[7] = (CheckHitKey(KEY_INPUT_E) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								//スロットル
								chara[1].key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
								chara[1].key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
								//脚
								chara[1].key[10] = false;
								//ブレーキ
								chara[1].key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
								//精密操作
								chara[1].key[12] = (CheckHitKey(KEY_INPUT_W) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[13] = (CheckHitKey(KEY_INPUT_S) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[14] = (CheckHitKey(KEY_INPUT_D) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[15] = (CheckHitKey(KEY_INPUT_A) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[16] = (CheckHitKey(KEY_INPUT_Q) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								chara[1].key[17] = (CheckHitKey(KEY_INPUT_E) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
							}
							//通常、VR共通
							if (!Drawparts->use_vr) {
								mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
								mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
								mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								//ヨー
								mine.key[6] = (CheckHitKey(KEY_INPUT_Q) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								mine.key[7] = (CheckHitKey(KEY_INPUT_E) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) == 0);
								//スロットル
								mine.key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
								mine.key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
								//脚
								mine.key[10] = false;
								//ブレーキ
								mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
								//精密操作
								mine.key[12] = (CheckHitKey(KEY_INPUT_W) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								mine.key[13] = (CheckHitKey(KEY_INPUT_S) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								mine.key[14] = (CheckHitKey(KEY_INPUT_D) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								mine.key[15] = (CheckHitKey(KEY_INPUT_A) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								mine.key[16] = (CheckHitKey(KEY_INPUT_Q) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
								mine.key[17] = (CheckHitKey(KEY_INPUT_E) != 0) && (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
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
						}
						//マウスと視点角度をリンク
						if (!replay_on) {
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
								mouse_aim(eyevec);
							}
						}
						else {
							mouse_aim(eyevec2);
						}
					}
					//反映
					if (!replay_on) {
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
								{
									if (veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] > 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] > 0) {
										easing_set(&veh.xradadd_right, (c.key[2] ? -rad_spec / 3.f : c.key[12] ? -rad_spec / 9.f : 0.f), 0.95f);
										easing_set(&veh.xradadd_left, (c.key[3] ? rad_spec / 3.f : c.key[13] ? rad_spec / 9.f : 0.f), 0.95f);
									}
									else {
										if (!(veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] == 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] == 0)) {
											easing_set(&veh.xradadd_right,
												float((-int(rad_spec / 12.f*1000.f) + GetRand(int(rad_spec / 12.f*2.f*1000.f)))) / 1000.f + (c.key[2] ? -rad_spec / 6.f : c.key[12] ? -rad_spec / 18.f : 0.f)
												, 0.95f);
											easing_set(&veh.xradadd_left,
												float((-int(rad_spec / 12.f*1000.f) + GetRand(int(rad_spec / 12.f*2.f*1000.f)))) / 1000.f + (c.key[3] ? rad_spec / 6.f : c.key[13] ? rad_spec / 18.f : 0.f)
												, 0.95f);
										}
										else {
											easing_set(&veh.xradadd_right,
												float((-int(rad_spec / 3.f*1000.f) + GetRand(int(rad_spec / 3.f*2.f*1000.f)))) / 1000.f + (c.key[2] ? -rad_spec / 12.f : c.key[12] ? -rad_spec / 36.f : 0.f)
												, 0.95f);
											easing_set(&veh.xradadd_left,
												float((-int(rad_spec / 3.f*1000.f) + GetRand(int(rad_spec / 3.f*2.f*1000.f)))) / 1000.f + (c.key[3] ? rad_spec / 12.f : c.key[13] ? rad_spec / 36.f : 0.f)
												, 0.95f);
										}
									}
								}
								//ロール
								{
									if (veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] > 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] > 0) {
										easing_set(&veh.zradadd_left, (c.key[5] ? (-rad_spec / 1.f) : (c.key[15] ? -rad_spec / 3.f : 0.f)), 0.95f);
										easing_set(&veh.zradadd_right, (c.key[4] ? (rad_spec / 1.f) : (c.key[14] ? rad_spec / 3.f : 0.f)), 0.95f);
									}
									else {
										if (!(veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] == 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] == 0)) {
											easing_set(&veh.zradadd_left,
												float((-int(rad_spec / 12.f*1000.f) + GetRand(int(rad_spec / 12.f*2.f*1000.f)))) / 1000.f + (c.key[5] ? (-rad_spec / 2.f) : (c.key[15] ? -rad_spec / 6.f : 0.f))
												, 0.95f);
											easing_set(&veh.zradadd_right,
												float((-int(rad_spec / 12.f*1000.f) + GetRand(int(rad_spec / 12.f*2.f*1000.f)))) / 1000.f + (c.key[4] ? (rad_spec / 2.f) : (c.key[14] ? rad_spec / 6.f : 0.f))
												, 0.95f);
										}
										else {
											easing_set(&veh.zradadd_left,
												float((-int(rad_spec / 6.f*1000.f) + GetRand(int(rad_spec / 6.f*2.f*1000.f)))) / 1000.f + (c.key[5] ? (-rad_spec / 4.f) : (c.key[15] ? -rad_spec / 12.f : 0.f))
												, 0.95f);
											easing_set(&veh.zradadd_right,
												float((-int(rad_spec / 6.f*1000.f) + GetRand(int(rad_spec / 6.f*2.f*1000.f)))) / 1000.f + (c.key[4] ? (rad_spec / 4.f) : (c.key[14] ? rad_spec / 12.f : 0.f))
												, 0.95f);
										}
									}
								}
								//ヨー
								{
									if (veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] > 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] > 0) {
										easing_set(&veh.yradadd_left, (c.key[6] ? -rad_spec / 24.f : (c.key[16] ? -rad_spec / 72.f : 0.f)), 0.95f);
										easing_set(&veh.yradadd_right, (c.key[7] ? rad_spec / 24.f : (c.key[17] ? rad_spec / 72.f : 0.f)), 0.95f);
									}
									else {
										if (!(veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] == 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] == 0)) {
											easing_set(&veh.yradadd_left,
												float((-int(rad_spec / 24.f*1000.f) + GetRand(int(rad_spec / 24.f*2.f*1000.f)))) / 1000.f + (c.key[6] ? -rad_spec / 48.f : (c.key[16] ? -rad_spec / 144.f : 0.f))
												, 0.95f);
											easing_set(&veh.yradadd_right,
												float((-int(rad_spec / 24.f*1000.f) + GetRand(int(rad_spec / 24.f*2.f*1000.f)))) / 1000.f + (c.key[7] ? rad_spec / 48.f : (c.key[17] ? rad_spec / 144.f : 0.f))
												, 0.95f);
										}
										else {
											easing_set(&veh.yradadd_left,
												float((-int(rad_spec / 9.f*1000.f) + GetRand(int(rad_spec / 9.f*2.f*1000.f)))) / 1000.f + (c.key[6] ? -rad_spec / 96.f : (c.key[16] ? -rad_spec / 288.f : 0.f))
												, 0.95f);
											easing_set(&veh.yradadd_right,
												float((-int(rad_spec / 9.f*1000.f) + GetRand(int(rad_spec / 9.f*2.f*1000.f)))) / 1000.f + (c.key[7] ? rad_spec / 96.f : (c.key[17] ? rad_spec / 288.f : 0.f))
												, 0.95f);
										}
									}
								}
								//スロットル
								if (veh.over_heat & (veh.accel >= 80.f)) {
									easing_set(&veh.accel_add, -200.f, 0.95f);
								}
								else {
									easing_set(&veh.accel_add, (c.key[8] ? 25.0f : (c.key[9] ? -25.0f : 0.f)), 0.95f);
								}

								veh.accel = std::clamp(veh.accel + veh.accel_add / GetFPS(), 0.f, 110.f);

								if (veh.accel >= 100.f) {
									//WIP
									easing_set(&veh.speed_add, (0.6f / 3.6f), 0.95f);//0.1km/h

									veh.WIP_timer += 1.0f / GetFPS();
									veh.WIP_timer_limit = 15.f*std::clamp(2.f - veh.speed / veh.use_veh.max_speed_limit, 0.f, 1.f);
									if (veh.WIP_timer >= veh.WIP_timer_limit) {
										//オーバーヒート
										veh.over_heat = true;
									}
								}
								else {

									veh.WIP_timer = std::max(veh.WIP_timer - 1.0f / GetFPS(), 0.f);
									if (veh.over_heat) {
										//オーバーヒート
										if (veh.WIP_timer <= 0.f) {
											veh.over_heat = false;
										}
									}

									if (veh.accel <= 50.f) {
										if (veh.speed <= ((veh.use_veh.mid_speed_limit*(veh.accel / 10.f) + veh.use_veh.min_speed_limit*0.5f *(5.f - (veh.accel / 10.f))) / 5.f)) {
											easing_set(&veh.speed_add, (((0.05f*(veh.accel / 10.f) + 0.2f *(5.f - (veh.accel / 10.f))) / 5.f) / 3.6f), 0.95f);//0.1km/h
										}
										else {
											easing_set(&veh.speed_add, (-((0.5f*(veh.accel / 10.f) + 0.85f *(5.f - (veh.accel / 10.f))) / 5.f) / 3.6f), 0.975f);//-0.05km/h
										}
									}
									else if (veh.speed >= ((veh.use_veh.mid_speed_limit*4.f + veh.use_veh.min_speed_limit*0.5f) / 5.f)) {
										if (veh.speed <= ((veh.use_veh.max_speed_limit*(veh.accel / 10.f - 5.f) + veh.use_veh.mid_speed_limit *(5.f - (veh.accel / 10.f - 5.f))) / 5.f)) {
											easing_set(&veh.speed_add, (((0.2f*(veh.accel / 10.f - 5.f) + 0.15f *(5.f - (veh.accel / 10.f - 5.f))) / 5.f) / 3.6f), 0.95f);//0.1km/h
										}
										else {
											easing_set(&veh.speed_add, (-((0.75f*(veh.accel / 10.f - 5.f) + 1.35f *(5.f - (veh.accel / 10.f - 5.f))) / 5.f) / 3.6f), 0.975f);//-0.05km/h
										}
									}
									else {//離昇出力
										easing_set(&veh.speed_add, (0.5f / 3.6f), 0.95f);//0.1km/h
									}
								}

								//スピード
								veh.speed += veh.speed_add * 60.f / GetFPS();
								{
									auto tmp = veh.mat.zvec();
									auto tmp2 = std::sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
									veh.speed += (((std::abs(tmp2) > std::sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / GetFPS(); //落下
								}
								//座標系反映
								{
									auto t_mat = veh.mat;
									veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / GetFPS());
									veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / GetFPS());
									veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / GetFPS());
								}
								//脚
								//c.changegear.get_in(c.key[10]);
								c.changegear.first = (veh.speed <= veh.use_veh.min_speed_limit*1.5f) && (veh.pos.y() <= 100.f);
								easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f);
								MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
								//舵
								for (auto& r : c.p_animes_rudder) {
									auto i = (size_t)(&r - &c.p_animes_rudder[0]);
									easing_set(&r.second, float(c.key[i + 2] + c.key[i + 12])*0.5f, 0.95f);
									MV1SetAttachAnimBlendRate(veh.obj.get(), r.first, r.second);
								}
								//
								{
									//
									if (veh.speed >= veh.use_veh.min_speed_limit) {
										easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f);
									}
									else {
										veh.add.yadd(M_GR / powf(GetFPS(), 2.f));
									}
									//
									if (c.p_anime_geardown.second >= 0.5f) {
										bool hit_f = false;
										for (auto& t : veh.use_veh.wheelframe) {
											t.math(mapparts, &c, &hit_f);
										}

										if (hit_f) {
											easing_set(&veh.wheel_Leftadd, -veh.speed / 20.f, 0.95f);
											easing_set(&veh.wheel_Rightadd, -veh.speed / 20.f, 0.95f);
										}
										else {
											easing_set(&veh.wheel_Leftadd, 0.f, 0.9f);
											easing_set(&veh.wheel_Rightadd, 0.f, 0.9f);
										}
										veh.wheel_Left += veh.wheel_Leftadd;
										veh.wheel_Right += veh.wheel_Rightadd;
										for (auto& f : veh.use_veh.wheelframe_nospring) {
											veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotAxis(MATRIX_ref::Vtrans(VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)), (f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *MATRIX_ref::Mtrans(f.frame.second));
										}
									}
									else {
										for (auto& t : veh.use_veh.wheelframe) {
											easing_set(&t.gndsmkeffcs.scale, 0.01f, 0.9f);
										}
									}
									veh.pos += veh.add + (veh.mat.zvec() * (-veh.speed / GetFPS()));
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
									veh.spawn(VGet(float(100 * (index / 3))*sin(deg2rad(-130)), 1000.f, float(100 * (index / 3))*cos(deg2rad(-130)) + float(100 * (index % 3))), MATRIX_ref::RotY(deg2rad(-130)));
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
								cg.math(c.key[((&cg - &veh.Gun_[0]) == 0) ? 0 : 1], &c);
							}
							//弾関連
							{
								//弾判定
								for (auto& cg : veh.Gun_) {
									cg.math_reco(mapparts, &c, &chara);
								}
								//弾痕
								for (auto& h : veh.hit_obj) {
									h.put(c);
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
											a.count += 1.f / GetFPS();
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
											a.count += 1.f / GetFPS();
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
								if (index != ef_smoke1 && index != ef_smoke2 && index != ef_smoke3) {
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

							for (auto& t : veh.use_veh.wingframe) {
								t.smkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), 10.f);
								if (start_c2) {
									t.smkeffcs.set_loop(Drawparts->get_effHandle(ef_smoke3));
								}
							}
						}
						start_c2 = false;
						//モデルに反映
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));

							for (auto& h : veh.HP_m) {
								size_t i = &h - &veh.HP_m[0];
								if (h > 0) {
									veh.info_break[i].add = veh.add;
									veh.info_break[i].speed = veh.speed;

									veh.info_break[i].pos = veh.pos;
									veh.info_break[i].mat = veh.mat;
									veh.info_break[i].per = 1.f;
								}
								else {
									veh.info_break[i].add.yadd(M_GR / powf(GetFPS(), 2.f));
									veh.info_break[i].pos += veh.info_break[i].add + (veh.info_break[i].mat.zvec() * (-veh.info_break[i].speed / GetFPS()));
									veh.info_break[i].per = std::max(veh.info_break[i].per - 1.f / GetFPS() / 10.f, 0.f);
								}
							}
						}
						for (auto& c : chara) {
							c.se_cockpit.SetPosition(c.vehicle.pos);
							c.se_hit.SetPosition(c.vehicle.pos);
							c.se_gun.SetPosition(c.vehicle.pos);
							c.se_missile.SetPosition(c.vehicle.pos);
						}
					}
					else {
						for (auto& cr : chara_rep) {//&cr-&chara_rep[0]
							cr->put_data(chara[&cr - &chara_rep[0]]);
						}
					}
					//アラート
					{
						if (mine.vehicle.pos.y() <= 30.f) {
							if (CheckSoundMem(se_alert.get()) != TRUE) {
								se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
							}
						}
						else {
							se_alert.stop();
						}
					}

					if (!replay_on) {
					}
					else {
						//cam_s.cam
						{
							if (sel_l == 0 && ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0)) {
								cam_s = *tcam;
								cam_s.cam.fov = deg2rad(fov_pc);
							}
							else {
								auto& veh = chara[sel_l].vehicle;
								//cam_s.cam
								{
									//campos,camvec,camup取得
									if (cam_s.Rot >= ADS) {
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
									//far取得
									cam_easy.far_ = (cam_s.Rot >= ADS) ? (500.f) : (20.f*(range_p - 5.f));
									//near取得
									cam_easy.near_ = (cam_s.Rot >= ADS) ? (3.f) : (range_p - 5.f);
									//fov
									cam_easy.fov = deg2rad(fov_pc / fovs);

									cam_s.cam = cam_easy;
								}
							}
						}
						//
						{
							start_stop.get_in(CheckHitKey(KEY_INPUT_SPACE) != 0);
							if (!start_stop.first) {
								bool tmp = false;
								for (auto& cr : chara_rep) {
									cr++;
									if (cr == chara[&cr - &chara_rep[0]].rep.end()) {
										tmp = true;
										break;
									}
								}
								if (tmp) {
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

								c.se_cockpit.SetPosition(veh.pos);
								c.se_hit.SetPosition(veh.pos);
								c.se_gun.SetPosition(veh.pos);
								c.se_missile.SetPosition(veh.pos);
								MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
								//舵
								for (int i = 0; i < c.p_animes_rudder.size(); i++) {
									MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
								}
								//車輪
								for (auto& f : veh.use_veh.wheelframe_nospring) {
									veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotAxis(MATRIX_ref::Vtrans(VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)), (f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *MATRIX_ref::Mtrans(f.frame.second));
								}
								//バーナー
								for (auto& be : c.p_burner) {
									veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(veh.speed / veh.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
								}
								//effect
								for (auto& t : c.effcs) {
									const size_t index = &t - &c.effcs[0];
									if (index != ef_smoke1 && index != ef_smoke2 && index != ef_smoke3) {
										t.put(Drawparts->get_effHandle(int(index)));
									}
								}
								for (auto& t : veh.use_veh.wheelframe) {
									t.gndsmkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), t.gndsmkeffcs.scale);
									if (start_c2) {
										t.gndsmkeffcs.set_loop(Drawparts->get_effHandle(ef_gndsmoke));
									}
								}
								for (auto& t : veh.use_veh.wingframe) {
									t.smkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), 10.f);
									if (start_c2) {
										t.smkeffcs.set_loop(Drawparts->get_effHandle(ef_smoke3));
									}
								}
								//銃砲
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
								//
							}
						}
						//
					}

					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos,
						[&] {
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							for (auto& h : veh.HP_m) {
								size_t i = &h - &veh.HP_m[0];
								if (h > 0) {
									veh.obj.DrawMesh(i);
								}
							}
						}
						mapparts->map_drawtree();
					}
					, VGet(200.f, 200.f, 200.f), VGet(2000.f, 2000.f, 2000.f));
					//VR更新
					if (!replay_on) {
						Drawparts->Move_Player();
					}

					//描画
					if (!replay_on) {
						{
							//自機描画
							{
								auto& veh = mine.vehicle;
								//cam_s.cam
								{
									//campos,camvec,camup取得
									if (cam_s.Rot >= ADS) {
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
									//far取得
									cam_easy.far_ = (cam_s.Rot >= ADS) ? (1500.f) : (60.f*(range_p - 5.f));
									//near取得
									cam_easy.near_ = (cam_s.Rot >= ADS) ? (3.f) : (range_p - 5.f);
									//fov
									cam_easy.fov = deg2rad(Drawparts->use_vr ? 90 : fov_pc / fovs);

									cam_s.cam = cam_easy;
								}
								//
								Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
								//コックピット演算
								if (cam_s.Rot >= ADS) {
									mine.cocks.ready_(mine);
								}
								//UI
								Hostpassparts->UI_Screen.SetDraw_Screen();
								{
									UIparts->draw(chara, Hostpassparts->MAIN_Screen, cam_s, mine.cocks, *Drawparts->get_device_hand1(), mine);
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
								//VRに移す
								Drawparts->draw_VR(
									[&] {
									SetCameraNearFar(0.01f, 2.f);
									SetUseZBuffer3D(FALSE);												//zbufuse
									SetWriteZBuffer3D(FALSE);											//zbufwrite
									DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, Drawparts->use_vr ? 1.8f : 1.475f / fovs, 0.f, Hostpassparts->MAIN_Screen.get(), TRUE);
									SetUseZBuffer3D(TRUE);												//zbufuse
									SetWriteZBuffer3D(TRUE);											//zbufwrite
									//コックピット
									if (cam_s.Rot >= ADS) {
										mine.cocks.obj.DrawModel();
									}
									//UI
									SetUseZBuffer3D(FALSE);												//zbufuse
									SetWriteZBuffer3D(FALSE);											//zbufwrite
									DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, Drawparts->use_vr ? 1.8f : 1.475f / fovs, 0.f, Hostpassparts->UI_Screen.get(), TRUE);
									SetUseZBuffer3D(TRUE);												//zbufuse
									SetWriteZBuffer3D(TRUE);											//zbufwrite
									//Hostpassparts->UI_Screen.DrawGraph(0, 0, true);
								}, cam_s.cam);
							}
							//カメラリプレイ
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
									mouse_aim(eyevec2);
									{
										cam_s.cam.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(VGet(0, 0.58f, 0), veh.mat);
										cam_s.cam.campos.y(std::max(cam_s.cam.campos.y(), 5.f));
										if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
											eyevec2 = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
										}
										cam_s.cam.camvec = cam_s.cam.campos - MATRIX_ref::Vtrans(eyevec2, veh.mat);
										cam_s.cam.camup = veh.mat.yvec();
									}
									//far取得
									cam_easy.far_ = (cam_s.Rot >= ADS) ? (2000.f) : (4000.f);
									//near取得
									cam_easy.near_ = (cam_s.Rot >= ADS) ? (3.f) : (range_p - 5.f);
									//fov
									cam_s.cam.fov = deg2rad(fov_pc / fovs);
								}
								//コックピット演算
								{
									ct.cocks.ready_(ct);
								}
								//UI
								Hostpass2parts->UI_Screen.SetDraw_Screen();
								{
									UIparts->draw(chara, Hostpass2parts->MAIN_Screen, cam_s, ct.cocks, *Drawparts->get_device_hand1(), ct, 0);
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
								//Screen2に移す
								outScreen2.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, cam_s.cam.near_, cam_s.cam.far_);
								{
									Hostpass2parts->MAIN_Screen.DrawGraph(0, 0, true);
									SetCameraNearFar(0.01f, 2.f);
									//コックピット
									ct.cocks.obj.DrawModel();
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
						//
					}
					else {

						Set3DSoundListenerPosAndFrontPosAndUpVec(cam_s.cam.campos.get(), cam_s.cam.camvec.get(), cam_s.cam.camup.get());
						//コックピット演算
						if (cam_s.Rot >= ADS) {
							chara[sel_l].cocks.ready_(chara[sel_l]);
						}
						//UI
						Hostpass2parts->UI_Screen.SetDraw_Screen();
						{
							UIparts->draw(chara, Hostpass2parts->MAIN_Screen, cam_s, chara[sel_l].cocks, *Drawparts->get_device_hand1(), chara[sel_l], 0);
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
						//VRに移す
						outScreen2.SetDraw_Screen(cam_s.cam.campos, cam_s.cam.camvec, cam_s.cam.camup, cam_s.cam.fov, cam_s.cam.near_, cam_s.cam.far_);
						{
							Hostpass2parts->MAIN_Screen.DrawGraph(0, 0, true);
							//コックピット
							if (cam_s.Rot >= ADS) {
								SetCameraNearFar(0.01f, 2.f);
								chara[sel_l].cocks.obj.DrawModel();
							}
							//UI
							Hostpass2parts->UI_Screen.DrawGraph(0, 0, true);
						}
						//draw
						GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
						{
							outScreen2.DrawGraph(0, 0, false);
							font18.DrawString(10, 10, "REPLAY", GetColor(255, 0, 0));
							if (start_stop.first) {
								font18.DrawString(10, 10 + 20, "|>", GetColor(255, 0, 0));
							}
							else {
								font18.DrawString(10, 10 + 20, "||", GetColor(255, 0, 0));
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
					//リプレイ
					if (!replay_on) {
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
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
				for (auto& c : chara) {
					c.se_cockpit.stop(); //gun
					c.se_gun.stop(); //gun
					c.se_missile.stop();
					c.se_hit.stop(); //gun
				}
			}
			//保存
			if (!replay_on) {
				std::ofstream fout;
				fout.open("data/file.txt", std::ios::binary);
				size_t size_c = chara.size();
				fout.write((char *)&size_c, sizeof(size_c));

				std::vector< std::list<Mainclass::Chara::sendstat>::iterator> chara_rep;
				for (auto& c : chara) {
					chara_rep.emplace_back(c.rep.begin());
				}
				auto tcam = rep_cam.begin();

				float bar = 0.f;
				int all = (int)(chara[0].rep.size());
				int now = all;
				ClearDrawScreen();
				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					if (now > 0) {
						{
							for (auto& cr : chara_rep) {
								cr->write(fout);
							}
							fout.write((char *)&tcam->cam, sizeof(tcam->cam));
							fout.write((char *)&tcam->Rot, sizeof(tcam->Rot));
						}
						{
							bool tmp = false;
							for (auto& cr : chara_rep) {
								cr++;
								if (cr == chara[&cr-&chara_rep[0]].rep.end()) {
									tmp = true;
									break;
								}
							}
							if (tmp) {
								break;
							}
							tcam++;
							if (tcam == rep_cam.end()) {
								break;
							}
						}
						now--;
					}
					else {
						break;
					}
					SetDrawScreen(DX_SCREEN_BACK);
					{
						DrawBox(Drawparts->out_disp_x - font18.GetDrawWidthFormat("リプレイデータ保存中… %04d/%04d  ", all - now, all), Drawparts->out_disp_y - y_r(70, Drawparts->out_disp_y), Drawparts->out_disp_x, Drawparts->out_disp_y - y_r(40, Drawparts->out_disp_y), GetColor(0, 0, 0), TRUE);
						font18.DrawStringFormat_RIGHT(Drawparts->out_disp_x, Drawparts->out_disp_y - y_r(70, Drawparts->out_disp_y), GetColor(0, 255, 0), "リプレイデータ保存中… %04d/%04d  ", all - now, all);
						DrawBox(0, Drawparts->out_disp_y - y_r(50, Drawparts->out_disp_y), int(float(Drawparts->out_disp_x) * bar / float(all)), Drawparts->out_disp_y - y_r(40, Drawparts->out_disp_y), GetColor(0, 255, 0), TRUE);
						easing_set(&bar, float(all - now), 0.95f);
					}
					Drawparts->Screen_Flip(waits);
				}
				fout.close();  //ファイルを閉じる
			}
			//解放
			{
				for (auto& c : chara) {
					c.rep.clear();
					auto& veh = c.vehicle;
					//エフェクト
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
					for (auto& t : veh.use_veh.wingframe) {
						t.smkeffcs.handle.Dispose();
					}
					veh.Dispose();
				}
				chara.clear();
				mapparts->delete_map();
				Drawparts->Delete_Shadow();
			}
			//
		} while (ProcessMessage() == 0 && ending);
	}
};