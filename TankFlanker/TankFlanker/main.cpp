#include "sub.hpp"
#include "UI.hpp"
#include "HostPass.hpp"
#include "map.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	//�ݒ�ǂݍ���
	bool dof_e = false;
	bool bloom_e = false;
	bool shadow_e = false;
	bool useVR_e = true;
	int dispx = 1080; /*�`��X*/
	int dispy = 1200; /*�`��Y*/
	int out_dispx = dispx * 960 / dispy; /*�E�B���h�EX*/
	int out_dispy = dispy * 960 / dispy; /*�E�B���h�EY*/
	{
		SetOutApplicationLogValidFlag(FALSE);  /*log*/
		int mdata = FileRead_open("data/setting.txt", FALSE);
		dof_e = getparams::_bool(mdata);
		bloom_e = getparams::_bool(mdata);
		shadow_e = getparams::_bool(mdata);
		useVR_e = getparams::_bool(mdata);
		FileRead_close(mdata);
	}
	//DXLib�`��
	//��ʎw��
	if (useVR_e) {
		dispx = 1080 * 2;
		dispy = 1200 * 2;
		out_dispx = dispx * 960 / dispy;
		out_dispy = dispy * 960 / dispy;
	}
	else {
		dispx = 1920;
		dispy = 1080;
		out_dispx = dispx;
		out_dispy = dispy;
	}
	//
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker", 90.f,useVR_e);		 /*�ėp�N���X*/
	auto UIparts = std::make_unique<UI>(out_dispx, out_dispy, dispx, dispy);		 /*UI*/
	auto Debugparts = std::make_unique<DeBuG>(90);						 /*�f�o�b�O*/
	auto Hostpassparts = std::make_unique<HostPassEffect>(dof_e, bloom_e, dispx, dispy);	 /*�z�X�g�p�X�G�t�F�N�g*/
	/*BOX2D*/
	VECTOR_ref eyevec;					    //���_
	VECTOR_ref campos, camvec, camup;			    //�J����
	VECTOR_ref aimpos;			    //�@�̂̑_��
	VECTOR_ref aimposout;					    //UI�ɏo��
	GraphHandle BufScreen = GraphHandle::Make(dispx, dispy);    //�`��X�N���[��
	GraphHandle outScreen = GraphHandle::Make(dispx, dispy);    //�`��X�N���[��
	/*map*/
	auto mapparts = std::make_unique<Mapclass>(dispx, dispy);
	//���̑�
	MV1 hit_pic;      //�e��
	//����
	float fov = deg2rad(useVR_e ? 90 : 45);
	bool ads = false;
	int Rot = 0;
	float ratio = 1.f;
	float range = 0.f;
	float range_p = 30.f;

	VECTOR_ref eye_pos_ads = VGet(0.f, 0.58f, 0.f);
	//���b�N�I��
	VECTOR_ref aimposout_lockon;
	switchs lock_on;
	size_t tgt = 0;
	float distance = 0.f;
	//�f�[�^
	std::vector<Mainclass::Chara> chara;						//�L����
	std::vector<Mainclass::treePats> tree;					//�ǂ��Z�b�g
	MV1::Load("data/model/hit/model.mv1", &hit_pic, true);			//�e��
	std::vector<Mainclass::Ammos> Ammo;						//�e��
	std::array<std::vector<Mainclass::Vehcs>, veh_all> Vehicles;			/*���p�f�[�^*/
	Mainclass::Vehcs::set_vehicles_pre("data/tank/", &Vehicles[0], true);		/**/
	Mainclass::Vehcs::set_vehicles_pre("data/plane/", &Vehicles[1], true);		/**/
	Mainclass::Vehcs::set_vehicles_pre("data/carrier/", &Vehicles[2], true);	/**/
	UIparts->load_window("�ԗ����f��");					//���[�h���
	Mainclass::Ammos::set_ammos(&Ammo);							//�e��
	Mainclass::Vehcs::set_vehicles(&Vehicles);					//���p
	VECTOR_ref HMDpos, HMDxvec, HMDyvec, HMDzvec;
	MATRIX_ref HMDmat;

	//�R�R����J��Ԃ��ǂݍ���//-------------------------------------------------------------------
	bool ending = true;
	do {
		//�L�����I��
		chara.resize(1);
		if (!UIparts->select_window(&chara[0], &Vehicles)) {
			break;
		}
		//�}�b�v�ǂݍ���
		mapparts->set_map_pre();
		UIparts->load_window("�}�b�v���f��");			   //���[�h���
		//��
		mapparts->set_map(&tree);
		//
		Drawparts->Set_Light_Shadow(mapparts->map_get().mesh_maxpos(0),mapparts->map_get().mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&mapparts] { mapparts->map_get().DrawModel(); });
		if (shadow_e) {
		}
		//�L�����I��
		{
			//��s�@
			chara.back().vehicle.pos = VGet(0.f, 10.f, 0.f);
			chara.back().vehicle.mat = MGetIdent();
		}
		//�L�����ݒ�
		fill_id(chara); //ID
		for (auto& c : chara) {
			c.set_human(Vehicles, Ammo, hit_pic);
		}
		//�K�v�Ȏ��ɓ����蔻������t���b�V������(��)
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
		//�e�ɕ`�悷����̂��w�肷��(��)
		auto draw_in_shadow_sky = [&chara, &tree] {
			for (auto& c : chara) {
				c.vehicle.obj.DrawModel();
			}
			for (auto& l : tree) {
				l.obj.DrawModel();
			}
		};
		auto draw_on_shadow = [&]() {
			//�}�b�v
			SetFogStartEnd(0.0f, 30000.f);
			SetFogColor(128, 128, 128);
			{
				mapparts->map_get().DrawModel();
			}
			//�C
			mapparts->sea_draw(campos);
			//�@��
			SetFogStartEnd(0.0f, 3000.f);
			SetFogColor(128, 128, 128);
			{
				for (auto& c : chara) {
					//�퓬�@
					c.vehicle.obj.DrawModel();
					//�e��
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
			}
			SetFogEnable(FALSE);
			SetUseLighting(FALSE);
			for (auto& c : chara) {
				for (auto& g : c.vehicle.Gun_) {
					for (auto& a : g.bullet) {
						if (a.flug) {
							DXDraw::Capsule3D(a.pos, a.repos, (((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f))*(1.f / std::max(ratio / 8.5f, 1.f)), a.color, GetColor(255, 255, 255));
						}
					}
				}
			}
			SetUseLighting(TRUE);
			SetFogEnable(TRUE);
		};
		//�ʐM�J�n
		{
		}
		//�J�n
		auto& mine = chara[0];
		Rot = 0;
		tgt = chara.size();
		eyevec = mine.vehicle.mat.zvec() * -1.f;
		campos = mine.vehicle.pos + VGet(0.f, 3.f, 0.f) + eyevec * range;
		for (auto& c : chara) {
			for (auto& g : c.vehicle.use_veh.wheelframe) {
				//g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
				g.gndsmksize = 0.1f;
			}
		}
		SetMouseDispFlag(FALSE);
		SetMousePoint(dispx / 2, dispy / 2);
		while (ProcessMessage() == 0) {
			const auto fps = GetFPS();
			const auto waits = GetNowHiPerformanceCount();
			for (auto& c : chara) {
				auto& veh = c.vehicle;
				//�����蔻�胊�t���b�V��
				if (veh.hit_check) {
					veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = false;
				}
			}
			Debugparts->put_way();
			//�v���C���[����
			{
				//�X�R�[�v
				{
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
					ratio = 1.f;
					ads = (Rot >= 2);
					easing_set(&range, range_p, 0.9f, fps);
				}
				//�C������
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

					if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 && !lock_on.first) { //�C�����b�N
						mine.view_yrad = 0.f;
						mine.view_xrad = 0.f;
					}
				}
				//�L�[
				{
					//�ʏ�AVR����
					{
						mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //�ˌ�
						mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //�}�V���K��
						mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
						mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
						mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
						mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
						//���[
						mine.key[6] = (CheckHitKey(KEY_INPUT_Q) != 0);
						mine.key[7] = (CheckHitKey(KEY_INPUT_E) != 0);
						//�X���b�g��
						mine.key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
						mine.key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
						//�r
						mine.key[10] = (CheckHitKey(KEY_INPUT_C) != 0);
						mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
						//��������
						mine.key[12] = (CheckHitKey(KEY_INPUT_LSHIFT) != 0);
						//���̓t�b�N�X�C�b�`
						mine.key[13] = (CheckHitKey(KEY_INPUT_X) != 0);
						//�J�^�p���g
						mine.key[14] = (CheckHitKey(KEY_INPUT_SPACE) != 0);
					}
					if (useVR_e) {
						if (Drawparts->get_hand1_num() != -1) {
							auto& ptr_LEFTHAND = *Drawparts->get_device_hand1();
							if (ptr_LEFTHAND.turn && ptr_LEFTHAND.now) {
								//���C��
								mine.key[0] |= ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Trigger)) != 0);
								//�T�u
								mine.key[1] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B)) != 0);
								//�s�b�`
								mine.key[2] |= (ptr_LEFTHAND.yvec.y() > sinf(deg2rad(20)));
								mine.key[3] |= (ptr_LEFTHAND.yvec.y() < sinf(deg2rad(-20)));
								//���[��
								mine.key[4] |= (ptr_LEFTHAND.zvec.x() > sinf(deg2rad(20)));
								mine.key[5] |= (ptr_LEFTHAND.zvec.x() < sinf(deg2rad(-20)));
								if ((ptr_LEFTHAND.on[0] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Touchpad)) != 0) {
									//���[
									mine.key[6] |= (ptr_LEFTHAND.touch.x() > 0.5f);
									mine.key[7] |= (ptr_LEFTHAND.touch.x() < -0.5f);
									//�X���b�g��
									mine.key[8] |= (ptr_LEFTHAND.touch.y() > 0.5f);
									mine.key[9] |= (ptr_LEFTHAND.touch.y() < -0.5f);
									//�u���[�L
									if (
										(ptr_LEFTHAND.touch.x() >= -0.5f) &&
										(ptr_LEFTHAND.touch.x() <= 0.5f) &&
										(ptr_LEFTHAND.touch.y() >= -0.5f) &&
										(ptr_LEFTHAND.touch.y() <= 0.5f)
										) {
										mine.key[11] |= true;
									}
								}
								//�r

								//��������
								mine.key[12] |= ((ptr_LEFTHAND.on[1] & vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_A)) != 0);
								//���̓t�b�N�X�C�b�`

								//�J�^�p���g

							}
						}
					}
				}
				//�}�E�X�Ǝ��_�p�x�������N
				if (useVR_e) {
					SetMousePoint(dispx / 2, dispy / 2);
					Drawparts->GetDevicePositionVR(Drawparts->get_hmd_num(), &HMDpos, &HMDmat);
					HMDmat = MATRIX_ref::Axis1(HMDmat.xvec()*-1.f, HMDmat.yvec(), HMDmat.zvec()*-1.f);
					eye_pos_ads = HMDmat.yvec() * -0.5f + VGet(HMDpos.x()*-1.f, HMDpos.y(), HMDpos.z()*-1.f);
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
					SetMousePoint(dispx / 2, dispy / 2);
					if (ads) {
						float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f / ratio);
						float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f / ratio);
						x = std::clamp(x, deg2rad(-45), deg2rad(45));
						eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
					}
					else {
						float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2) * 0.1f);
						float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2) * 0.1f);
						x = std::clamp(x, deg2rad(-25), deg2rad(89));
						eyevec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
					}
				}
			}
			{
				//���̃L�[���͂������Ŏ擾(�z�X�g)
			}
			//���f
			for (auto& c : chara) {
				auto& veh = c.vehicle;
				//��s�@���Z
				{
					float rad_spec = deg2rad(veh.use_veh.body_rad_limit * (veh.use_veh.mid_speed_limit / veh.speed));
					if (veh.speed < veh.use_veh.min_speed_limit) {
						rad_spec = deg2rad(veh.use_veh.body_rad_limit * (std::clamp(veh.speed, 0.f, veh.use_veh.min_speed_limit) / veh.use_veh.min_speed_limit));
					}
					//�s�b�`
					easing_set(&veh.xradadd_right, ((c.key[2]) ? -(c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
					easing_set(&veh.xradadd_left, ((c.key[3]) ? (c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
					//���[��
					easing_set(&veh.zradadd_right, ((c.key[4]) ? (c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
					easing_set(&veh.zradadd_left, ((c.key[5]) ? -(c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
					//���[
					easing_set(&veh.yradadd_left, ((c.key[6]) ? -(c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
					easing_set(&veh.yradadd_right, ((c.key[7]) ? (c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
					//�X���b�g��
					easing_set(&veh.speed_add, (((c.key[8]) && veh.speed < veh.use_veh.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f, fps);
					easing_set(&veh.speed_sub, (c.key[9]) ? ((veh.speed > veh.use_veh.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f, fps);
					//�X�s�[�h
					veh.speed += (veh.speed_add + veh.speed_sub) * 60.f / fps;
					{
						auto tmp = veh.mat.zvec();
						auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
						veh.speed += (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //����
					}
					//���W�n���f
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
					//�r
					c.changegear.second = std::min<uint8_t>(c.changegear.second + 1, uint8_t((c.key[10]) ? 2 : 0));
					if (c.changegear.second == 1) {
						c.changegear.first ^= 1;
					}
					easing_set(&c.p_anime_geardown.second, float(c.changegear.first), 0.95f, fps);
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
					//��
					for (int i = 0; i < c.p_animes_rudder.size(); i++) {
						easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2]), 0.95f, fps);
						MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
					}
					//
					{
						//
						if (veh.speed >= veh.use_veh.min_speed_limit) {
							easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f, fps);
						}
						else {
							veh.add.yadd(M_GR / powf(fps, 2.f));
						}

						//���̓��C��-����
						{
							veh.obj.frame_reset(veh.use_veh.hook.first);
							veh.obj.SetFrameLocalMatrix(veh.use_veh.hook.first, MATRIX_ref::RotX(deg2rad(c.p_landing_per)) * MATRIX_ref::Mtrans(veh.use_veh.hook.second));
							easing_set(&c.p_landing_per, (c.landing.first) ? 20.f : 0.f, 0.95f, fps);
						}

						if (c.p_anime_geardown.second >= 0.5f) {
							bool hit_f = false;
							for (auto& w : veh.use_veh.wheelframe) {
								easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
								auto tmp = veh.obj.frame(int(w.frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
								//�n��
								{
									auto hp = mapparts->map_col_line(tmp + (veh.mat.yvec() * (0.5f)), tmp, 0);
									if (hp.HitFlag == TRUE) {
										veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
										{
											auto normal = veh.mat.yvec();
											easing_set(&normal, hp.Normal, 0.95f, fps);
											veh.mat *= MATRIX_ref::RotVec2(veh.mat.yvec(), normal);
										}
										w.gndsmksize = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
										if (veh.speed >= 0.f && (c.key[11])) {
											veh.speed += -0.5f / 3.6f;
										}
										if (veh.speed <= 0.f) {
											easing_set(&veh.speed, 0.f, 0.9f, fps);
										}
										hit_f = true;
									}
								}
							}

							if (hit_f) {
								easing_set(&veh.wheel_Leftadd, -veh.speed / 20.f, 0.95f, fps);
								easing_set(&veh.wheel_Rightadd, -veh.speed / 20.f, 0.95f, fps);
							}
							else {
								easing_set(&veh.wheel_Leftadd, 0.f, 0.9f, fps);
								easing_set(&veh.wheel_Rightadd, 0.f, 0.9f, fps);
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
								easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
							}
						}
						veh.pos += veh.add + (veh.mat.zvec() * (-veh.speed / fps));
					}
					//�ǂ̓����蔻��
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
					//�o�[�i�[
					for (auto& be : c.p_burner) {
						veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(c.vehicle.speed / c.vehicle.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
					}
				}
				//�ˌ�
				for (int i = 0; i < veh.Gun_.size(); i++) {
						auto& cg = veh.Gun_[i];
						if (c.key[(i == 0) ? 0 : 1] && cg.loadcnt == 0 && cg.rounds > 0) {
							auto& u = cg.bullet[cg.usebullet];
							++cg.usebullet %= cg.bullet.size();
							//�R�R�����ω�
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
								//set_effect(&c.effcs[ef_fire], veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
								if (u.spec.caliber_a >= 0.037f) {
									//set_effect(&c.effcs_gun[c.gun_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
									//set_pos_effect(&c.effcs_gun[c.gun_effcnt].first, Drawparts->get_effHandle(ef_smoke2));
									c.effcs_gun[c.gun_effcnt].second = &u;
									c.effcs_gun[c.gun_effcnt].cnt = 0;
									++c.gun_effcnt %= c.effcs_gun.size();
								}
							}
							else {
								//set_effect(&c.effcs_missile[c.missile_effcnt].first, veh.obj.frame(cg.gun_info.frame3.first), u.vec);
								//set_pos_effect(&c.effcs_missile[c.missile_effcnt].first, Drawparts->get_effHandle(ef_smoke1));
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
							//set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
						}
					}
					for (auto& t : c.vehicle.use_veh.wheelframe) {
						t.gndsmkeffcs.handle.SetPos(c.vehicle.obj.frame(int(t.frame.first + 1)));
						t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
					}
				}
			}
			//���W���f
			for (auto& c : chara) {
				//�e�֘A
				auto& veh = c.vehicle;
				{
					//�e����
					for (auto& g : veh.Gun_) {
						for (auto& a : g.bullet) {
							float size = 3.f;
							for (int z = 0; z < int(size); z++) {
								if (a.flug) {
									a.repos = a.pos;
									a.pos += a.vec * (a.spec.speed_a / fps / size);
									//����
									{
										bool ground_hit = false;
										VECTOR_ref normal;
										//��ԈȊO�ɓ�����
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
										auto hitplane = c.get_reco(chara, a);		//��s�@�ɂ�����
										//���̌㏈��
										switch (a.spec.type_a) {
										case 0: //AP
											if (!hitplane) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														//set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														//set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
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
														//set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														//set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												a.spec.speed_a -= 5.f / fps / size;
												a.pos += VGet(0.f, a.yadd / size, 0.f);
											}
											break;
										case 2: //�~�T�C��
											if (!hitplane) {
												if (ground_hit) {
													if (a.spec.caliber_a >= 0.020f) {
														//set_effect(&c.effcs[ef_gndhit], a.pos + normal * (0.1f), normal);
													}
													else {
														//set_effect(&c.effcs[ef_gndhit2], a.pos + normal * (0.1f), normal);
													}
													a.flug = false;
												}
											}
											if (a.flug) {
												size_t id = chara.size();
												VECTOR_ref pos;
												float dist = (std::numeric_limits<float>::max)();
												for (auto& t : chara) {
													//�e�֘A
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
													//���f
													auto vec_z = a.vec;
													float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
													float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
													float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
													float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos�擾2D
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

									//����(2�b�������A�X�s�[�h��100�ȉ��A�ђʂ�0�ȉ�)
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
					//�e��
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
			//�؃Z�b�g
			for (auto& l : tree) {
				if (l.fall_flag) {
					l.fall_rad = std::clamp(l.fall_rad + deg2rad(30.f / fps), deg2rad(0.f), deg2rad(90.f));
				}
				l.obj.SetMatrix(MATRIX_ref::RotAxis(l.fall_vec, l.fall_rad) * l.mat * MATRIX_ref::Mtrans(l.pos));
				l.obj_far.SetMatrix(MATRIX_ref::RotAxis(l.fall_vec, l.fall_rad) * l.mat * MATRIX_ref::Mtrans(l.pos));
			}
			{
				//���̍��W�������ŏo��(�z�X�g)
			}
			{
				//�z�X�g����̍��W�������œ���
			}
			//���f���ɔ��f
			for (auto& c : chara) {
				auto& veh = c.vehicle;
				veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
			}
			//�e�p��
			if (shadow_e) {
				Drawparts->Ready_Shadow(campos, draw_in_shadow_sky, VGet(500.f, 500.f, 500.f));
			}
			Drawparts->Move_Player();
			//campos,camvec,camup�̎w��
			{
				auto& veh = mine.vehicle;
				{
					if (ads) {
						campos = veh.obj.frame(veh.use_veh.fps_view.first) + MATRIX_ref::Vtrans(eye_pos_ads, veh.mat);
						campos.y(std::max(campos.y(), 5.f));
						if (useVR_e) {
							camvec = campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
						}
						else {
							if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
								eyevec = veh.mat.zvec();
								camvec = campos - eyevec;
							}
							else {
								camvec = campos - MATRIX_ref::Vtrans(eyevec, veh.mat);
							}
						}
						camup = veh.mat.yvec();
					}
					else {
						camvec = veh.pos + veh.mat.yvec() * (6.f);
						camvec.y(std::max(camvec.y(), 5.f));
						if (useVR_e) {
							campos = camvec + MATRIX_ref::Vtrans(eyevec, veh.mat) * range;
							campos.y(std::max(campos.y(), 0.f));
							if (mapparts->map_col_line_nearest(camvec, &campos)) {
								campos = camvec + (campos - camvec) * (0.9f);
							}
							camup = veh.mat.yvec();
						}
						else {
							if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
								eyevec = (camvec - aimpos).Norm();
								campos = camvec + eyevec * range;
								camup = veh.mat.yvec();

							}
							else {
								campos = camvec + eyevec * range;
								campos.y(std::max(campos.y(), 0.f));
								if (mapparts->map_col_line_nearest(camvec, &campos)) {
									campos = camvec + (campos - camvec) * (0.9f);
								}
								camup = VGet(0.f, 1.f, 0.f);
							}
						}

					}
				}
			}
			{
				float fardist = 1.f;
				float neardist = 1.f;
				if (ads) {
					VECTOR_ref aimingpos = campos + (camvec - campos).Norm() * (3000.f);
					mapparts->map_col_line_nearest(campos, &aimingpos);
					fardist = std::clamp((campos - aimingpos).size(), 300.f, 3000.f);
				}
				else {
					fardist = 6000.f;
				}

				neardist = ads ? (5.f + 25.f * (fardist - 300.f) / (3000.f - 300.f)) : (range_p - 5.f);
				//��ʑ̐[�x�`��
				if (shadow_e) {
					Hostpassparts->dof(&BufScreen, mapparts->sky_draw(campos, camvec, camup, fov / ratio), [&Drawparts, &draw_on_shadow] { Drawparts->Draw_by_Shadow(draw_on_shadow); }, campos, camvec, camup, fov / ratio, fardist, neardist);
				}
				else {
					Hostpassparts->dof(&BufScreen, mapparts->sky_draw(campos, camvec, camup, fov / ratio), draw_on_shadow, campos, camvec, camup, fov / ratio, fardist, neardist);
				}
			}

			GraphHandle::SetDraw_Screen(DX_SCREEN_BACK, campos, camvec, camup, fov / ratio, 0.01f, 5000.0f);
			//�Ə����W�擾
			{
				VECTOR_ref startpos, endpos;
				startpos = mine.vehicle.pos;
				endpos = startpos + mine.vehicle.mat.zvec() * (-1000.f);
				//�n�`
				mapparts->map_col_line_nearest(startpos, &endpos);
				//
				easing_set(&aimpos, endpos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos.get());
			}
			//
			//�`��
			outScreen.SetDraw_Screen();
			{
				//�w�i
				BufScreen.DrawGraph(0, 0, false);
				//�u���[��
				Hostpassparts->bloom(BufScreen, 255);
				//UI
				UIparts->draw(aimposout, mine, ads, fps, lock_on.first, distance, aimposout_lockon, ratio, campos, camvec, camup, eye_pos_ads,useVR_e, false);
				//VR�p�I�v�V����
				if (useVR_e) {
					UIparts->draw_in_vr(*Drawparts->get_device_hand1());
				}
			}
			//VR�Ɉڂ�
			if (useVR_e) {
				GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK));
				outScreen.DrawGraph(0, 0, false);
				for (char i = 0; i < 2; i++) {
					Drawparts->PutEye((ID3D11Texture2D*)GetUseDirect3D11BackBufferTexture2D(), i);
				}
			}
			GraphHandle::SetDraw_Screen(int(DX_SCREEN_BACK));
			{
				outScreen.DrawExtendGraph(0, 0, out_dispx, out_dispy, false);
				//�f�o�b�O
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
		SetMousePoint(dispx / 2, dispy / 2);
		//���
		{
			for (auto& c : chara) {
				/*�G�t�F�N�g*/
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
	return 0; // �\�t�g�̏I��
}
