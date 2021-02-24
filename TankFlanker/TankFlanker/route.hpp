#pragma once
class main_c : Mainclass {
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;

	cam_info cam_easy;
	VECTOR_ref eyevec, eyevec2;																	//視点
	FontHandle font18;
	//描画スクリーン
	GraphHandle outScreen2;
	GraphHandle UI_Screen, UI_Screen2;
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
	sounds_3D se;

	SoundHandle se_alert;
	SoundHandle se_alert2;
	SoundHandle se_timer;
	SoundHandle bgm_title;
	SoundHandle bgm_main;
	SoundHandle bgm_win;

	float danger_height = 300.f;
	float se_vol = 0.35f;
	class voices {
	public:
		std::vector <std::string> str;
		std::vector<SoundHandle> handle;
		float timer = 0.f;
		int select = 0;
	};
	std::vector<voices> voice_;

	class voice_strs {
	public:
		std::string str;
		SoundHandle *handle;
	};
	std::vector<voice_strs> voice_str;
	//timer
	float timer = 0.f;
	float ready_timer = 0.f;
	//設定
	bool oldv = false;
	bool start_c = true;
	bool start_c2 = true;
	bool ending = true;
	bool ending_win = true;
	float fov_pc = 45.f;
	//
	struct keys {
		int mac = 0, px = 0, py = 0;
		char onhandle[256], offhandle[256];
	};
	struct keyhandle {
		keys key;
		GraphHandle onhandle, offhandle;
	};
	std::vector<keyhandle> keyg;
	std::vector < std::pair<int, std::string> > key_use_ID;
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
			fov_pc = getparams::_float(mdata);
			FileRead_close(mdata);
			SetOutApplicationLogValidFlag(TRUE);	//log
		}
		//
		auto Drawparts = std::make_unique<DXDraw>("TankFlanker", FRAME_RATE, useVR_e, shadow_e);						//汎用クラス
		auto UIparts = std::make_unique<UI>(Drawparts->disp_x, Drawparts->disp_y);										//UI
		auto Debugparts = std::make_unique<DeBuG>(FRAME_RATE);															//デバッグ
		auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);	//ホストパスエフェクト
		UI_Screen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);										//VR、フルスクリーン共用UI
		auto Hostpass2parts = std::make_unique<HostPassEffect>(dof_e, bloom_e, deskx, desky);							//ホストパスエフェクト
		UI_Screen2 = GraphHandle::Make(deskx, desky, true);																//フルスクリーン向けUI
		auto mapparts = std::make_unique<Mapclass>();																	//map
		outScreen2 = GraphHandle::Make(deskx, desky, true);	//描画スクリーン
		font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
		//その他
		se.Load();
		se_alert = SoundHandle::Load("data/audio/alert.wav");
		se_alert2 = SoundHandle::Load("data/audio/alert2.wav");
		se_timer = SoundHandle::Load("data/audio/timer.wav");

		bgm_title = SoundHandle::Load("data/audio/BGM/title.wav");
		bgm_main = SoundHandle::Load(std::string("data/audio/BGM/bgm")+std::to_string(GetRand(3))+".wav");
		bgm_win = SoundHandle::Load("data/audio/BGM/win.wav");

		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_W,"下降"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_A, "左ロール"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_S, "上昇"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_D, "右ロール"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_Q, "左ヨー"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_E, "右ヨー"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_R, "スロットル開く"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_F, "スロットル絞る"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_G, "ランディングブレーキ"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_LSHIFT, "精密動作"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_O, "タイトル画面に戻る"));
		key_use_ID.emplace_back(std::pair<int, std::string>(KEY_INPUT_ESCAPE, "強制終了"));
		{
			std::fstream file;
			/*
			std::vector<keys> key;
			file.open("data/1.dat", std::ios::binary | std::ios::out);
			for (auto& m : key)
				file.write((char*)&m, sizeof(m));
			file.close();
			//*/
			//*
			file.open("data/1.dat", std::ios::binary | std::ios::in);
			keys keytmp;
			while (true) {
				file.read((char*)&keytmp, sizeof(keytmp));
				if (file.eof()) {
					break;
				}
				for (auto& k : key_use_ID) {
					if (keytmp.mac == k.first) {
						keyg.resize(keyg.size() + 1);
						keyg.back().key = keytmp;
						keyg.back().onhandle = GraphHandle::Load(keyg.back().key.onhandle);
						keyg.back().offhandle = GraphHandle::Load(keyg.back().key.offhandle);
						break;
					}
				}
			}
			file.close();
			//*/
		}

		{
			WIN32_FIND_DATA win32fdt;
			HANDLE hFind;
			int pt = -1;
			hFind = FindFirstFile("data/audio/voice/*", &win32fdt);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (win32fdt.cFileName[0] != '.') {
						int ttm = win32fdt.cFileName[0] - '0';
						if (pt != ttm) {
							voice_.resize(voice_.size() + 1);
						}
						pt = ttm;
						voice_.back().handle.resize(voice_.back().handle.size() + 1);
						voice_.back().handle.back() = SoundHandle::Load(std::string("data/audio/voice/") + win32fdt.cFileName);
						voice_.back().str.resize(voice_.back().str.size() + 1);
						voice_.back().str.back() = win32fdt.cFileName;
						voice_.back().str.back() = voice_.back().str.back().substr(2, voice_.back().str.back().find('.') - 2);
					}
				} while (FindNextFile(hFind, &win32fdt));
			} //else{ return false; }
			FindClose(hFind);
		}
		//
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
				mapparts->sea_draw();
				//雲
				mapparts->cloud_draw();//2
				//機体
				SetFogStartEnd(0.0f, 3000.f);
				SetFogColor(128, 128, 128);
				{
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						//戦闘機
						for (auto& h : veh.HP_m) {
							size_t i = &h - &veh.HP_m[0];
							if (i >= 3) {
								if (h > 0) {
									veh.obj.DrawMesh(int(veh.use_veh.module_mesh[int(i - 3)].second));
								}
								else {
									if (veh.info_break[i].per > 0.1f) {
										veh.obj_break.SetMatrix(veh.info_break[i].mat * MATRIX_ref::Mtrans(veh.info_break[i].pos));
										veh.obj_break.DrawMesh(int(veh.use_veh.module_mesh[int(i - 3)].second));
									}
								}
							}
						}
						for (int i = 0; i < veh.use_veh.module_mesh[0].second; i++) {
							veh.obj.DrawMesh(i);
						}
					}
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
			chara.resize(24);
			{
				oldv = false;
				start_c = true;
				start_c2 = true;
				//キャラ選択
				bgm_title.play(DX_PLAYTYPE_LOOP, TRUE);
				bgm_title.vol(int(float(255)*0.5f));
				if (!UIparts->select_window(&chara[0], &Vehicles, Drawparts)) {
					break;
				}
				bgm_title.stop();
				//マップ読み込み
				mapparts->set_map_pre();
				UIparts->load_window("マップモデル");			   //ロード画面
				mapparts->set_map("data/grassput.bmp", 35000.f, 35000.f, -35000.f, -35000.f);
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
					if (i >= (chara.size() / 2)) {
						c.id = 1;
					}

					float rad = deg2rad(-130);
					c.vehicle.spawn(VGet(

						float(-2000 + 4000 * int(i / (chara.size() / 2)))*sin(rad) + float(100 * (i % (chara.size() / 2)))*cos(rad),
						1500.f,
						float(-2000 + 4000 * int(i / (chara.size() / 2)))*cos(rad) - float(100 * (i % (chara.size() / 2)))*sin(rad)
					), MATRIX_ref::RotY(deg2rad(((c.id == 0) ? 180 : 0) - 130)));

					//キャラ設定
					c.set_human(Vehicles, Ammo);	//
					c.cocks.set_(cockpit);			//コックピット
					//
					c.se.Duplicate(se);
				}
				se_alert.vol(int(float(192)*se_vol));
				se_alert2.vol(int(float(192)*se_vol));
				//開始共通
				eye_pos_ads = VGet(0, 0.58f, 0);
				auto& mine = chara[0];
				switchs start_stop;
				//開始
				{
					eyevec = mine.vehicle.mat.zvec() * -1.f;
					cam_s.cam.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
					cam_s.Rot = ADS;
					eyevec2 = chara[0].vehicle.mat.zvec() * -1.f;
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						for (auto& t : veh.use_veh.wheelframe) {
							t.init();
						}
					}
				}
				for (auto& c : chara) {
					c.se.cockpit.play(DX_PLAYTYPE_LOOP, TRUE);
					c.se.cockpit.vol(int(float(128)*se_vol));
					c.se.engine.play(DX_PLAYTYPE_LOOP, TRUE);
				}

				bgm_main.play(DX_PLAYTYPE_LOOP, TRUE);

				bgm_main.vol(int(float(255)*0.5f));
				bgm_win.vol(int(float(255)*0.5f));
				SetMouseDispFlag(FALSE);
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
				voice_[0].select = GetRand(int(voice_[0].handle.size() - 1));
				voice_[0].handle[voice_[0].select].play(DX_PLAYTYPE_BACK, TRUE);
				voice_[0].handle[voice_[0].select].vol(255);
				voice_str.resize(voice_str.size() + 1);
				voice_str.back().handle = &voice_[0].handle[0];
				voice_str.back().str = voice_[0].str[0];
				timer = 60.f*5.f;
				ready_timer = 5.f;
				while (ProcessMessage() == 0) {
					const auto waits = GetNowHiPerformanceCount();
					Debugparts->put_way();
					{
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
							c.aim_cnt = 0;
							c.missile_cnt = 0;
							easing_set(&veh.HP_r, float(veh.HP), 0.95f);

							c.se.engine.vol(int(float(128 + int(127.f * c.vehicle.accel / 100.f))*se_vol));

							if (veh.kill_f) {
								veh.kill_time -= 1.f / GetFPS();
								if (veh.kill_time <= 0.f) {
									veh.kill_time = 0.f;
									veh.kill_f = false;
								}
							}
							if (veh.HP == 0) {
								veh.HP_m[GetRand(int(veh.HP_m.size() - 1))] = 0;
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
							//
							for (auto& c : chara) {
								auto& veh = c.vehicle;
								if (&c - &chara[0] >= (Drawparts->use_vr ? 1 : 1)) {

									c.key[0] = false;//GetRand(100) <= 10;   //射撃
									c.key[1] = false;//GetRand(100) <= 1; //マシンガン

									bool ret = false;
									bool up = false;
									size_t id = chara.size();
									VECTOR_ref tgt_pos;

									float dist = (std::numeric_limits<float>::max)();
									for (auto& t : chara) {
										//弾関連
										if (&c == &t ||
											c.id == t.id ||
											(c.vehicle.pos - t.vehicle.pos).size() >= c.vehicle.use_veh.canlook_dist ||
											t.aim_cnt > 2
											) {
											continue;
										}
										auto p = (t.vehicle.pos - veh.pos).size();
										if (dist > p) {
											dist = p;
											id = &t - &chara[0];
											tgt_pos = t.vehicle.pos;
										}
									}

									if (id == chara.size()) {
										tgt_pos = veh.pos - veh.mat.zvec();
									}
									else {
										chara[id].aim_cnt++;
									}
									{
										auto tmpv = veh.pos;
										tmpv.y(0);
										if (tmpv.size() >= 5000) {
											tgt_pos = VGet(0, 0, 0);
											ret = true;
										}
										if (veh.pos.y() <= 400) {
											tmpv.y(500);
											tgt_pos = tmpv;
											up = true;
										}
									}

									VECTOR_ref tgt_zvec = (tgt_pos - veh.pos).Norm();
									VECTOR_ref my_xvec = veh.mat.xvec();
									VECTOR_ref my_yvec = veh.mat.yvec();
									VECTOR_ref my_zvec = veh.mat.zvec();
									//ピッチ
									{
										if (id == chara.size() && !ret && !up) {
											tgt_zvec.y(0.f);
											tgt_zvec = tgt_zvec.Norm();
										}
										auto tgt_yvec = tgt_zvec.cross(my_xvec);
										auto cross_yvec = tgt_zvec.cross(tgt_yvec);
										auto cross_vec = tgt_zvec.cross(my_zvec);
										auto dot = cross_vec.dot(cross_yvec);
										if (dot >= 0.1f) {
											c.key[2] = GetRand(10) <= 5;
											c.key[3] = false;
											c.key[12] = GetRand(10) <= 5;
											c.key[13] = false;
										}
										else if (dot <= -0.1f) {
											c.key[2] = false;
											c.key[3] = GetRand(10) <= 5;
											c.key[12] = false;
											c.key[13] = GetRand(10) <= 5;
										}
										else {
											c.key[2] = false;
											c.key[3] = false;
											if (dot >= 0.f) {
												c.key[12] = true;
												c.key[13] = false;
											}
											else {
												c.key[12] = false;
												c.key[13] = true;
											}
											if (id != chara.size() && !ret && !up) {
												if ((c.vehicle.mat.zvec()).dot(tgt_pos - c.vehicle.pos) < 0) {
													if ((tgt_pos - c.vehicle.pos).size() <= 300) {
														c.key[0] = GetRand(100) <= 20;   //射撃
													}
													if ((tgt_pos - c.vehicle.pos).size() <= 1500) {
														c.key[1] = GetRand(200) <= 1; //マシンガン
													}
												}
											}
										}
									}
									//ロール
									{
										if (id == chara.size() && !ret && !up) {
											tgt_zvec = VGet(0, 1, 0);
										}
										auto tgt_xvec = tgt_zvec.cross(my_zvec).Norm();
										auto tgt_yvec = tgt_xvec.cross(my_zvec);
										auto cross_vec = tgt_xvec.cross(my_xvec);
										auto dot = cross_vec.dot(tgt_xvec.cross(tgt_yvec));
										if (dot >= 0.1f) {
											c.key[4] = GetRand(10) <= 5;
											c.key[5] = false;
											c.key[14] = GetRand(10) <= 5;
											c.key[15] = false;
										}
										else if (dot <= -0.1f) {
											c.key[4] = false;
											c.key[5] = GetRand(10) <= 5;
											c.key[14] = false;
											c.key[15] = GetRand(10) <= 5;
										}
										else {
											c.key[4] = false;
											c.key[5] = false;
											if (dot >= 0.f) {
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
							//通常、VR共通
							if (!Drawparts->use_vr) {
								mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
								mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
								mine.key[2] = (CheckHitKey(key_use_ID[0].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								mine.key[3] = (CheckHitKey(key_use_ID[2].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								mine.key[4] = (CheckHitKey(key_use_ID[3].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								mine.key[5] = (CheckHitKey(key_use_ID[1].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								//ヨー
								mine.key[6] = (CheckHitKey(key_use_ID[4].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								mine.key[7] = (CheckHitKey(key_use_ID[5].first) != 0) && (CheckHitKey(key_use_ID[9].first) == 0);
								//スロットル
								mine.key[8] = (CheckHitKey(key_use_ID[6].first) != 0);
								mine.key[9] = (CheckHitKey(key_use_ID[7].first) != 0);
								//脚
								mine.key[10] = false;
								//ブレーキ
								mine.key[11] = (CheckHitKey(key_use_ID[8].first) != 0);
								//精密操作
								mine.key[12] = (CheckHitKey(key_use_ID[0].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);
								mine.key[13] = (CheckHitKey(key_use_ID[2].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);
								mine.key[14] = (CheckHitKey(key_use_ID[3].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);
								mine.key[15] = (CheckHitKey(key_use_ID[1].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);

								mine.key[16] = (CheckHitKey(key_use_ID[4].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);
								mine.key[17] = (CheckHitKey(key_use_ID[5].first) != 0) && (CheckHitKey(key_use_ID[9].first) != 0);
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
								if (&c != &mine && (c.id == mine.id)) {
									if (c.vehicle.HP > 0) {
										if (c.key[0] && GetRand(100) < 10) {
											if (CheckSoundMem(voice_[1].handle[voice_[1].select].get()) != TRUE && voice_[1].timer == 0.f) {
												voice_[1].select = GetRand(int(voice_[1].handle.size() - 1));
												voice_[1].handle[voice_[1].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[1].handle[voice_[1].select].vol(255);
												voice_[1].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[1].handle[voice_[1].select];
												voice_str.back().str = voice_[1].str[voice_[1].select];
											}
										}
										if (c.key[1] && GetRand(100) < 10) {
											if (CheckSoundMem(voice_[2].handle[voice_[2].select].get()) != TRUE && voice_[2].timer == 0.f) {
												voice_[2].select = GetRand(int(voice_[2].handle.size() - 1));
												voice_[2].handle[voice_[2].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[2].handle[voice_[2].select].vol(255);
												voice_[2].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[2].handle[voice_[2].select];
												voice_str.back().str = voice_[2].str[voice_[2].select];
											}
										}
										if (c.vehicle.hitf) {
											voice_[3].select = GetRand(int(voice_[3].handle.size() - 1));
											if (CheckSoundMem(voice_[3].handle[voice_[3].select].get()) != TRUE && voice_[3].timer == 0.f) {
												voice_[3].handle[voice_[3].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[3].handle[voice_[3].select].vol(255);
												voice_[3].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[3].handle[voice_[3].select];
												voice_str.back().str = voice_[3].str[voice_[3].select];
											}
											c.vehicle.hitf = false;
										}
										if (c.vehicle.killf) {
											voice_[4].select = GetRand(int(voice_[4].handle.size() - 1));
											if (CheckSoundMem(voice_[4].handle[voice_[4].select].get()) != TRUE && voice_[4].timer == 0.f) {
												voice_[4].handle[voice_[4].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[4].handle[voice_[4].select].vol(255);
												voice_[4].timer = 0.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[4].handle[voice_[4].select];
												voice_str.back().str = voice_[4].str[voice_[4].select];
											}
											c.vehicle.killf = false;
										}
										if (c.aim_cnt > 0 && GetRand(100) < 10) {
											voice_[5].select = GetRand(int(voice_[5].handle.size() - 1));
											if (CheckSoundMem(voice_[5].handle[voice_[5].select].get()) != TRUE && voice_[5].timer == 0.f) {
												voice_[5].handle[voice_[5].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[5].handle[voice_[5].select].vol(255);
												voice_[5].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[5].handle[voice_[5].select];
												voice_str.back().str = voice_[5].str[voice_[5].select];
											}
										}
										if (c.missile_cnt > 0 && GetRand(100) < 10) {
											voice_[6].select = GetRand(int(voice_[6].handle.size() - 1));
											if (CheckSoundMem(voice_[6].handle[voice_[6].select].get()) != TRUE && voice_[6].timer == 0.f) {
												voice_[6].handle[voice_[6].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[6].handle[voice_[6].select].vol(255);
												voice_[6].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[6].handle[voice_[6].select];
												voice_str.back().str = voice_[6].str[voice_[6].select];
											}
										}
										if (c.vehicle.dmgf) {
											voice_[7].select = GetRand(int(voice_[7].handle.size() - 1));
											if (CheckSoundMem(voice_[7].handle[voice_[7].select].get()) != TRUE && voice_[7].timer == 0.f) {
												voice_[7].handle[voice_[7].select].play(DX_PLAYTYPE_BACK, TRUE);
												voice_[7].handle[voice_[7].select].vol(255);
												voice_[7].timer = float(GetRand(170) + 30) / 10.f;
												voice_str.resize(voice_str.size() + 1);
												voice_str.back().handle = &voice_[7].handle[voice_[7].select];
												voice_str.back().str = voice_[7].str[voice_[7].select];
											}
											c.vehicle.dmgf = false;
										}
									}
									if (c.vehicle.deathf) {
										voice_[8].select = GetRand(int(voice_[8].handle.size() - 1));
										if (CheckSoundMem(voice_[8].handle[voice_[8].select].get()) != TRUE && voice_[8].timer == 0.f) {
											voice_[8].handle[voice_[8].select].play(DX_PLAYTYPE_BACK, TRUE);
											voice_[8].handle[voice_[8].select].vol(255);
											voice_[8].timer = 0.f;
											voice_str.resize(voice_str.size() + 1);
											voice_str.back().handle = &voice_[8].handle[voice_[8].select];
											voice_str.back().str = voice_[8].str[voice_[8].select];
										}
										c.vehicle.deathf = false;
									}
								}
							}
							//
							for (auto&v : voice_) {
								v.timer = std::max(v.timer - 1.f / GetFPS(), 0.f);
							}
							for (auto& v : voice_str) {
								if (CheckSoundMem(v.handle->get()) != TRUE) {
									voice_str.erase(voice_str.begin() + (&v - &voice_str[0]));
									break;
								}
							}
						}
						//マウスと視点角度をリンク
						{
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
								eye_pos_ads = HMDpos + VGet(0, -0.42f, 0);
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
					}
					//反映
					for (auto& c : chara) {
						auto& veh = c.vehicle;
						{
							//飛行機演算
							{
								float rad_spec = deg2rad(veh.use_veh.body_rad_limit * (veh.use_veh.mid_speed_limit / veh.speed));
								if (veh.speed < veh.use_veh.min_speed_limit) {
									rad_spec = deg2rad(veh.use_veh.body_rad_limit * (std::clamp(veh.speed, 0.f, veh.use_veh.min_speed_limit) / veh.use_veh.min_speed_limit));
								}
								//ピッチ
								/*
								if(&c==&mine)
								{
									if (veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] > 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] > 0) {
										easing_set(&veh.xradadd_right, (c.key[2] ? -rad_spec * 3.f / 3.f : c.key[12] ? -rad_spec * 3.f / 9.f : 0.f), 0.95f);
										easing_set(&veh.xradadd_left, (c.key[3] ? rad_spec * 3.f / 3.f : c.key[13] ? rad_spec * 3.f / 9.f : 0.f), 0.95f);
									}
									else {
										if (!(veh.HP_m[c.vehicle.use_veh.module_mesh[0].first] == 0 && veh.HP_m[c.vehicle.use_veh.module_mesh[1].first] == 0)) {
											easing_set(&veh.xradadd_right,
												float((-int(rad_spec * 3.f / 12.f*1000.f) + GetRand(int(rad_spec * 3.f / 12.f*2.f*1000.f)))) / 1000.f + (c.key[2] ? -rad_spec * 3.f / 6.f : c.key[12] ? -rad_spec * 3.f / 18.f : 0.f)
												, 0.95f);
											easing_set(&veh.xradadd_left,
												float((-int(rad_spec * 3.f / 12.f*1000.f) + GetRand(int(rad_spec * 3.f / 12.f*2.f*1000.f)))) / 1000.f + (c.key[3] ? rad_spec * 3.f / 6.f : c.key[13] ? rad_spec * 3.f / 18.f : 0.f)
												, 0.95f);
										}
										else {
											easing_set(&veh.xradadd_right,
												float((-int(rad_spec * 3.f / 3.f*1000.f) + GetRand(int(rad_spec * 3.f / 3.f*2.f*1000.f)))) / 1000.f + (c.key[2] ? -rad_spec * 3.f / 12.f : c.key[12] ? -rad_spec * 3.f / 36.f : 0.f)
												, 0.95f);
											easing_set(&veh.xradadd_left,
												float((-int(rad_spec * 3.f / 3.f*1000.f) + GetRand(int(rad_spec * 3.f / 3.f*2.f*1000.f)))) / 1000.f + (c.key[3] ? rad_spec * 3.f / 12.f : c.key[13] ? rad_spec * 3.f / 36.f : 0.f)
												, 0.95f);
										}
									}
								}
								else
								//*/
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
								{
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
								}
								//スピード
								{
									veh.speed += veh.speed_add * 60.f / GetFPS();
									{
										auto tmp = veh.mat.zvec();
										auto tmp2 = std::sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
										veh.speed += (((std::abs(tmp2) > std::sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / GetFPS(); //落下
									}
								}
								//座標系反映
								{
									auto t_mat = veh.mat;
									veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / GetFPS());
									veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / GetFPS());
									veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / GetFPS());
								}
								//脚演算
								{
									//c.changegear.get_in(c.key[10]);
									c.changegear.first = (veh.speed <= veh.use_veh.min_speed_limit*1.5f) && (veh.pos.y() <= 100.f);
									easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f);
								}
								//舵演算
								for (auto& r : c.p_animes_rudder) {
									auto i = (size_t)(&r - &c.p_animes_rudder[0]);
									easing_set(&r.second, float(c.key[i + 2] + c.key[i + 12])*0.5f, 0.95f);
								}
								//車輪その他
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
								//死亡関連
								{
									if (c.death) {
										c.death_timer -= 1.f / GetFPS();
										if (c.death_timer <= 0.f) {
											c.death = false;
											c.death_timer = 0.f;
											veh.respawn();
											c.p_anime_geardown.second = 1.f;
											c.changegear.first = true;
										}
									}
								}
								//壁の当たり判定
								{
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
									if (hitb && !c.death) {
										veh.HP = 0;
										c.death_timer = 3.f;
										c.death = true;
									}
								}
							}
							//射撃
							{
								for (auto& cg : veh.Gun_) {
									c.ms_key = c.key[((&cg - &veh.Gun_[0]) == 0) ? 0 : 1];
									if (c.ms_key && (&cg - &veh.Gun_[0]) != 0) {
										if (!c.ms_on) {
											c.ms_key = false;
										}
										if (cg.loadcnt == 0) {
											c.ms_on = false;
										}
									}
									cg.math(c.ms_key, &c);
								}

								if (!c.ms_on) {
									c.ms_cnt += 1.f / GetFPS();
									if (c.ms_cnt >= 8.f) {
										c.ms_cnt = 0.f;
										c.ms_on = true;
									}
								}
							}
							//弾関連
							{
								//弾判定
								for (auto& cg : veh.Gun_) {
									cg.math_reco(mapparts, &c, &chara);
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
											if (a.count >= 4.5f) {
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
											if (a.count >= 4.5f) {
												a.first.handle.Stop();
												a.count = -1.f;
											}
										}
									}
								}
							}
						}
						//飛行機演算共通
						{
							//舵
							for (auto& r : c.p_animes_rudder) {
								MV1SetAttachAnimBlendRate(veh.obj.get(), r.first, r.second);
							}
							//脚
							MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
							//バーナー
							for (auto& be : c.p_burner) {
								veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(veh.speed / veh.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
							}
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
							if (/*start_c2*/ c.death) {
								if (c.death_timer == 3.f) {
									t.smkeffcs.set_loop(Drawparts->get_effHandle(ef_smoke3));
								}
							}
							else {
								t.smkeffcs.handle.Stop();
							}
						}
						//銃砲
						for (auto& t : c.effcs_gun) {
							t.first.put(Drawparts->get_effHandle(ef_smoke2));
						}
						//ミサイル
						for (auto& t : c.effcs_missile) {
							t.first.put(Drawparts->get_effHandle(ef_smoke1));
						}
						//モデルに反映
						{
							veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
							//破損表現
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
									veh.info_break[i].add.yadd(M_GR / powf(GetFPS(), 2.f)/2.f);
									veh.info_break[i].pos += veh.info_break[i].add + (veh.info_break[i].mat.zvec() * (-veh.info_break[i].speed / GetFPS()));
									veh.info_break[i].per = std::max(veh.info_break[i].per - 1.f / GetFPS() / 10.f, 0.f);
								}
							}
						}
					}
					//
					start_c2 = false;
					//サウンド
					{
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							c.se.cockpit.SetPosition(veh.pos);
							c.se.engine.SetPosition(veh.pos);
							c.se.hit.SetPosition(veh.pos);
							c.se.gun.SetPosition(veh.pos);
							c.se.missile.SetPosition(veh.pos);
						}
						//アラート
						{
							bool ttttt = false;
							if (mine.vehicle.speed < mine.vehicle.use_veh.min_speed_limit) {
								if (CheckSoundMem(se_alert2.get()) != TRUE) {
									se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
								}
								ttttt = true;
							}
							if (mine.missile_cnt > 0) {
								if (CheckSoundMem(se_alert2.get()) != TRUE) {
									se_alert2.play(DX_PLAYTYPE_LOOP, TRUE);
								}
								ttttt = true;
							}
							if (!ttttt) {
								se_alert2.stop();
							}
							ttttt = false;
							if (mine.vehicle.pos.y() <= danger_height) {
								if (CheckSoundMem(se_alert.get()) != TRUE) {
									se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
								}
								ttttt = true;
							}
							if (mine.aim_cnt > 0) {
								if (CheckSoundMem(se_alert.get()) != TRUE) {
									se_alert.play(DX_PLAYTYPE_LOOP, TRUE);
								}
								ttttt = true;
							}
							if (!ttttt) {
								se_alert.stop();
							}
						}
					}
					//影用意
					Drawparts->Ready_Shadow(cam_s.cam.campos,
						[&] {
						for (auto& c : chara) {
							auto& veh = c.vehicle;
							for (auto& h : veh.HP_m) {
								size_t i = &h - &veh.HP_m[0];
								if (h > 0) {
									veh.obj.DrawMesh(int(i));
								}
							}
						}
					}
					, VGet(200.f, 200.f, 200.f), VGet(2000.f, 2000.f, 2000.f));
					//VR更新
					Drawparts->Move_Player();
					//描画
					mapparts->sea_draw_set();
					{
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
								UI_Screen.SetDraw_Screen();
								{
									UIparts->draw(mine, cam_s.Rot >= ADS, *Drawparts->get_device_hand1(), danger_height, Drawparts->use_vr);
								}
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
									//被写体深度描画
									Hostpassparts->BUF_draw([&]() { mapparts->sky_draw(); }, ram_draw, tmp_cams.cam);
									//最終描画
									Hostpassparts->MAIN_draw();

									GraphHandle::SetDraw_Screen(tmp, tmp_cams.cam.campos, tmp_cams.cam.camvec, tmp_cams.cam.camup, tmp_cams.cam.fov, tmp_cams.cam.near_, tmp_cams.cam.far_);
									{
										Hostpassparts->get_main().DrawGraph(0, 0, true);


										SetCameraNearFar(0.01f, 2.f);
										//コックピット
										if (tmp_cams.Rot >= ADS) {
											mine.cocks.obj.DrawModel();
										}
										if (Drawparts->use_vr) {
											//UI
											SetUseZBuffer3D(FALSE);												//zbufuse
											SetWriteZBuffer3D(FALSE);											//zbufwrite
											DrawBillboard3D((cam_s.cam.campos + (cam_s.cam.camvec - cam_s.cam.campos).Norm()*1.0f).get(), 0.5f, 0.5f, 1.8f, 0.f, UI_Screen.get(), TRUE);
											SetUseZBuffer3D(TRUE);												//zbufuse
											SetWriteZBuffer3D(TRUE);											//zbufwrite
										}
										else {
											this->UI_Screen.DrawGraph(0, 0, TRUE);
										}
										UIparts->item_draw(chara, mine, tmp_cams.Rot >= ADS, danger_height, Drawparts->use_vr);
										//
										{
											int yyy = 30;
											for (auto& v : voice_str) {
												if (CheckSoundMem(v.handle->get()) == TRUE) {
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
										{
											if ((ready_timer <= 0.f) || (ready_timer > 0.f && ((GetNowHiPerformanceCount() / 100000) % 10 > 5))) {
												font18.DrawStringFormat_MID(Drawparts->disp_x / 2, Drawparts->disp_y / 2 - Drawparts->disp_y / 6, GetColor(255, 255, 0), "%02d:%02d", int(timer) / 60, int(timer) % 60);
											}
											if (timer <= 0.f) {
												//リザルト
												UIparts->res_draw(mine, Drawparts->use_vr);
											}
										}
									}
								}, cam_s.cam);
							}
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

							{
								int xp_s = 100, yp_s = 300, y_size = 25;
								for (auto& m : keyg) {
									for (auto& i : key_use_ID) {
										if (m.key.mac == i.first) {
											if (CheckHitKey(m.key.mac) != 0) {
												m.onhandle.DrawRotaGraph(xp_s + y_size / 2, yp_s + y_size / 2, float(y_size - 2) / 30.f, 0.f, false);
											}
											else {
												m.offhandle.DrawRotaGraph(xp_s + y_size / 2, yp_s + y_size / 2, float(y_size - 2) / 30.f, 0.f, false);
											}
											font18.DrawString(xp_s + y_size * 2, yp_s + y_size - 18, i.second, GetColor(255, 255, 255)); yp_s += y_size;
										}
									}
								}
							}

							Debugparts->end_way();
							Debugparts->debug(10, 10, float(GetNowHiPerformanceCount() - waits) / 1000.f);
						}
					}
					Drawparts->Screen_Flip(waits);
					if (CheckHitKey(key_use_ID[11].first) != 0) {
						ending = false;
						break;
					}
					if (CheckHitKey(key_use_ID[10].first) != 0) {
						break;
					}
					//
					{
						if (ready_timer <= 0.f) {
							timer -= 1.f / GetFPS();
							if (timer <= 0.f && ending_win) {
								bgm_main.stop();
								bgm_win.play(DX_PLAYTYPE_BACK, TRUE);
								ending_win = false;
							}
							if (timer <= 0.f) {
								timer = 0.f;
							}
						}
						else {
							ready_timer -= 1.f / GetFPS();

							if ((GetNowHiPerformanceCount() / 100000) % 10 == 0) {
								if (CheckSoundMem(se_timer.get()) != TRUE) {
									se_timer.play(DX_PLAYTYPE_BACK, TRUE);
								}
							}
						}
					}
				}

				ending_win = true;
				bgm_main.stop();
				bgm_win.stop();

				SetMouseDispFlag(TRUE);
				SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
				for (auto& c : chara) {
					c.se.engine.stop();
					c.se.cockpit.stop(); //gun
					c.se.gun.stop(); //gun
					c.se.missile.stop();
					c.se.hit.stop(); //gun
				}
				se_alert.stop();
				se_alert2.stop();
			}
			//解放
			{
				for (auto& c : chara) {
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