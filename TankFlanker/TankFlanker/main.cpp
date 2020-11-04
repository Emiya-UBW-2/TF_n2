#include "sub.hpp"
#include "UI.hpp"
#include "HostPass.hpp"
#include "map.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	//設定読み込み
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;
	{
		SetOutApplicationLogValidFlag(FALSE);  /*log*/
		int mdata = FileRead_open("data/setting.txt", FALSE);
		dof_e = getparams::_bool(mdata);
		bloom_e = getparams::_bool(mdata);
		shadow_e = getparams::_bool(mdata);
		useVR_e = getparams::_bool(mdata);
		FileRead_close(mdata);
	}
	//
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker", 90.f,useVR_e,shadow_e);		 /*汎用クラス*/
	auto UIparts = std::make_unique<UI>(Drawparts->out_disp_x, Drawparts->out_disp_y, Drawparts->disp_x, Drawparts->disp_y);		 /*UI*/
	auto Debugparts = std::make_unique<DeBuG>(90);						 /*デバッグ*/
	auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, Drawparts->disp_x, Drawparts->disp_y);	 /*ホストパスエフェクト*/
	DXDraw::cam_info cams;
	VECTOR_ref eyevec;					    //視点
	VECTOR_ref aimpos;			    //機体の狙い
	VECTOR_ref aimposout;					    //UIに出力
	GraphHandle BufScreen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);    //描画スクリーン
	GraphHandle UI_Screen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);    //描画スクリーン
	GraphHandle MAIN_Screen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y, true);    //描画スクリーン
	/*map*/
	auto mapparts = std::make_unique<Mapclass>(Drawparts->disp_x, Drawparts->disp_y);
	GraphHandle SkyScreen;
	SkyScreen = GraphHandle::Make(Drawparts->disp_x, Drawparts->disp_y);    //空描画
//その他
	MV1 hit_pic;      //弾痕
	frames	stickx_f, sticky_f, stickz_f,
		compass_f,
		speed_f, spd3_f, spd2_f, spd1_f
		;
	VECTOR_ref cockpit_v;
	MV1 cockpit;
	//操作
	cams.fov = deg2rad(Drawparts->use_vr ? 90 : 45);
	bool ads = false;
	int Rot = 0;
	float range = 0.f;
	float range_p = 30.f;

	VECTOR_ref eye_pos_ads = VGet(0.f, 0.58f, 0.f);
	//ロックオン
	VECTOR_ref aimposout_lockon;
	switchs lock_on;
	size_t tgt = 0;
	float distance = 0.f;
	//データ
	std::vector<Mainclass::Chara> chara;						//キャラ
	std::vector<Mainclass::treePats> tree;					//壁をセット
	
	
	std::vector<Mainclass::Ammos> Ammo;						//弾薬
	std::vector<Mainclass::Vehcs> Vehicles;			/*車輛データ*/

	MV1::Load("data/model/hit/model.mv1", &hit_pic, true);			//弾痕

	Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles, true);		/**/
	MV1::Load("data/model/cockpit/model.mv1", &cockpit, false);/**/

	UIparts->load_window("車両モデル");					//ロード画面

	for (int i = 0; i < cockpit.frame_num(); i++) {
		std::string p = cockpit.frame_name(i);
		if (p.find("座席", 0) != std::string::npos) {
			cockpit_v = cockpit.frame(i);
		}
		else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
			compass_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
			//ジャイロコンパス
		}
		else if (p.find("スティック縦", 0) != std::string::npos) {
			stickx_f = { i,cockpit.frame(i) };
			stickz_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
		}
		else if ((p.find("ペダル", 0) != std::string::npos) && (p.find("右", 0) == std::string::npos) && (p.find("左", 0) == std::string::npos)) {
			sticky_f = { i,cockpit.frame(i) };
		}
		else if ((p.find("速度計", 0) != std::string::npos)) {
			speed_f = { i,cockpit.frame(i) };
		}
		else if ((p.find("速度100", 0) != std::string::npos)) {
			spd3_f = { i,cockpit.frame(i) };
		}
		else if ((p.find("速度010", 0) != std::string::npos)) {
			spd2_f = { i,cockpit.frame(i) };
		}
		else if ((p.find("速度001", 0) != std::string::npos)) {
			spd1_f = { i,cockpit.frame(i) };
		}
	}

	Mainclass::Ammos::set_ammos(&Ammo);							//弾薬
	Mainclass::Vehcs::set_vehicles(&Vehicles);					//車輛
	
	VECTOR_ref HMDpos;
	MATRIX_ref HMDmat;


	VECTOR_ref rec_HMD;
	bool oldv = false;
	bool start_c = true;
	//ココから繰り返し読み込み//-------------------------------------------------------------------
	bool ending = true;
	do {
		oldv = false;
		start_c = true;
		//キャラ選択
		chara.resize(5);
		if (!UIparts->select_window(&chara[0], &Vehicles)) {
			break;
		}
		//マップ読み込み
		mapparts->set_map_pre();
		UIparts->load_window("マップモデル");			   //ロード画面
		//壁
		mapparts->set_map(&tree);
		//
		Drawparts->Set_Light_Shadow(mapparts->map_get().mesh_maxpos(0),mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&mapparts] { mapparts->map_get().DrawModel(); });
		if (shadow_e) {
		}
		//キャラ選択
		{
			//飛行機
			chara[0].vehicle.pos = VGet(0.f, 10.f, 0.f);
			chara[0].vehicle.mat = MGetIdent();

			//飛行機
			chara[1].vehicle.pos = VGet(0.f, 10.f, 30.f);
			chara[1].vehicle.mat = MGetIdent();
			//飛行機
			chara[2].vehicle.pos = VGet(0.f, 10.f, 60.f);
			chara[2].vehicle.mat = MGetIdent();
			//飛行機
			chara[3].vehicle.pos = VGet(0.f, 10.f, 90.f);
			chara[3].vehicle.mat = MGetIdent();

			//飛行機
			chara[4].vehicle.pos = VGet(0.f, 10.f, 120.f);
			chara[4].vehicle.mat = MGetIdent();
		}
		//キャラ設定
		fill_id(chara); //ID
		for (auto& c : chara) {
			c.set_human(Vehicles, Ammo, hit_pic, 9);//
		}
		//必要な時に当たり判定をリフレッシュする(仮)
		auto ref_col = [&chara](const Mainclass::Chara& p, const VECTOR_ref& startpos, const VECTOR_ref& endpos) {
			for (auto& c : chara) {
				auto& veh = c.vehicle;
				if (p.id == c.id || veh.hit_check) {
					continue;
				}
				if ((Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > 5.f)) {
					continue;
				}
				veh.col.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
				for (int i = 0; i < veh.col.mesh_num(); i++) {
					veh.col.RefreshCollInfo(-1, i);
				}
				veh.hit_check = true;
			}
			//
		};
		//影に描画するものを指定する(仮)
		auto draw_in_shadow_sky = [&chara, &tree] {
			for (auto& c : chara) {
				c.vehicle.obj.DrawModel();
			}
			for (auto& l : tree) {
				l.obj.DrawModel();
			}
		};
		auto draw_on_shadow = [&]() {
			Drawparts->Draw_by_Shadow(
				[&]() {
					//マップ
					SetFogStartEnd(30000.0f, 60000.f);
					SetFogColor(128, 128, 128);
					{
						mapparts->map_get().DrawModel();
					}
					//海
					mapparts->sea_draw(cams.campos);
					//機体
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 128, 128);
					{
						for (auto& c : chara) {
							//戦闘機
							c.vehicle.obj.DrawModel();
							//弾痕
							for (auto& h : c.vehicle.hit_obj) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);
								}
							}
						}
						for (auto& l : tree) {
							l.obj.DrawModel();
							//l.obj_far.DrawModel();
						}
						//

					}
					SetFogEnable(FALSE);
					SetUseLighting(FALSE);
					for (auto& c : chara) {
						for (auto& g : c.vehicle.Gun_) {
							for (auto& a : g.bullet) {
								if (a.flug) {
									DXDraw::Capsule3D(a.pos, a.repos, (((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - cams.campos).size() / 24.f))*4.5f, a.color, GetColor(255, 255, 255));
								}
							}
						}
					}
					SetUseLighting(TRUE);
					SetFogEnable(TRUE);
				}
			);
		};
		//通信開始
		{
		}
		//開始
		auto& mine = chara[0];
		Rot = 0;
		tgt = chara.size();
		eyevec = mine.vehicle.mat.zvec() * -1.f;
		cams.campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
		for (auto& c : chara) {
			for (auto& g : c.vehicle.use_veh.wheelframe) {
				g.gndsmkeffcs.handle = Drawparts->get_effHandle(ef_gndsmoke).Play3D();
				g.gndsmksize = 0.1f;
			}
		}
		SetMouseDispFlag(FALSE);
		SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
		while (ProcessMessage() == 0) {
			const auto fps = GetFPS();
			const auto waits = GetNowHiPerformanceCount();
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
			Debugparts->put_way();
			//プレイヤー操作
			{
				//スコープ
				if (Drawparts->use_vr) {
					ads = true;
				}
				else {
					Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
					switch (Rot) {
					case 2:
						break;
					case 1:
						range_p = 15.f;
						break;
					case 0:
						range_p = 30.f;
						break;
					}
					ads = (Rot >= 2);
					easing_set(&range, range_p, 0.9f);
				}
				//砲塔旋回
				{
					lock_on.second = std::min<uint8_t>(lock_on.second + 1, ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) ? 2 : 0);
					if (lock_on.second == 1) {
						if (lock_on.first == true) {
							lock_on.first = false;
							lock_on.second = 2;
						}
					}
					if (lock_on.second == 1) {
						if (lock_on.first == false && tgt != chara.size()) {
							lock_on.first = true;
							lock_on.second = 2;
						}
					}

					if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 && !lock_on.first) { //砲塔ロック
						mine.view_yrad = 0.f;
						mine.view_xrad = 0.f;
					}
				}
				//キー
				{
					//通常、VR共通
					{
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
						mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
						//精密操作
						mine.key[12] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
						//着艦フックスイッチ
						mine.key[13] = (CheckHitKey(KEY_INPUT_X) != 0);
						//カタパルト
						mine.key[14] = (CheckHitKey(KEY_INPUT_SPACE) != 0);
					}
					//VR専用
					if (Drawparts->use_vr) {
						if (Drawparts->get_hand1_num() != -1) {
							auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
							if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
								//メイン武器
								mine.key[0] |= ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Trigger)) != 0);
								//サブ武器
								mine.key[1] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B)) != 0);
								//ピッチ
								mine.key[2] |= (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(20)));
								mine.key[3] |= (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-20)));
								//ロール
								mine.key[4] |= (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(20)));
								mine.key[5] |= (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-20)));
								if ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)) != 0) {
									//ヨー
									mine.key[6] |= (ptr_LEFTHAND.touch.x() > 0.5f);
									mine.key[7] |= (ptr_LEFTHAND.touch.x() < -0.5f);
									//スロットル
									mine.key[8] |= (ptr_LEFTHAND.touch.y() > 0.5f);
									mine.key[9] |= (ptr_LEFTHAND.touch.y() < -0.5f);
									//ブレーキ
									if ((ptr_LEFTHAND.touch.x() >= -0.5f) && (ptr_LEFTHAND.touch.x() <= 0.5f) && (ptr_LEFTHAND.touch.y() >= -0.5f) && (ptr_LEFTHAND.touch.y() <= 0.5f)) {
										mine.key[11] |= true;
									}
								}
								//脚

								//精密操作
								mine.key[12] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_A)) != 0);
							}
						}
					}
				}
				//マウスと視点角度をリンク
				if (Drawparts->use_vr) {
					//+視点取得
					auto& ptr_ = (*Drawparts->get_device())[Drawparts->get_hmd_num()];
					Drawparts->GetDevicePositionVR(Drawparts->get_hmd_num(), &HMDpos, &HMDmat);
					if (start_c && (ptr_.turn && ptr_.now) != oldv) {
						rec_HMD = VGet(HMDpos.x(), 0.f, HMDpos.z());
						start_c = false;
					}
					if (!start_c && (ptr_.turn && ptr_.now) ==false) {
						start_c = true;
					}
					oldv = ptr_.turn && ptr_.now;
					HMDpos = HMDpos - rec_HMD;
					HMDmat = MATRIX_ref::Axis1(HMDmat.xvec()*-1.f, HMDmat.yvec(), HMDmat.zvec()*-1.f);
					eye_pos_ads = HMDpos+VGet(0,-0.8f,0);
					eye_pos_ads = VGet(
						std::clamp(eye_pos_ads.x(), -0.18f, 0.18f),
						std::clamp(eye_pos_ads.y(), 0.f, 0.8f),
						std::clamp(eye_pos_ads.z(), -0.18f, 0.1f)
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
					eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
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
					easing_set(&veh.xradadd_right, ((c.key[2]) ? -(c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f);
					easing_set(&veh.xradadd_left, ((c.key[3]) ? (c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f);
					//ロール
					easing_set(&veh.zradadd_right, ((c.key[4]) ? (c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f);
					easing_set(&veh.zradadd_left, ((c.key[5]) ? -(c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f);
					//ヨー
					easing_set(&veh.yradadd_left, ((c.key[6]) ? -(c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f);
					easing_set(&veh.yradadd_right, ((c.key[7]) ? (c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f);
					//スロットル
					easing_set(&veh.speed_add, (((c.key[8]) && veh.speed < veh.use_veh.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f);
					easing_set(&veh.speed_sub, (c.key[9]) ? ((veh.speed > veh.use_veh.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f);
					//スピード
					veh.speed += (veh.speed_add + veh.speed_sub) * 60.f / fps;
					{
						auto tmp = veh.mat.zvec();
						auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
						veh.speed += (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //落下
					}
					//座標系反映
					{
						auto t_mat = veh.mat;
						veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / fps);
						veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / fps);
						veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / fps);
					}
					//
					c.landing.second = std::min<uint8_t>(c.landing.second + 1, uint8_t((c.key[13]) ? 2 : 0));
					if (c.landing.second == 1) {
						c.landing.first ^= 1;
					}
					//脚
					c.changegear.second = std::min<uint8_t>(c.changegear.second + 1, uint8_t((c.key[10]) ? 2 : 0));
					if (c.changegear.second == 1) {
						c.changegear.first ^= 1;
					}
					easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f);
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
					//舵
					for (int i = 0; i < c.p_animes_rudder.size(); i++) {
						easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2]), 0.95f);
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

						//着艦ワイヤ-処理
						{
							veh.obj.frame_reset(veh.use_veh.hook.first);
							veh.obj.SetFrameLocalMatrix(veh.use_veh.hook.first, MATRIX_ref::RotX(deg2rad(c.p_landing_per)) * MATRIX_ref::Mtrans(veh.use_veh.hook.second));
							easing_set(&c.p_landing_per, (c.landing.first) ? 20.f : 0.f, 0.95f);
						}

						if (c.p_anime_geardown.second >= 0.5f) {
							bool hit_f = false;
							for (auto& w : veh.use_veh.wheelframe) {
								easing_set(&w.gndsmksize, 0.01f, 0.9f);
								auto tmp = veh.obj.frame(int(w.frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
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
										w.gndsmksize = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
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

								veh.obj.SetFrameLocalMatrix(f.frame.first,
									MATRIX_ref::RotAxis(
										MATRIX_ref::Vtrans(
											VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)),
										(f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *

									MATRIX_ref::Mtrans(f.frame.second));
							}
						}
						else {
							for (auto& w : veh.use_veh.wheelframe) {
								easing_set(&w.gndsmksize, 0.01f, 0.9f);
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
					}
					if (hitb) {
						veh.pos = VGet(0.f, 10.f, 0.f);
						veh.mat = MGetIdent();
						veh.xradadd_right = 0.f;
						veh.xradadd_left = 0.f;
						veh.yradadd_left = 0.f;
						veh.yradadd_right = 0.f;
						veh.zradadd_right = 0.f;
						veh.zradadd_left = 0.f;
						veh.speed_add = 0.f;
						veh.speed_sub = 0.f;
						veh.speed = 0.f;
						veh.add = VGet(0.f, 0.f, 0.f);
						c.p_anime_geardown.second = 1.f;
						c.landing.first = false;
					}
					//バーナー
					for (auto& be : c.p_burner) {
						veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(c.vehicle.speed / c.vehicle.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
					}
				}
				//射撃
				for (int i = 0; i < veh.Gun_.size(); i++) {
						auto& cg = veh.Gun_[i];
						if (c.key[(i == 0) ? 0 : 1] && cg.loadcnt == 0 && cg.rounds > 0) {
							auto& u = cg.bullet[cg.usebullet];
							++cg.usebullet %= cg.bullet.size();
							//ココだけ変化
							u.spec = cg.Spec[0];
							u.spec.speed_a *= float(75 + GetRand(50)) / 100.f;
							u.pos = veh.obj.frame(cg.gun_info.frame2.first);
							u.vec = (veh.obj.frame(cg.gun_info.frame3.first) - veh.obj.frame(cg.gun_info.frame2.first)).Norm();
							//
							cg.loadcnt = cg.gun_info.load_time;
							cg.rounds = std::max<uint16_t>(cg.rounds - 1, 0);
							if (i == 0) {
								cg.fired = 1.f;
							}
							u.hit = false;
							u.flug = true;
							u.cnt = 0.f;
							u.yadd = 0.f;
							u.repos = u.pos;
							if (u.spec.type_a != 2) {
								c.effcs[ef_fire].set(veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
								if (u.spec.caliber_a >= 0.017f) {
									c.effcs_gun[c.gun_effcnt].first.set(veh.obj.frame(cg.gun_info.frame3.first), u.vec);
									c.effcs_gun[c.gun_effcnt].first.put(Drawparts->get_effHandle(ef_smoke2));
									c.effcs_gun[c.gun_effcnt].second = &u;
									c.effcs_gun[c.gun_effcnt].cnt = 0;
									++c.gun_effcnt %= c.effcs_gun.size();
								}
							}
							else {
								c.effcs_missile[c.missile_effcnt].first.set(veh.obj.frame(cg.gun_info.frame3.first), u.vec);
								c.effcs_missile[c.missile_effcnt].first.put(Drawparts->get_effHandle(ef_smoke1));
								c.effcs_missile[c.missile_effcnt].second = &u;
								c.effcs_missile[c.missile_effcnt].cnt = 0;
								++c.missile_effcnt %= c.effcs_missile.size();
							}
						}
						cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
						cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
					}
				/*effect*/
				{
					for (auto& t : c.effcs) {
						if (t.id != ef_smoke1 && t.id != ef_smoke2) {
							t.put(Drawparts->get_effHandle(int(t.id)));
						}
					}
					for (auto& t : c.vehicle.use_veh.wheelframe) {
						t.gndsmkeffcs.handle.SetPos(c.vehicle.obj.frame(int(t.frame.first + 1)));
						t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
					}
				}
			}
			//座標反映
			for (auto& c : chara) {
				//弾関連
				auto& veh = c.vehicle;
				{
					//弾判定
					for (auto& g : veh.Gun_) {
						for (auto& a : g.bullet) {
							float size = 3.f;
							for (int z = 0; z < int(size); z++) {
								if (a.flug) {
									a.repos = a.pos;
									a.pos += a.vec * (a.spec.speed_a / fps / size);
									//判定
									{
										bool ground_hit = false;
										VECTOR_ref normal;
										//戦車以外に当たる
										{
											for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
												auto hps = mapparts->map_col_line(a.repos, a.pos, i);
												if (hps.HitFlag) {
													a.pos = hps.HitPosition;
													normal = hps.Normal;
													ground_hit = true;
												}
											}
										}
										ref_col(c, a.pos, a.repos);
										auto hitplane = c.get_reco(chara, a);		//飛行機にあたる
										//その後処理
										switch (a.spec.type_a) {
										case 0: //AP
											if (!hitplane) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														c.effcs[ef_gndhit].set(a.pos + normal * (0.1f), normal);
													}
													else {
														c.effcs[ef_gndhit2].set(a.pos + normal * (0.1f), normal);
													}
													if ((a.vec.Norm().dot(normal)) <= cos(deg2rad(60))) {
														a.flug = false;
													}
													else {
														a.vec += normal * ((a.vec.dot(normal)) * -2.f);
														a.vec = a.vec.Norm();
														a.pos += a.vec * (0.01f);
														a.spec.pene_a /= 2.f;
													}
												}
											}
											if (a.flug) {
												a.spec.pene_a -= 1.0f / fps / size;
												a.spec.speed_a -= 5.f / fps / size;
												a.pos += VGet(0.f, a.yadd / size, 0.f);
											}
											break;
										case 1: //HE
											if (!hitplane) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														c.effcs[ef_gndhit].set(a.pos + normal * (0.1f), normal);
													}
													else {
														c.effcs[ef_gndhit2].set(a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												a.spec.speed_a -= 5.f / fps / size;
												a.pos += VGet(0.f, a.yadd / size, 0.f);
											}
											break;
										case 2: //ミサイル
											if (!hitplane) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														c.effcs[ef_gndhit].set(a.pos + normal * (0.1f), normal);
													}
													else {
														c.effcs[ef_gndhit2].set(a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												size_t id = chara.size();
												VECTOR_ref pos;
												float dist = (std::numeric_limits<float>::max)();
												for (auto& t : chara) {
													//弾関連
													if (c.id == t.id) {
														continue;
													}
													{
														auto& veh_t = t.vehicle;
														auto p = (veh_t.pos - a.pos).size();
														if (dist > p) {
															dist = p;
															id = t.id;
															pos = veh_t.pos;
														}
													}
												}
												if (id != chara.size()) {
													auto vec_a = (a.pos - pos).Norm();
													//反映
													auto vec_z = a.vec;
													float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
													float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
													float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
													float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
													float view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
													{
														float limit = deg2rad(30.f) / fps;
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

									//消す(2秒たった、スピードが100以下、貫通が0以下)
									if (a.cnt >= 2.f || a.spec.speed_a < 100.f || a.spec.pene_a <= 0.f) {
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
							a.cnt += 1.f / fps;
						}
					}
					//弾痕
					for (auto& h : veh.hit_obj) {
						if (h.flug) {
							h.pic.SetMatrix(h.mat* veh.mat*MATRIX_ref::Mtrans(veh.pos + MATRIX_ref::Vtrans(h.pos, veh.mat)));
						}
					}
				}
				for (auto& a : c.effcs_missile) {
					if (a.second != nullptr) {
						if (a.second->flug) {
							a.first.handle.SetPos(a.second->pos);
						}
						if (a.cnt != -1) {
							a.cnt++;
							if (a.cnt >= 3.f * GetFPS()) {
								a.first.handle.Stop();
								a.cnt = -1;
							}
						}
					}
				}
				for (auto& a : c.effcs_gun) {
					if (a.second != nullptr) {
						if (a.second->flug) {
							a.first.handle.SetPos(a.second->pos);
						}
						if (a.cnt != -1) {
							a.cnt++;
							if (a.cnt >= 3.f * GetFPS()) {
								a.first.handle.Stop();
								a.cnt = -1;
							}
						}
					}
				}
			}
			//木セット
			for (auto& l : tree) {
				l.obj.SetMatrix(l.mat * MATRIX_ref::Mtrans(l.pos));
				l.obj_far.SetMatrix(l.mat * MATRIX_ref::Mtrans(l.pos));
			}
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
			//影用意
			if (shadow_e) {
				Drawparts->Ready_Shadow(cams.campos, draw_in_shadow_sky, VGet(200.f, 200.f, 200.f), VGet(2000.f, 2000.f, 2000.f));
			}
			Drawparts->Move_Player();
			//描画
			{
				{
					//視点取得
					{
						//campos,camvec,camup
						{
							auto& veh = mine.vehicle;
							if (ads) {
								cams.campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
								cams.campos.y(std::max(cams.campos.y(), 5.f));
								if (Drawparts->use_vr) {
									cams.camvec = cams.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
									cams.camup = MATRIX_ref::Vtrans(HMDmat.yvec(), veh.mat);//veh.mat.yvec();
								}
								else {
									if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
										cams.camvec = cams.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
									}
									else {
										eyevec = MATRIX_ref::Vtrans(veh.mat.zvec(), veh.mat.Inverse());
										cams.camvec = cams.campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
									}
									cams.camup = veh.mat.yvec();
								}
							}
							else {
								cams.camvec = veh.pos + veh.mat.yvec() * (6.f);
								cams.camvec.y(std::max(cams.camvec.y(), 5.f));
								if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0) {
									cams.campos = cams.camvec + eyevec * range;
									cams.campos.y(std::max(cams.campos.y(), 0.f));
									if (mapparts->map_col_line_nearest(cams.camvec, &cams.campos)) {
										cams.campos = cams.camvec + (cams.campos - cams.camvec) * (0.9f);
									}
									cams.camup = VGet(0.f, 1.f, 0.f);
								}
								else {
									eyevec = (cams.camvec - aimpos).Norm();
									cams.campos = cams.camvec + eyevec * range;
									cams.camup = veh.mat.yvec();
								}
							}
						}
						//near,far
						{
							cams.far_ = 1.f;
							cams.near_ = 0.01f;
							if (ads) {
								VECTOR_ref aimingpos = cams.campos + (cams.camvec - cams.campos).Norm() * (3000.f);
								mapparts->map_col_line_nearest(cams.campos, &aimingpos);
								cams.far_ = std::clamp((cams.campos - aimingpos).size(), 300.f, 3000.f);
							}
							else {
								cams.far_ = 6000.f;
							}

							cams.near_ = ads ? (5.f + 25.f * (cams.far_ - 300.f) / (3000.f - 300.f)) : (range_p - 5.f);
						}
					}
					//照準座標取得
					{
						GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, cams.campos, cams.camvec, cams.camup, cams.fov, 0.01f, 5000.0f);
						{
							VECTOR_ref startpos, endpos;
							startpos = mine.vehicle.pos;
							endpos = startpos + mine.vehicle.mat.zvec() * (-1000.f);
							//地形
							mapparts->map_col_line_nearest(startpos, &endpos);
							//
							easing_set(&aimpos, endpos, 0.9f);
							aimposout = ConvWorldPosToScreenPos(aimpos.get());
						}
					}
				}
				//UI
				UI_Screen.SetDraw_Screen();
				{
					UIparts->draw(aimposout, mine, lock_on.first, distance, aimposout_lockon, Drawparts->use_vr, false);
					//VR用オプション
					if (Drawparts->use_vr) {
						UIparts->draw_in_vr(*Drawparts->get_device_hand1());
					}
					UIparts->draw_in_vr(mine);
				}
				//被写体深度描画
				SkyScreen.SetDraw_Screen(cams.campos - cams.camvec, VGet(0, 0, 0), cams.camup, cams.fov, 1.0f, 50.0f);
				{
					mapparts->sky_draw();
				}
				Hostpassparts->dof(&BufScreen, SkyScreen, draw_on_shadow, cams);
				//
				MAIN_Screen.SetDraw_Screen();
				{
					BufScreen.DrawGraph(0, 0, false);
					Hostpassparts->bloom(BufScreen, 255);//ブルーム
				}
				//VRに移す
				Drawparts->draw_VR(
					[&] {
						MAIN_Screen.DrawGraph(0, 0, true);
						//コックピット
						if (ads) {
							SetCameraNearFar(0.01f, 2.f);
							auto& veh = chara[0].vehicle;

							float px = (chara[0].p_animes_rudder[1].second - chara[0].p_animes_rudder[0].second)*deg2rad(30);
							float pz = (chara[0].p_animes_rudder[2].second - chara[0].p_animes_rudder[3].second)*deg2rad(30);
							float py = (chara[0].p_animes_rudder[5].second - chara[0].p_animes_rudder[4].second)*deg2rad(20);

							cockpit.SetFrameLocalMatrix(sticky_f.first, MATRIX_ref::RotY(py) * MATRIX_ref::Mtrans(sticky_f.second));
							cockpit.SetFrameLocalMatrix(stickz_f.first, MATRIX_ref::RotZ(pz) * MATRIX_ref::Mtrans(stickz_f.second));
							cockpit.SetFrameLocalMatrix(stickx_f.first, MATRIX_ref::RotX(px) * MATRIX_ref::Mtrans(stickx_f.second));
							cockpit.SetFrameLocalMatrix(compass_f.first, MATRIX_ref(veh.mat).Inverse() * MATRIX_ref::Mtrans(compass_f.second));
							{
								float spd_buf = veh.speed*3.6f;
								float spd = 0.f;
								if (spd_buf <= 400.f) {
									spd = 180.f*spd_buf / 440.f;
								}
								else {
									spd = 180.f*(400.f / 440.f + (spd_buf - 400.f) / 880.f);
								}
								cockpit.frame_reset(speed_f.first);
								cockpit.SetFrameLocalMatrix(speed_f.first, MATRIX_ref::RotAxis(MATRIX_ref::Vtrans(cockpit.frame(speed_f.first + 1) - cockpit.frame(speed_f.first), MATRIX_ref(veh.mat).Inverse()), -deg2rad(spd)) *						MATRIX_ref::Mtrans(speed_f.second));
							}
							{
								float spd_buf = veh.speed*3.6f / 1224.f;

								cockpit.SetFrameLocalMatrix(spd3_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*1.f)) * MATRIX_ref::Mtrans(spd3_f.second));
								cockpit.SetFrameLocalMatrix(spd2_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*10.f)) * MATRIX_ref::Mtrans(spd2_f.second));
								cockpit.SetFrameLocalMatrix(spd1_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*100.f)) * MATRIX_ref::Mtrans(spd1_f.second));
							}

							cockpit.SetMatrix(MATRIX_ref(veh.mat)*MATRIX_ref::Mtrans(veh.obj.frame(veh.use_veh.fps_view.first) - MATRIX_ref::Vtrans(cockpit_v, veh.mat)));
							cockpit.DrawModel();
						}
						//UI
						UI_Screen.DrawGraph(0, 0, true);
					},
					cams);
			}
			//draw
			GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK), false);
			{
				Drawparts->outScreen[0].DrawExtendGraph(0, 0, Drawparts->out_disp_x, Drawparts->out_disp_y, false);
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
		}
		SetMouseDispFlag(TRUE);
		SetMousePoint(Drawparts->disp_x / 2, Drawparts->disp_y / 2);
		//解放
		{
			for (auto& c : chara) {
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

				{
					auto& veh = c.vehicle;
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.handle.Dispose();
					}
					veh.reset();
				}
			}
			chara.clear();
			mapparts->delete_map(&tree);
			if (shadow_e) {
				Drawparts->Delete_Shadow();
			}
		}
		//
	}while (ProcessMessage() == 0 && ending);
	return 0; // ソフトの終了
}
