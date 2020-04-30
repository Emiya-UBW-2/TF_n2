#define NOMINMAX
#include "sub.hpp"

constexpr auto EXTEND = 4;				  /*�u���[���p*/

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker"); /*�ėp�N���X*/
	auto UIparts = std::make_unique<UI>();/*UI*/
	auto Debugparts = std::make_unique<DeBuG>(60); /*�ėp�N���X*/
	//waord
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* ���̂�ێ�����уV�~�����[�g���郏�[���h�I�u�W�F�N�g���\�z*/
	//�J����
	float eye_xrad, eye_yrad;
	VECTOR_ref campos, camvec, camup;
	VECTOR_ref eyevec;
	VECTOR_ref vec_a;
	VECTOR_ref aimpos, aimpos2, aimposout;
	VECTOR_ref aimingpos;
	//�X�N���[���n���h��
	GraphHandle SkyScreen = GraphHandle::Make(dispx, dispy);
	GraphHandle FarScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle MainScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle NearScreen = GraphHandle::Make(dispx, dispy, true);
	GraphHandle BufScreen = GraphHandle::Make(dispx, dispy);
	GraphHandle HighBrightScreen = GraphHandle::Make(dispx, dispy);		     /*�G�t�F�N�g*/
	GraphHandle GaussScreen = GraphHandle::Make(dispx / EXTEND, dispy / EXTEND); /*�G�t�F�N�g*/
	//�n��
	MV1 map, map_col;
	//���
	MV1 carrier;
	MV1 carrier_col;
	VECTOR_ref car_pos = VGet(0.f, 0.f, -1500.f);
	float car_yrad = 0.f;
	VECTOR_ref car_pos_add;
	float car_yrad_add = 0.f;

	//��
	MV1 sky;
	//�C
	VERTEX3D Vertex[6];
	{
		// ����̒��_�̏����Z�b�g
		Vertex[0].pos = VGet(-10000.f, 0.f, -10000.f);
		Vertex[0].dif = GetColorU8(0, 192, 255, 245);
		// �E��̒��_�̏����Z�b�g
		Vertex[1].pos = VGet(10000.f, 0.f, -10000.f);
		Vertex[1].dif = GetColorU8(0, 192, 255, 245);
		// �����̒��_�̏����Z�b�g
		Vertex[2].pos = VGet(-10000.f, 0.f, 10000.f);
		Vertex[2].dif = GetColorU8(0, 192, 255, 245);
		// �E���̒��_�̏����Z�b�g
		Vertex[3].pos = VGet(10000.f, 0.f, 10000.f);
		Vertex[3].dif = GetColorU8(0, 192, 255, 245);
		//
		Vertex[4] = Vertex[2];
		Vertex[5] = Vertex[1];
	}
	//�e��
	MV1 hit_pic;
	MV1::Load("data/hit/model.mv1", &hit_pic);
	//��s�@�G�t�F�N�g
	MV1 plane_effect;
	MV1::Load("data/plane_effect/model.mv1", &plane_effect);
	//����
	bool ads = false;
	bool locktrt = false;
	int Rot = 0;
	float ratio = 1.f;
	float range_p = 1.f;
	float range = 1.f;
	//�e��
	std::vector<hit::ammo> Ammos;
	//�C�e
	{
		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "M728 APDS";
		Ammos.back().type = 0;//ap=0,he=1
		Ammos.back().caliber = 0.105f;
		Ammos.back().penetration = 268.f;
		Ammos.back().speed = 1426.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "M393 HESH";
		Ammos.back().type = 1;//ap=0,he=1
		Ammos.back().caliber = 0.105f;
		Ammos.back().penetration = 145.f;
		Ammos.back().speed = 730.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "NATO 7.62";
		Ammos.back().type = 1;//ap=0,he=1
		Ammos.back().caliber = 0.00762f;
		Ammos.back().penetration = 12.f;
		Ammos.back().speed = 600.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "BR-412D APCBC";
		Ammos.back().type = 0;//ap=0,he=1
		Ammos.back().caliber = 0.100f;
		Ammos.back().penetration = 175.f;
		Ammos.back().speed = 887.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "USSR 12.7";
		Ammos.back().type = 1;//ap=0,he=1
		Ammos.back().caliber = 0.0127f;
		Ammos.back().penetration = 24.f;
		Ammos.back().speed = 850.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "vulcan 20";
		Ammos.back().type = 1;//ap=0,he=1
		Ammos.back().caliber = 0.020f;
		Ammos.back().penetration = 30.f;
		Ammos.back().speed = 800.f;

		Ammos.resize(Ammos.size() + 1);
		Ammos.back().name = "M344A1 HEAT";
		Ammos.back().type = 1;//ap=0,he=1
		Ammos.back().caliber = 0.106f;
		Ammos.back().penetration = 400.f;
		Ammos.back().speed = 503.f;
	}
	//���p
	std::vector<hit::Tanks> tank;
	find_folders("data/tank/*", &tank);
	for (auto& t : tank) {
		MV1::Load("data/tank/" + t.name + "/model.mv1", &t.obj);
		MV1::Load("data/tank/" + t.name + "/col.mv1", &t.col);
		//
		t.down_in_water = 0.f;
		for (int i = 0; i < t.obj.mesh_num(); i++) {
			auto p = t.obj.mesh_maxpos(i).y();
			if (t.down_in_water < p) {
				t.down_in_water = p;
			}
		}
		t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		for (int i = 0; i < t.obj.frame_num(); i++) {
			std::string p = t.obj.frame_name(i);
			if (p.find("�]��", 0) != std::string::npos) {
				t.wheelframe.resize(t.wheelframe.size() + 1);
				t.wheelframe.back().frame.first = i;
				t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);
			}
			else if (p.find("����", 0) != std::string::npos) {
				t.gunframe.resize(t.gunframe.size() + 1);
				t.gunframe.back().frame1.first = i;
				t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
				auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
				if (p2 >= 0) {
					t.gunframe.back().frame1.second -= t.obj.frame(int(p2));//�e�����鎞�����Ƃ�
				}
				if (t.obj.frame_child_num(t.gunframe.back().frame1.first) >= 0) {
					if (t.obj.frame_name(t.gunframe.back().frame1.first + 1).find("�p", 0) != std::string::npos) {
						t.gunframe.back().frame2.first = t.gunframe.back().frame1.first + 1;
						t.gunframe.back().frame2.second = t.obj.frame(t.gunframe.back().frame2.first) - t.obj.frame(t.gunframe.back().frame1.first);
						if (t.obj.frame_child_num(t.gunframe.back().frame1.first) >= 0) {
							t.gunframe.back().frame3.first = t.gunframe.back().frame2.first + 1;
							t.gunframe.back().frame3.second = t.obj.frame(t.gunframe.back().frame3.first) - t.obj.frame(t.gunframe.back().frame2.first);
						}
						else {
							t.gunframe.back().frame3.first = -1;
						}
					}
				}
				else {
					t.gunframe.back().frame2.first = -1;
				}
			}
			else if (p.find("min", 0) != std::string::npos) {
				t.minpos = t.obj.frame(i);
			}
			else if (p.find("max", 0) != std::string::npos) {
				t.maxpos = t.obj.frame(i);
			}
		}
		//2	���㕔0
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (f.frame.second.x() >= 0) {
					t.square[0] = f.frame.first;
					tmp = f.frame.second.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[0] != f.frame.first) {
					if (f.frame.second.x() >= 0) {
						if (tmp < f.frame.second.z()) {
							t.square[0] = f.frame.first;
							tmp = f.frame.second.z();
						}
					}
				}
			}
		}
		//10	���O��1
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (f.frame.second.x() >= 0) {
					t.square[1] = f.frame.first;
					tmp = f.frame.second.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[1] != f.frame.first) {
					if (f.frame.second.x() >= 0) {
						if (tmp > f.frame.second.z()) {
							t.square[1] = f.frame.first;
							tmp = f.frame.second.z();
						}
					}
				}
			}
		}
		//3	�E�㕔2
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (!(f.frame.second.x() >= 0)) {
					t.square[2] = f.frame.first;
					tmp = f.frame.second.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[2] != f.frame.first) {
					if (!(f.frame.second.x() >= 0)) {
						if (tmp < f.frame.second.z()) {
							t.square[2] = f.frame.first;
							tmp = f.frame.second.z();
						}
					}
				}
			}
		}
		//11	�E�O��3
		{
			float tmp = 0.f;
			for (auto& f : t.wheelframe) {
				if (!(f.frame.second.x() >= 0)) {
					t.square[3] = f.frame.first;
					tmp = f.frame.second.z();
					break;
				}
			}
			for (auto& f : t.wheelframe) {
				if (t.square[3] != f.frame.first) {
					if (!(f.frame.second.x() >= 0)) {
						if (tmp > f.frame.second.z()) {
							t.square[3] = f.frame.first;
							tmp = f.frame.second.z();
						}
					}
				}
			}
		}
		//
		for (int i = 0; i < t.col.mesh_num(); i++) {
			std::string p = t.col.material_name(i);
			if (p.find("armer", 0) != std::string::npos) {//���b
				t.armer_mesh.resize(t.armer_mesh.size() + 1);
				t.armer_mesh.back().first = i;
				t.armer_mesh.back().second = std::stof(getright(p.c_str()));//���b�l
			}
			else if (p.find("space", 0) != std::string::npos) {//��ԑ��b
				t.space_mesh.resize(t.space_mesh.size() + 1);
				t.space_mesh.back() = i;
			}
			else {//���W���[��
				t.module_mesh.resize(t.module_mesh.size() + 1);
				t.module_mesh.back() = i;
			}
		}

		{
			int mdata = FileRead_open(("data/tank/" + t.name + "/data.txt").c_str(), FALSE);
			char mstr[64]; /*tank*/
			FileRead_gets(mstr, 64, mdata);
			t.isfloat = (getright(mstr).find("true") != std::string::npos);
			t.flont_speed_limit = getparam_f(mdata);
			t.back_speed_limit = getparam_f(mdata);
			t.body_rad_limit = getparam_f(mdata);
			t.turret_rad_limit = getparam_f(mdata);

			FileRead_gets(mstr, 64, mdata);
			for(auto& g : t.gunframe){
				g.name = getright(mstr);
				g.load_time = getparam_f(mdata);
				while (true) {
					FileRead_gets(mstr, 64, mdata);
					if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
						break;
					}
					g.useammo.resize(g.useammo.size() + 1);
					g.useammo.back() = getright(mstr);
				}
			}
			FileRead_close(mdata);
		}
	}
	//��s�@
	std::vector<hit::Planes> plane;
	find_folders("data/plane/*", &plane);
	for (auto& t : plane) {
		//���f��
		MV1::Load("data/plane/" + t.name + "/model.mv1", &t.obj);
		MV1::Load("data/plane/" + t.name + "/col.mv1", &t.col);
		//���e�X�g
		t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		//�ő�ŏ����擾
		for (int i = 0; i < t.obj.mesh_num(); i++) {
			if (t.maxpos.x() < t.obj.mesh_maxpos(i).x()) { t.maxpos.x(t.obj.mesh_maxpos(i).x()); }
			if (t.maxpos.z() < t.obj.mesh_maxpos(i).z()) { t.maxpos.z(t.obj.mesh_maxpos(i).z()); }
			if (t.minpos.x() > t.obj.mesh_minpos(i).x()) { t.minpos.x(t.obj.mesh_minpos(i).x()); }
			if (t.minpos.z() > t.obj.mesh_minpos(i).z()) { t.minpos.z(t.obj.mesh_minpos(i).z()); }
		}
		//�t���[��
		for (int i = 0; i < t.obj.frame_num(); i++) {
			std::string p = t.obj.frame_name(i);
			if (p.find("�r", 0) != std::string::npos) {
				if (p.find("�n�b�`", 0) == std::string::npos) {
					t.wheelframe.resize(t.wheelframe.size() + 1);
					t.wheelframe.back().frame.first = i;
					t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);
				}
			}
			else if (p.find("�o�[�i�[", 0) != std::string::npos) {
				t.burner.resize(t.burner.size() + 1);
				t.burner.back().first = i;
				t.burner.back().second = t.obj.frame(t.burner.back().first);
			}
			else if (p.find("�t�b�N", 0) != std::string::npos) {
				t.hook.first = i;
				t.hook.second = t.obj.frame(t.hook.first);
			}
		}
		//�f�[�^�擾
		{
			int mdata = FileRead_open(("data/plane/" + t.name + "/data.txt").c_str(), FALSE);
			char mstr[64]; /*tank*/
			t.max_speed_limit = getparam_f(mdata) / 3.6f;
			t.mid_speed_limit = getparam_f(mdata) / 3.6f;
			t.min_speed_limit = getparam_f(mdata) / 3.6f;
			t.rad_limit = getparam_f(mdata);

			FileRead_gets(mstr, 64, mdata);

			if (std::string(mstr).find(("gun_" + std::to_string(t.gunframe.size()))) != std::string::npos) {
				t.gunframe.resize(t.gunframe.size() + 1);
			}
			for (auto& g : t.gunframe) {
				g.name = getright(mstr);
				g.load_time = getparam_f(mdata);
				while (true) {
					FileRead_gets(mstr, 64, mdata);
					if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
						break;
					}
					g.useammo.resize(g.useammo.size() + 1);
					g.useammo.back() = getright(mstr);
				}
			}

			FileRead_close(mdata);
		}
	}
	//�L����
	std::vector<hit::Chara> chara;
	//���
	MV1::Load("data/carrier/model.mv1", &carrier);
	std::vector < hit::frames > wire;
	for (int i = 0; i < carrier.frame_num(); i++) {
		std::string p = carrier.frame_name(i);
		if (p.find("ܲ԰", 0) != std::string::npos) {
			wire.resize(wire.size() + 1);
			wire.back().first = i;
			wire.back().second = carrier.frame(wire.back().first);
		}
	}
	carrier.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
	MV1::Load("data/carrier/col.mv1", &carrier_col);
	carrier_col.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
	carrier_col.SetupCollInfo(32,32,32);
	//map
	MV1::Load("data/map/model.mv1", &map);	//map
	{
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		Drawparts->Set_Shadow(14, VGet(100.f, 100.f, 100.f), map.mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&map, &carrier] {
			map.DrawModel();
			carrier.DrawModel();
		});
	}
	//col
	MV1::Load("data/map/col.mv1", &map_col);//map�R���W����
	{
		VECTOR_ref size;
		for (int i = 0; i < map_col.mesh_num(); i++) {
			VECTOR_ref sizetmp = map_col.mesh_maxpos(i) - map_col.mesh_minpos(i);
			if (size.x() < sizetmp.x()) { size.x(sizetmp.x()); }
			if (size.y() < sizetmp.y()) { size.y(sizetmp.y()); }
			if (size.z() < sizetmp.z()) { size.z(sizetmp.z()); }
		}
		for (int i = 0; i < map_col.mesh_num(); i++) {
			map_col.SetupCollInfo(int(size.x() / 10.f), int(size.y() / 10.f), int(size.z() / 10.f), 0, i);
		}
	}
	MV1::Load("data/sky/model.mv1", &sky);	//��
	//
	{
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 0;
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		chara.back().vehicle[1].use_id = 1;
	//	chara.back().vehicle[1].pos = VGet(0.f, 24.f, -1490.f);
		chara.back().vehicle[1].pos = VGet(0.f, 5.f, 0.f);
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size()+1);
		//���
		chara.back().vehicle[0].use_id = 0;
		chara.back().vehicle[0].pos = VGet(10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		chara.back().vehicle[1].use_id = 1;
		chara.back().vehicle[1].pos = VGet(10.f, 30.f, -1470.f + float(i * 12));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 1;
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		chara.back().vehicle[1].use_id = 1;
		chara.back().vehicle[1].pos = VGet(0.f, 30.f, -1470.f + float(i * 12));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 2;
		chara.back().vehicle[0].pos = VGet(-10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		chara.back().vehicle[1].use_id = 1;
		chara.back().vehicle[1].pos = VGet(-10.f, 30.f, -1470.f + float(i * 12));
	}
	for (int i = 0; i < 10; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 3;
		chara.back().vehicle[0].pos = VGet(-20.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		chara.back().vehicle[1].use_id = 0;
		chara.back().vehicle[1].pos = VGet(-20.f, 30.f, -1470.f + float(i * 12));
	}
	//
	fill_id(chara);
	for (auto& c : chara) {
		//����
		std::fill(c.key.begin(), c.key.end(), false);
		//���
		{
			c.vehicle[0].use_id = std::clamp<size_t>(c.vehicle[0].use_id, 0, tank.size() - 1);
			c.usetank.into(tank[c.vehicle[0].use_id]);//�f�[�^���
			//�]��
			c.wheel_normal = VGet(0.f, 1.f, 0.f);//�]�ւ̖@���Y
			//�C
			{
				c.vehicle[0].Gun_.resize(c.usetank.gunframe.size());
				for (int i = 0; i < c.vehicle[0].Gun_.size(); i++) {
					if (c.vehicle[0].Gun_.size() > i) {
						c.vehicle[0].Gun_[i].gun_info = c.usetank.gunframe[i];
						c.vehicle[0].Gun_[i].loadcnt_all = c.vehicle[0].Gun_[i].gun_info.load_time;
						//�g�p�C�e
						c.vehicle[0].Gun_[i].Spec.resize(c.vehicle[0].Gun_[i].Spec.size() + 1);
						for (auto& pa : Ammos) {
							if (pa.name.find(c.vehicle[0].Gun_[i].gun_info.useammo[0]) != std::string::npos) {
								c.vehicle[0].Gun_[i].Spec.back() = pa;
								break;
							}
						}
						//��C
						for (auto& p : c.vehicle[0].Gun_[i].Ammo) {
							p.color = Drawparts->GetColor(255, 255, 172);
							p.spec = c.vehicle[0].Gun_[i].Spec[0];
						}
					}
				}
			}
			//�R���W����
			for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
				c.usetank.col.SetPosition(VGet(0.f, -100.f, 0.f));
				c.usetank.col.SetupCollInfo(8, 8, 8, -1, i);
			}
			c.useplane.col.SetPosition(VGet(0.f, -100.f, 0.f));
			c.useplane.col.SetupCollInfo(8, 8, 8);

			//���W���[�����Ƃ̓����蔻��
			c.hitssort.resize(c.usetank.col.mesh_num());
			//�����蔻���u���Ă���
			c.hitres.resize(c.usetank.col.mesh_num());
			//�q�b�g�|�C���g
			c.HP.resize(c.usetank.col.mesh_num());
			//�e��
			for (auto& h : c.hit) {
				h.flug = false;
				h.pic = hit_pic.Duplicate();
				h.use = 0;
				h.scale = VGet(1.f, 1.f, 1.f);
				h.pos = VGet(1.f, 1.f, 1.f);
			}
			//
			fill_id(c.effcs);
		}
		//��s�@
		{
			c.vehicle[1].use_id = std::clamp<size_t>(c.vehicle[1].use_id, 0, plane.size() - 1);
			c.useplane.into(plane[c.vehicle[1].use_id]);
			c.changegear = true;
			{
				MV1AttachAnim(c.useplane.obj.get(), 0);//�_�~�[
				c.p_anime_geardown.first = MV1AttachAnim(c.useplane.obj.get(), 1);
				c.p_anime_geardown.second = 1.f;
				MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//��
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					c.p_animes_rudder[i].first = MV1AttachAnim(c.useplane.obj.get(), 2 + i);
					c.p_animes_rudder[i].second = 0.f;
					MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
			}
			//�C
			{
				c.vehicle[1].Gun_.resize(c.useplane.gunframe.size());
				for (int i = 0; i < c.vehicle[1].Gun_.size(); i++) {
					if (c.vehicle[1].Gun_.size() > i) {
						//��C
						c.vehicle[1].Gun_[i].gun_info = c.useplane.gunframe[i];
						c.vehicle[1].Gun_[i].loadcnt_all = c.vehicle[1].Gun_[i].gun_info.load_time;
						c.vehicle[1].Gun_[i].Spec.resize(c.vehicle[1].Gun_[i].Spec.size() + 1);
						for (auto& pa : Ammos) {
							if (pa.name.find(c.vehicle[1].Gun_[i].gun_info.useammo[0]) != std::string::npos) {
								c.vehicle[1].Gun_[i].Spec.back() = pa;
								break;
							}
						}
						for (auto& a : c.vehicle[1].Gun_[i].Ammo) {
							a.spec = c.vehicle[1].Gun_[i].Spec[0];
							a.color = Drawparts->GetColor(255, 255, 172);
						}
					}
				}
			}
			//�G�t�F�N�g
			{
				//plane_effect
				for (auto& be : c.useplane.burner) {
					c.p_burner.resize(c.p_burner.size() + 1);
					c.p_burner.back().frame = be;
					c.p_burner.back().effectobj = plane_effect.Duplicate();
				}
			}
		}
		c.vehicle[1].mat = MGetIdent();
		c.vehicle[1].xradadd_right = 0.f;
		c.vehicle[1].xradadd_left = 0.f;
		c.vehicle[1].yradadd_left = 0.f;
		c.vehicle[1].yradadd_right = 0.f;
		c.vehicle[1].zradadd_right = 0.f;
		c.vehicle[1].zradadd_left = 0.f;
		c.vehicle[1].speed_add = 0.f;
		c.vehicle[1].speed_sub = 0.f;
		c.vehicle[1].speed = 0.f;
		c.vehicle[1].add = VGet(0.f, 0.f, 0.f);

		c.flight = //true;
			false;
	}
	for (auto& c : chara) {
		for (int i = 0; i < c.usetank.obj.material_num(); ++i) {
			MV1SetMaterialSpcColor(c.usetank.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
			MV1SetMaterialSpcPower(c.usetank.obj.get(), i, 50.0f);
		}
	}
	//����set
	for (auto& c : chara) {
		b2PolygonShape dynamicBox; /*�_�C�i�~�b�N�{�f�B�ɕʂ̃{�b�N�X�V�F�C�v���`���܂��B*/
		dynamicBox.SetAsBox((c.usetank.maxpos.x() - c.usetank.minpos.x()) / 2, (c.usetank.maxpos.z() - c.usetank.minpos.z()) / 2, b2Vec2((c.usetank.minpos.x() + c.usetank.maxpos.x()) / 2, (c.usetank.minpos.z() + c.usetank.maxpos.z()) / 2), 0.f);
		b2FixtureDef fixtureDef;				    /*���I�{�f�B�t�B�N�X�`�����`���܂�*/
		fixtureDef.shape = &dynamicBox;				    /**/
		fixtureDef.density = 1.0f;				    /*�{�b�N�X���x���[���ȊO�ɐݒ肷��ƁA���I�ɂȂ�܂�*/
		fixtureDef.friction = 0.3f;				    /*�f�t�H���g�̖��C���I�[�o�[���C�h���܂�*/
		b2BodyDef bodyDef;					    /*�_�C�i�~�b�N�{�f�B���`���܂��B���̈ʒu��ݒ肵�A�{�f�B�t�@�N�g�����Ăяo���܂�*/
		bodyDef.type = b2_dynamicBody;				    /**/
		bodyDef.position.Set(c.vehicle[0].pos.x(), c.vehicle[0].pos.z());	   /**/
		bodyDef.angle = c.vehicle[0].yrad;					    /**/
		c.mine.body.reset(world->CreateBody(&bodyDef));		    /**/
		c.mine.playerfix = c.mine.body->CreateFixture(&fixtureDef); /*�V�F�C�v���{�f�B�ɒǉ����܂�*/
		/* ���̂�ێ�����уV�~�����[�g���郏�[���h�I�u�W�F�N�g���\�z*/
	}
	std::vector<hit::wallPats> wall;	//�ǂ��Z�b�g����
	{
		MV1SetupReferenceMesh(map_col.get(), 0, FALSE);
		MV1_REF_POLYGONLIST p = MV1GetReferenceMesh(map_col.get(), 0, FALSE);

		for (int i = 0; i < p.PolygonNum; i++) {
			if (p.Polygons[i].MaterialIndex == 2) {
				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}

				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}

				wall.resize(wall.size() + 1);
				wall.back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
				wall.back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
				if (b2DistanceSquared(b2Vec2(wall.back().pos[0].x(), wall.back().pos[0].z()), b2Vec2(wall.back().pos[1].x(), wall.back().pos[1].z())) <= 0.005f*0.005f) {
					wall.pop_back();
				}
			}
		}
	}
	for (auto&w : wall){
		// This a chain shape with isolated vertices
		std::array<b2Vec2, 2> vs;
		vs[0].Set(w.pos[0].x(), w.pos[0].z());
		vs[1].Set(w.pos[1].x(), w.pos[1].z());
		b2ChainShape chain;
		chain.CreateChain(&vs[0], 2);
		b2FixtureDef fixtureDef;				    /*���I�{�f�B�t�B�N�X�`�����`���܂�*/
		fixtureDef.shape = &chain;				    /**/
		fixtureDef.density = 1.0f;				    /*�{�b�N�X���x���[���ȊO�ɐݒ肷��ƁA���I�ɂȂ�܂�*/
		fixtureDef.friction = 0.3f;				    /*�f�t�H���g�̖��C���I�[�o�[���C�h���܂�*/
		b2BodyDef bodyDef;					    /*�_�C�i�~�b�N�{�f�B���`���܂��B���̈ʒu��ݒ肵�A�{�f�B�t�@�N�g�����Ăяo���܂�*/
		bodyDef.type = b2_staticBody;				    /**/
		bodyDef.position.Set(0, 0);		   /**/
		bodyDef.angle = 0.f;					    /**/
		w.b2.body.reset(world->CreateBody(&bodyDef));		    /**/
		w.b2.playerfix = w.b2.body->CreateFixture(&fixtureDef); /*�V�F�C�v���{�f�B�ɒǉ����܂�*/
	}
	//�e�֐�(��)
	auto draw_bullets = [&chara, &campos](const unsigned int& color) {
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		for (auto& c : chara) {
			for (auto& gns : c.vehicle) {
				for (auto& g : gns.Gun_) {
					for (auto& a : g.Ammo) {
						if (a.flug) {
							SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255.f * std::clamp<float>((a.spec.speed - 100.f) / (200.f - 100.f), 0.f, 1.f)));
							DrawCapsule3D(a.pos.get(), a.repos.get(), ((a.spec.caliber - 0.00762f)*0.1f + 0.00762f) * ((a.pos - campos).size() / 24.f), 4, a.color, color, TRUE);//7.62mm�p
						}
					}
				}
			}
		}
		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);			// �`��u�����h���[�h��߂�
	};
	//�ˌ��֐�(��)
	auto shoot = [](const bool& model,hit::Chara* c,const VECTOR_ref& pos, const VECTOR_ref& vec, const float& fps) {
		if (model) {//���
			if (!c->flight) {
				for (int i = 0; i < c->vehicle[0].Gun_.size(); i++) {
					auto& cg = c->vehicle[0].Gun_[i];
					if (c->key[(i == 0) ? 0 : 1] && cg.loadcnt == 0) {
						auto& u = cg.Ammo[cg.useammo];
						cg.useammo++;
						cg.useammo %= cg.Ammo.size();
						//�R�R�����ω�
						u.spec = cg.Spec[0];
						u.pos = c->usetank.obj.frame(cg.gun_info.frame2.first);
						u.vec = (c->usetank.obj.frame(cg.gun_info.frame3.first) - c->usetank.obj.frame(cg.gun_info.frame2.first)).Norm();
						//
						cg.loadcnt = cg.loadcnt_all;
						if (i == 0) {
							cg.fired = 1.f;
						}
						u.hit = false;
						u.flug = true;
						u.cnt = 0.f;
						u.yadd = 0.f;
						u.repos = u.pos;
						set_effect(&c->effcs[ef_fire], c->usetank.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber / 0.1f);
					}
					cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
					cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
				}
			}
		}
		else {//��s�@
			if (c->flight) {
				for (auto& cg : c->vehicle[1].Gun_) {
					if (c->key[0] && cg.loadcnt == 0) {
						auto& u = cg.Ammo[cg.useammo];
						cg.useammo++;
						cg.useammo %= cg.Ammo.size();
						//�R�R�����ω�
						u.spec = cg.Spec[0];
						u.pos = pos;
						u.vec = vec;
						//
						cg.loadcnt = cg.loadcnt_all;
						cg.fired = 1.f;
						u.hit = false;
						u.flug = true;
						u.cnt = 0.f;
						u.yadd = 0.f;
						u.repos = u.pos;
						set_effect(&c->effcs[ef_fire], c->vehicle[1].pos, u.vec, u.spec.caliber / 0.1f);
					}
					cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
					cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
				}
			}
		}
	};
	//�K�v�Ȏ��ɓ����蔻������t���b�V������(��)
	auto ref_col = [&chara](const size_t& id, const VECTOR_ref& startpos, const VECTOR_ref& endpos,const float& distance) {
		for (auto& t : chara) {
			if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), t.vehicle[0].pos.get()) > distance)) {
				continue;
			}
			if (!t.vehicle[0].hit_check) {
				t.usetank.col.SetPosition(t.vehicle[0].pos);
				t.usetank.col.SetRotationZYAxis(t.zvec, t.yvec, 0.f);
				for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
					t.usetank.col.RefreshCollInfo(-1, i);
				}
				t.vehicle[0].hit_check = true;
			}
		}
		for (auto& t : chara) {
			if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), t.vehicle[1].pos.get()) > distance)) {
				continue;
			}
			if (!t.vehicle[1].hit_check) {
				t.useplane.col.SetMatrix(t.vehicle[1].mat*t.vehicle[1].pos.Mtrans());
				t.useplane.col.RefreshCollInfo();
				t.vehicle[1].hit_check = true;
			}
		}
	};
	//�J�n
	auto& mine = chara[0];
	ads = false;
	locktrt = false;
	Rot = 0;
	ratio = 1.f;
	range = 1.f;
	range_p = 5.f;
	eye_xrad = 0.f;
	eye_yrad = -mine.vehicle[0].yrad;
	eyevec = VGet(cos(eye_xrad)*sin(eye_yrad), sin(eye_xrad), cos(eye_xrad)*cos(eye_yrad));
	vec_a = eyevec;
	campos = mine.vehicle[0].pos + VGet(0.f, 3.f, 0.f) + eyevec.Scale(range);
	SetMouseDispFlag(FALSE);
	SetMousePoint(dispx / 2, dispy / 2);
	for (auto& c : chara) {
		for (auto& g : c.usetank.wheelframe) {
			g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize = 0.1f;
		}
		for (auto& g : c.useplane.wheelframe) {
			g.gndsmkeffcs.handle = Drawparts->get_gndhitHandle().Play3D();
			g.gndsmksize = 0.1f;
		}
		//c.effcs[ef_smoke2].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
		//c.effcs[ef_smoke3].handle = Drawparts->get_effHandle(ef_smoke2).Play3D();
	}
	while (ProcessMessage() == 0) {
		const auto fps = GetFPS();
		const auto waits = GetNowHiPerformanceCount();
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { break; }
		Debugparts->put_way();
		//�����蔻�胊�t���b�V���t���O
		{
			for (auto& c : chara) {
				if (c.vehicle[0].hit_check) {
					c.usetank.col.SetPosition(VGet(0.f, -100.f, 0.f));
					for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
						c.usetank.col.RefreshCollInfo(-1, i);
					}
					c.vehicle[0].hit_check = false;
				}
				if (c.vehicle[1].hit_check) {
					c.useplane.col.SetMatrix(VECTOR_ref(VGet(0.f, -100.f, 0.f)).Mtrans());
					c.vehicle[1].hit_check = false;
				}
			}
		}
		//���ړ�
		{
			float spd = -(60.f / 3.6f) / fps;
			car_yrad_add = deg2rad(0.f) / fps;
			car_yrad += car_yrad_add;
			car_pos_add = VGet(spd*sin(car_yrad_add), 0.f, spd*cos(car_yrad_add));
			car_pos += car_pos_add;
			carrier_col.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
			carrier.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
			carrier_col.RefreshCollInfo();
		}
		//���_
		{
			//�X�R�[�v
			auto oldads = ads;
			if (!mine.flight) {
				Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 7);
				switch (Rot) {
				case 7:
					ads = true;
					ratio = 40.f;
					range_p = 1.f;
					break;
				case 6:
					ads = true;
					ratio = 20.f;
					range_p = 1.f;
					break;
				case 5:
					ads = true;
					ratio = 10.f;
					range_p = 1.f;
					break;
				case 4:
					ads = true;
					ratio = 5.f;
					range_p = 1.f;
					break;
				case 3:
					ads = true;
					ratio = 1.f;
					range_p = 1.f;
					break;
				case 2:
					ads = false;
					ratio = 1.f;
					range_p = 1.f;
					break;
				case 1:
					ads = false;
					ratio = 1.f;
					range_p = 7.5f;
					break;
				case 0:
					ads = false;
					ratio = 1.f;
					range_p = 15.f;
					break;
				}
			}
			else {
				Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
				switch (Rot) {
				case 2:
					ads = false;
					ratio = 1.f;
					range_p = 1.f;
					break;
				case 1:
					ads = false;
					ratio = 1.f;
					range_p = 15.f;
					break;
				case 0:
					ads = false;
					ratio = 1.f;
					range_p = 30.f;
					break;
				}

			}
			if (ads) {
				range = range_p;
			}
			else {
				range += (range_p - range)*0.05f;
			}
			if (ads != oldads) {
				eyevec = vec_a;
			}
			//�C������
			{
				if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 || mine.flight) {//�C�����b�N
					mine.view_yrad = 0.f;
					mine.view_xrad = 0.f;
				}
				else {
					//�_��
					if (ads) {
						vec_a = eyevec;
					}
					else {
						VECTOR_ref endpos = campos - eyevec.Scale(1000.f);
						//�}�b�v�ɓ���������
						for (int i = 0; i < map_col.mesh_num(); i++) {
							auto hp = map_col.CollCheck_Line(campos, endpos, 0, i);
							if (hp.HitFlag == TRUE) {
								endpos = hp.HitPosition;
							}
						}
						//���p�ɓ���������
						for (auto& c : chara) {
							if (mine.id == c.id || (Segment_Point_MinLength(campos.get(), endpos.get(), c.vehicle[0].pos.get()) > 5.f)) {
								continue;
							}
							for (int i = 0; i < c.usetank.col.mesh_num(); i++) {
								const auto hp = c.usetank.col.CollCheck_Line(campos, endpos, -1, i);
								if (hp.HitFlag == TRUE) {
									endpos = hp.HitPosition;
								}
							}
						}
						vec_a = (mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - endpos).Norm();
					}
					{
						auto vec_z = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
						float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
						float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
						float cost = (vec_a.z() * vec_z.x() - vec_a.x()*vec_z.z()) / (a_hyp * z_hyp);
						mine.view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f;//cos�擾2D
						mine.view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
					}
				}
			}
			//�L�[
			mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);//�ˌ�
			mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0);//�}�V���K��
			mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
			mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
			mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
			mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
			//��s���݂̂̑���
			if (mine.flight) {
				//���[
				mine.key[6] = (CheckHitKey(KEY_INPUT_Q) != 0);
				mine.key[7] = (CheckHitKey(KEY_INPUT_E) != 0);
				//�X���b�g��
				mine.key[8] = (CheckHitKey(KEY_INPUT_R) != 0);
				mine.key[9] = (CheckHitKey(KEY_INPUT_F) != 0);
				//�r
				mine.key[10] = (CheckHitKey(KEY_INPUT_C) != 0);
				mine.key[11] = (CheckHitKey(KEY_INPUT_G) != 0);
			}
			//��s�ւ̈ڍs
			{
				if (!mine.flight) {
					if (CheckHitKey(KEY_INPUT_P) != 0) {
						{
							auto tmp = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[1].mat);
							eye_xrad = atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z()));
							eye_yrad = atan2f(tmp.x(), tmp.z());
						}
						mine.vehicle[0].add = VGet(0.f, 0.f, 0.f);
						mine.flight = true;
					}
				}
			}
			//�}�E�X�Ǝ��_�p�x�������N
			{
				int mousex, mousey;
				GetMousePoint(&mousex, &mousey);
				SetMousePoint(dispx / 2, dispy / 2);
				if (ads) {
					eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f / ratio), deg2rad(-20), deg2rad(10));
					eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f / ratio);
				}
				else {
					eye_xrad = std::clamp(eye_xrad + deg2rad(float(mousey - dispy / 2)*0.1f), deg2rad(-25), deg2rad(89));
					eye_yrad += deg2rad(float(mousex - dispx / 2)*0.1f);
				}
				eyevec = VGet(cos(eye_xrad)*sin(eye_yrad), sin(eye_xrad), cos(eye_xrad)*cos(eye_yrad));
			}
		}
		//���f
		for (auto& c : chara) {
			{
				//�C������
				{
					float limit = deg2rad(c.usetank.turret_rad_limit) / fps;
					c.vehicle[0].Gun_[0].gun_info.yrad += std::clamp(c.view_yrad, -limit, limit);//c.vehicle[0].Gun_[0].gun_info.yrad = std::clamp(c.vehicle[0].Gun_[0].gun_info.yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//�ˊE����
					for (auto& g : c.vehicle[0].Gun_) {
						g.gun_info.xrad = std::clamp(g.gun_info.xrad + std::clamp(c.view_xrad, -limit, limit), deg2rad(-10), deg2rad(20));
					}
				}
				//���f
				for (auto& f : c.vehicle[0].Gun_) {
					c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					if (f.gun_info.frame2.first >= 0) {
						c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
						c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
					}
					if (f.gun_info.frame3.first >= 0) {
						c.usetank.obj.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//���R�C��
						c.usetank.col.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//���R�C��
					}
				}
				//�]��
				for (auto& f : c.usetank.wheelframe) {
					MATRIX_ref tmp;
					c.usetank.obj.frame_reset(f.frame.first);
					auto startpos = c.usetank.obj.frame(f.frame.first);
					auto hp = map_col.CollCheck_Line(startpos + c.yvec.Scale((-f.frame.second.y()) + 2.f), startpos + c.yvec.Scale((-f.frame.second.y()) - 0.3f), 0, 0);
					if (hp.HitFlag == TRUE) {
						tmp = VECTOR_ref(VGet(0.f, hp.HitPosition.y + c.yvec.y()*f.frame.second.y() - startpos.y(), 0.f)).Mtrans();
					}
					else {
						tmp = VECTOR_ref(VGet(0.f, -0.3f, 0.f)).Mtrans();
					}

					c.usetank.obj.SetFrameLocalMatrix(f.frame.first, RotX((f.frame.second.x() >= 0) ? c.wheel_Left : c.wheel_Right)*tmp*f.frame.second.Mtrans());
				}
				c.wheel_normal += (((c.usetank.obj.frame(c.usetank.square[0]) - c.usetank.obj.frame(c.usetank.square[3])) * (c.usetank.obj.frame(c.usetank.square[1]) - c.usetank.obj.frame(c.usetank.square[2]))).Norm() - c.wheel_normal).Scale(0.05f);
				//�ړ�
				auto hp = map_col.CollCheck_Line(c.vehicle[0].pos + VGet(0.f, 2.f, 0.f), c.vehicle[0].pos - VGet(0.f, 0.1f, 0.f), 0, 0);
				auto isfloat = (c.vehicle[0].pos.y() == -c.usetank.down_in_water / 2.f);
				//Z�AY�x�N�g���擾
				{
					c.zvec = VGet(sinf(c.vehicle[0].yrad), 0.f, cosf(c.vehicle[0].yrad));
					if (c.usetank.isfloat && isfloat) {
						c.yvec = VGet(0.f, 1.f, 0.f);
					}
					else {
						c.yvec = c.wheel_normal;
						c.zvec = c.zvec.Vtrans(MGetRotVec2(VGet(0.f, 1.f, 0.f), c.yvec.get()));
					}
				}
				if (hp.HitFlag == TRUE || (c.usetank.isfloat && isfloat)) {
					//�O�i���
					{
						const auto old = c.vehicle[0].speed_add + c.vehicle[0].speed_sub;
						if (c.key[2] && !c.flight) {
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add < (c.usetank.flont_speed_limit / 3.6f)) ? (c.vehicle[0].speed_add + (0.03f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_add;
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub < 0.f) ? (c.vehicle[0].speed_sub + (0.1f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_sub;
						}
						if (c.key[3] && !c.flight) {
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub > (c.usetank.back_speed_limit / 3.6f)) ? (c.vehicle[0].speed_sub - (0.03f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_sub;
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add > 0.f) ? (c.vehicle[0].speed_add - (0.1f / 3.6f)*(420.f / fps)) : c.vehicle[0].speed_add;
						}
						if (!(c.key[2] && !c.flight) && !(c.key[3] && !c.flight)) {
							c.vehicle[0].speed_add = (c.vehicle[0].speed_add > 0.f) ? (c.vehicle[0].speed_add - (0.05f / 3.6f)*(420.f / fps)) : 0.f;
							c.vehicle[0].speed_sub = (c.vehicle[0].speed_sub < 0.f) ? (c.vehicle[0].speed_sub + (0.05f / 3.6f)*(420.f / fps)) : 0.f;
						}
						c.vehicle[0].speed = (old + ((c.vehicle[0].speed_add + c.vehicle[0].speed_sub) - old)*0.1f) / fps;
						c.vehicle[0].add = c.zvec.Scale(-c.vehicle[0].speed);
					}
					//����
					{
						c.vehicle[0].yradadd_left = (c.key[4] && !c.flight) ?
							std::max(c.vehicle[0].yradadd_left - deg2rad(0.5f*(420.f / fps)), deg2rad(-c.usetank.body_rad_limit)) :
							std::min(c.vehicle[0].yradadd_left + deg2rad(0.3f*(420.f / fps)), 0.f);
						c.vehicle[0].yradadd_right = (c.key[5] && !c.flight) ?
							std::min(c.vehicle[0].yradadd_right + deg2rad(0.5f*(420.f / fps)), deg2rad(c.usetank.body_rad_limit)) :
							std::max(c.vehicle[0].yradadd_right - deg2rad(0.3f*(420.f / fps)), 0.f);
						c.vehicle[0].yradadd = (c.vehicle[0].yradadd_left + c.vehicle[0].yradadd_right) / fps;
						c.vehicle[0].yrad += c.vehicle[0].yradadd;
					}
					//����
					{
						auto xradold = c.vehicle[0].xradadd;
						c.vehicle[0].xradadd = deg2rad(-(c.vehicle[0].speed / (420.f / fps)) / ((0.1f / 3.6f) / fps)*30.f);
						c.vehicle[0].xrad += (std::clamp(c.vehicle[0].xradadd - xradold, deg2rad(-15.f), deg2rad(15.f)) - c.vehicle[0].xrad)*(1.f - powf(0.995f, 60.f / fps));
						c.vehicle[0].xrad = std::clamp(c.vehicle[0].xrad, deg2rad(-7.5f), deg2rad(7.5f));
						MATRIX_ref avm = (c.zvec*c.yvec).GetRotAxis(c.vehicle[0].xrad);
						c.yvec = c.yvec.Vtrans(avm);
						c.zvec = c.zvec.Vtrans(avm);
						auto zradold = c.vehicle[0].zradadd;
						c.vehicle[0].zradadd = deg2rad(-c.vehicle[0].yradadd / (deg2rad(5.f) / fps)*30.f);
						c.vehicle[0].zrad += ((c.vehicle[0].zradadd - zradold) - c.vehicle[0].zrad)*0.005f;
						MATRIX_ref bvm = c.zvec.GetRotAxis(c.vehicle[0].zrad);
						c.yvec = c.yvec.Vtrans(bvm);
						c.zvec = c.zvec.Vtrans(bvm);
					}
					if (hp.HitFlag == TRUE) {
						c.vehicle[0].pos.yadd((hp.HitPosition.y - c.vehicle[0].pos.y())*(1.f - powf(0.9f, 60.f / fps)));
					}
				}
				else {
					c.vehicle[0].add.yadd(-9.8f / powf(fps, 2.f));
				}
				//�ˌ�����
				{
					c.xrad_shot += (deg2rad(-c.vehicle[0].Gun_[0].fired*c.vehicle[0].Gun_[0].Spec[0].caliber*50.f)*cos(c.vehicle[0].Gun_[0].gun_info.yrad) - c.xrad_shot)*(1.f - powf(0.85f, 60.f / fps));
					MATRIX_ref avm = (c.zvec*c.yvec).GetRotAxis(c.xrad_shot);
					c.yvec = c.yvec.Vtrans(avm);
					c.zvec = c.zvec.Vtrans(avm);
					c.zrad_shot += (deg2rad(-c.vehicle[0].Gun_[0].fired*c.vehicle[0].Gun_[0].Spec[0].caliber*50.f)*sin(c.vehicle[0].Gun_[0].gun_info.yrad) - c.zrad_shot)*(1.f - powf(0.85f, 60.f / fps));
					MATRIX_ref bvm = c.zvec.GetRotAxis(c.zrad_shot);
					c.yvec = c.yvec.Vtrans(bvm);
					c.zvec = c.zvec.Vtrans(bvm);
				}
				//����
				if (c.usetank.isfloat) {
					c.vehicle[0].pos.y(std::max(c.vehicle[0].pos.y(), -c.usetank.down_in_water / 2.f));
				}
				//�]��
				c.wheel_Left -= c.vehicle[0].speed *2.f - c.vehicle[0].yradadd * 5.f;
				c.wheel_Right -= c.vehicle[0].speed *2.f + c.vehicle[0].yradadd * 5.f;
				//�ˌ�
				shoot(true, &c, c.vehicle[1].pos, VECTOR_ref(VGet(0.f, 0.f, -1.f)).Vtrans(c.vehicle[1].mat), fps);
			}
			if (c.flight){
				//��s�@
				float rad_spec = deg2rad(c.useplane.rad_limit*(c.useplane.mid_speed_limit / c.vehicle[1].speed));
				if (c.vehicle[1].speed < c.useplane.min_speed_limit) {
					rad_spec = deg2rad(c.useplane.rad_limit*(std::clamp(c.vehicle[1].speed, 0.f, c.useplane.min_speed_limit) / c.useplane.min_speed_limit));
				}
				//�s�b�`
				c.vehicle[1].xradadd_right += (((c.key[2]) ? -rad_spec / 4.f : 0.f) - c.vehicle[1].xradadd_right)*(1.f - powf(0.95f, 60.f / fps));
				c.vehicle[1].xradadd_left += (((c.key[3]) ? rad_spec / 4.f : 0.f) - c.vehicle[1].xradadd_left)*(1.f - powf(0.95f, 60.f / fps));
				//���[��
				c.vehicle[1].zradadd_right += (((c.key[4]) ? rad_spec : 0.f) - c.vehicle[1].zradadd_right)*(1.f - powf(0.95f, 60.f / fps));
				c.vehicle[1].zradadd_left += (((c.key[5]) ? -rad_spec : 0.f) - c.vehicle[1].zradadd_left)*(1.f - powf(0.95f, 60.f / fps));
				//���[
				c.vehicle[1].yradadd_left += (((c.key[6])  ? -rad_spec / 8.f : 0.f) - c.vehicle[1].yradadd_left)*(1.f - powf(0.95f, 60.f / fps));
				c.vehicle[1].yradadd_right += (((c.key[7]) ? rad_spec / 8.f : 0.f) - c.vehicle[1].yradadd_right)*(1.f - powf(0.95f, 60.f / fps));
				//�X���b�g��
				c.vehicle[1].speed_add += (((c.key[8] && c.vehicle[1].speed < c.useplane.max_speed_limit) ? ( 0.5f / 3.6f) : 0.f) - c.vehicle[1].speed_add)*(1.f - powf(0.95f, 60.f / fps));
				c.vehicle[1].speed_sub += (((c.key[9] && c.vehicle[1].speed > c.useplane.min_speed_limit) ? 
					(-0.5f / 3.6f) : 
					((c.key[9] && c.vehicle[1].speed > 0.f) ? (-0.2f / 3.6f) : 0.f)
					) - c.vehicle[1].speed_sub)*(1.f - powf(0.95f, 60.f / fps));
				//�X�s�[�h
				c.vehicle[1].speed += c.vehicle[1].speed_add + c.vehicle[1].speed_sub;
				{
					auto tmp = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);
					auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
					c.vehicle[1].speed += ((std::abs(tmp2)>sin(deg2rad(1.0f))) ? tmp2*0.5f : 0.f) / 3.6f;	//����
				}
				//���W�n���f
				{
					auto t_mat = c.vehicle[1].mat;
					c.vehicle[1].mat *= VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].xradadd_right + c.vehicle[1].xradadd_left) / fps);
					c.vehicle[1].mat *= VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].zradadd_right + c.vehicle[1].zradadd_left) / fps);
					c.vehicle[1].mat *= VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(t_mat).GetRotAxis((c.vehicle[1].yradadd_left + c.vehicle[1].yradadd_right) / fps);
				}
				//�r
				c.changegear_cnt = std::min<uint8_t>(c.changegear_cnt + 1, (c.key[10]) ? 2 : 0);
				if (c.changegear_cnt == 1) {
					c.changegear ^= 1;
				}
				c.p_anime_geardown.second += (float(c.changegear) - c.p_anime_geardown.second)*(1.f - powf(0.9f, 60.f / fps));
				MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//��
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					c.p_animes_rudder[i].second += (float(c.key[i+2]) - c.p_animes_rudder[i].second)*(1.f - powf(0.95f, 60.f / fps));
					MV1SetAttachAnimBlendRate(c.useplane.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
				//
				{
					//
					if (c.vehicle[1].speed >= c.useplane.min_speed_limit) {
						c.vehicle[1].add = c.vehicle[1].add.Scale(powf(0.9f, 60.f / fps));
					}
					else {
						c.vehicle[1].add.yadd(-9.8f / powf(fps, 2.f));
					}
					//c.useplane.obj.SetFrameLocalMatrix(c.useplane.hook.first, RotX(deg2rad(60.f*c.p_anime_geardown.second))*c.useplane.hook.second.Mtrans());//���̓t�b�N
					if (c.p_anime_geardown.second >= 0.5f) {
						for (auto& w : c.useplane.wheelframe) {
							w.gndsmksize *= powf(0.9f, 60.f / fps);
							auto tmp = c.useplane.obj.frame(int(w.frame.first + 1));
							{
								auto hp = map_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(c.vehicle[1].mat), tmp, 0, 0);
								if (hp.HitFlag == TRUE) {
									c.vehicle[1].add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);
										auto normal = y_vec + (VECTOR_ref(hp.Normal) - y_vec).Scale(1.f - powf(0.95f, 60.f / fps));

										c.vehicle[1].mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);
									}
									w.gndsmksize = std::clamp(c.vehicle[1].speed*3.6f / 50.f, 0.1f, 1.f);
									if (c.vehicle[1].speed >= 0.f && c.key[11]) {
										c.vehicle[1].speed += -0.5f / 3.6f;
									}
									if (c.vehicle[1].speed <= 0.f) {
										c.vehicle[1].speed *= powf(0.9f, 60.f / fps);
									}
								}
							}
							{
								auto hp = carrier_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(c.vehicle[1].mat), tmp);
								if (hp.HitFlag == TRUE) {
									c.vehicle[1].add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(c.vehicle[1].mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(c.vehicle[1].mat);
										auto normal = y_vec + (VECTOR_ref(hp.Normal) - y_vec).Scale(1.f - powf(0.95f, 60.f / fps));

										c.vehicle[1].mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);

									}
									c.vehicle[1].add += car_pos_add;
									c.vehicle[1].add += ((c.vehicle[1].pos - car_pos).Vtrans(RotY(car_yrad_add)) - (c.vehicle[1].pos - car_pos));
									c.vehicle[1].mat *= RotY(car_yrad_add);
									{
										//���̓��C��-����
										bool to = false;
										for (auto& wi : wire) {
											carrier.frame_reset(wi.first);
											if ((c.useplane.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).size() <= 20.f) {
												VECTOR_ref vec1 = (c.useplane.obj.frame(c.useplane.hook.first + 1) - c.useplane.obj.frame(c.useplane.hook.first)).Norm();
												VECTOR_ref vec2 = (carrier.frame(wi.first) - c.useplane.obj.frame(c.useplane.hook.first)).Norm();
												if (vec1%vec2 >= 0) {
													to = true;
													c.useplane.obj.frame_reset(c.useplane.hook.first);
													c.useplane.obj.SetFrameLocalMatrix(c.useplane.hook.first,
														(
															MATRIX_ref(MGetRotVec2(vec1.get(), vec2.get()))
															)
														*c.useplane.hook.second.Mtrans()
													);

													carrier.SetFrameLocalMatrix(wi.first,
														(
														(c.useplane.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).Mtrans()
															*c.vehicle[1].mat.Inverse()
															)
														*wi.second.Mtrans()
													);
													break;
												}
											}
										}
										if (to && c.vehicle[1].speed > 0.f) {
											c.vehicle[1].speed += -1.75f / 3.6f;
										}
									}
									w.gndsmksize = std::clamp(c.vehicle[1].speed*3.6f / 50.f, 0.1f, 1.f);
									if (c.vehicle[1].speed >= 0.f && c.key[11]) {
										c.vehicle[1].speed += -1.0f / 3.6f;
									}
									if (c.vehicle[1].speed <= 0.f) {
										c.vehicle[1].speed *= powf(0.9f, 60.f / fps);
									}
								}
							}
						}
					}
					else {
						for (auto& w : c.useplane.wheelframe) {
							w.gndsmksize *= powf(0.9f, 60.f / fps);
						}
					}
					c.vehicle[1].pos += c.vehicle[1].add + VECTOR_ref(VGet(0.f, 0.f, -c.vehicle[1].speed / fps)).Vtrans(c.vehicle[1].mat);
				}
				//�ˌ�
				shoot(false,&c, c.vehicle[1].pos, VECTOR_ref(VGet(0.f, 0.f, -1.f)).Vtrans(c.vehicle[1].mat),fps);
				//�ǂ̓����蔻��
				VECTOR_ref p_0 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_1 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_2 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(c.vehicle[1].mat);
				VECTOR_ref p_3 = c.vehicle[1].pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(c.vehicle[1].mat);
				bool hitb = false;
				if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) { hitb = true; }
				if (!hitb) {
					while (true) {
						if (carrier_col.CollCheck_Line(p_0, p_1).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_1, p_2).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_2, p_3).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (carrier_col.CollCheck_Line(p_3, p_0).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						break;
					}
				}
				if (!hitb) {
					for (int i = 0; i < map_col.mesh_num(); i++) {
						if (map_col.CollCheck_Line(p_0, p_1, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_1, p_2, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_2, p_3, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
						if (map_col.CollCheck_Line(p_3, p_0, 0, i).HitFlag == TRUE) {
							hitb = true;
							break;
						}
					}
				}
				if (hitb) {
					c.flight = false;
					c.vehicle[1].pos = VGet(0.f, 5.f, 0.f);
					c.vehicle[1].mat = MGetIdent();
					c.vehicle[1].xradadd_right = 0.f;
					c.vehicle[1].xradadd_left = 0.f;
					c.vehicle[1].yradadd_left = 0.f;
					c.vehicle[1].yradadd_right = 0.f;
					c.vehicle[1].zradadd_right = 0.f;
					c.vehicle[1].zradadd_left = 0.f;
					c.vehicle[1].speed_add = 0.f;
					c.vehicle[1].speed_sub = 0.f;
					c.vehicle[1].speed = 0.f;// c.useplane.min_speed_limit;
					c.vehicle[1].add = VGet(0.f, 0.f, 0.f);
				}
			}
			/*effect*/
			{
				for (auto& t : c.effcs) {
					if (t.id != ef_smoke1 && t.id != ef_smoke2 && t.id != ef_smoke3) {
						set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
					}
				}
				for (int i = 0; i < c.effcs.size(); ++i) {
					if (i != ef_smoke1 && i != ef_smoke2 && i != ef_smoke3) {
						set_pos_effect(&c.effcs[i], Drawparts->get_effHandle(i));
					}
				}
				for (auto& t : c.usetank.wheelframe) {
					t.gndsmksize = 0.1f + std::abs(c.vehicle[0].speed) / ((c.usetank.flont_speed_limit / 3.6f) / fps)*0.6f;
					t.gndsmkeffcs.handle.SetPos(c.usetank.obj.frame(t.frame.first) + c.yvec.Scale(-t.frame.second.y()));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}
				for (auto& t : c.useplane.wheelframe) {
					t.gndsmkeffcs.handle.SetPos(c.useplane.obj.frame(int(t.frame.first + 1)));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}

				//c.effcs[ef_smoke1].handle.SetPos(c.usetank.obj.frame(c.ptr->engineframe));
				//c.effcs[ef_smoke2].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[0]));
				//c.effcs[ef_smoke3].handle.SetPos(c.usetank.obj.frame(c.ptr->smokeframe[1]));
			}
		}
		//�ǔ���
		for (auto& c : chara) {
			c.mine.body->SetLinearVelocity(b2Vec2(c.vehicle[0].add.x(), c.vehicle[0].add.z()));
			c.mine.body->SetAngularVelocity(c.vehicle[0].yradadd);
		}
		world->Step(1.f, 1, 1);
		for (auto& c : chara) {
			c.vehicle[0].yrad = -c.mine.body->GetAngle();
			c.vehicle[0].pos.x(c.mine.body->GetPosition().x);
			c.vehicle[0].pos.z(c.mine.body->GetPosition().y);
			c.vehicle[0].mat = Axis1(c.zvec* c.yvec, c.yvec, c.zvec);
			float spdrec = c.spd;
			c.spd += (std::hypot(c.mine.body->GetLinearVelocity().x, c.mine.body->GetLinearVelocity().y) * ((c.spd > 0) ? 1.f : -1.f) - c.spd)* (1.f - powf(0.99f, 60.f / fps));
			c.vehicle[0].speed = c.spd - spdrec;
		}
		//���f���ɔ��f
		for (auto& c : chara) {
			c.usetank.obj.SetPosition(c.vehicle[0].pos);
			c.usetank.obj.SetRotationZYAxis(c.zvec, c.yvec, 0.f);
			c.useplane.obj.SetMatrix(c.vehicle[1].mat*c.vehicle[1].pos.Mtrans());
			for (auto& be : c.p_burner) {
				be.effectobj.SetMatrix(MATRIX_ref(MGetScale(VGet(1.f, 1.f, std::clamp(c.vehicle[1].speed / c.useplane.mid_speed_limit, 0.1f, 1.f))))*be.frame.second.Mtrans()  * c.vehicle[1].mat *c.vehicle[1].pos.Mtrans());
			}
		}
		//�e����,�e��
		for (auto& c : chara) {
			//�e����
			for (auto& gns : c.vehicle) {
				for (auto& g : gns.Gun_) {
					for (auto& a : g.Ammo) {
						if (a.flug) {
							a.repos = a.pos;
							a.pos += a.vec.Scale(a.spec.speed / fps);
							a.pos.yadd(a.yadd);
							ref_col(c.id, a.pos, a.repos, a.spec.speed / fps);
							//����
							{
								bool ground_hit = false;
								VECTOR_ref normal;
								{
									auto hps = carrier_col.CollCheck_Line(a.repos, a.pos);
									if (hps.HitFlag) {
										a.pos = hps.HitPosition;
										normal = hps.Normal;
										ground_hit = true;
									}
								}
								for (int i = 0; i < map_col.mesh_num(); i++) {
									auto hps = map_col.CollCheck_Line(a.repos, a.pos, 0, i);
									if (hps.HitFlag) {
										a.pos = hps.HitPosition;
										normal = hps.Normal;
										ground_hit = true;
									}
								}
								if (a.spec.type == 0) {//AP
									{
										for (auto& t : chara) {
											if (t.vehicle[1].hit_check) {
												auto hps = t.useplane.col.CollCheck_Line(a.repos, a.pos, -1, -1);
												if (hps.HitFlag) {
													a.flug = false;
													if (a.spec.caliber >= 0.020f) {
														set_effect(&c.effcs[ef_reco], hps.HitPosition, hps.Normal);
													}
													else {
														set_effect(&c.effcs[ef_reco2], hps.HitPosition, hps.Normal);
													}
												}
											}
										}
									}
									if (a.flug) {
										if (!hit::get_reco(c, chara, a)) {
											if (ground_hit) {
												if (a.spec.caliber >= 0.020f) {
													set_effect(&c.effcs[ef_gndhit], a.pos + normal.Scale(0.1f), normal);
												}
												else {
													set_effect(&c.effcs[ef_gndhit2], a.pos + normal.Scale(0.1f), normal);
												}
												if ((a.vec.Norm() % normal) <= cos(deg2rad(60))) {
													a.flug = false;
												}
												else {
													a.vec += normal.Scale((a.vec % normal) * -2.f);
													a.vec = a.vec.Norm();
													a.pos += a.vec.Scale(0.01f);
													a.spec.penetration /= 2.0f;
												}
											}
										}
										a.spec.penetration -= 1.0f / fps;
										a.spec.speed -= 5.f / fps;
									}
								}
								if (a.spec.type == 1) {//HE
									{
										for (auto& t : chara) {
											if (t.vehicle[1].hit_check) {
												auto hps = t.useplane.col.CollCheck_Line(a.repos, a.pos, -1, -1);
												if (hps.HitFlag) {
													a.flug = false;
													if (a.spec.caliber >= 0.020f) {
														set_effect(&c.effcs[ef_reco], hps.HitPosition, hps.Normal);
													}
													else {
														set_effect(&c.effcs[ef_reco2], hps.HitPosition, hps.Normal);
													}
												}
											}
										}
									}
									if (a.flug) {
										if (!hit::get_reco(c, chara, a)) {
											if (ground_hit) {
												if (a.spec.caliber >= 0.020f) {
													set_effect(&c.effcs[ef_gndhit], a.pos + normal.Scale(0.1f), normal);
												}
												else {
													set_effect(&c.effcs[ef_gndhit2], a.pos + normal.Scale(0.1f), normal);
												}
												a.flug = false;
											}
										}
										a.spec.speed -= 5.f / fps;
									}
								}
							}
							a.yadd += -9.8f / powf(fps, 2.f);
							a.cnt += 1.f / fps;
							//����(3�b�������A�X�s�[�h��0�ȉ��A�ђʂ�0�ȉ�)
							if (a.cnt >= 3.f || a.spec.speed < 100.f || a.spec.penetration <= 0.f) {
								a.flug = false;
							}
						}
					}
				}
			}
			//�e��
			for (auto& h : c.hit) {
				if (h.flug) {
					auto yvec = h.yvec.Vtrans(c.vehicle[0].mat);
					auto zvec = h.zvec.Vtrans(c.vehicle[0].mat);

					h.pic.SetScale(h.scale);
					h.pic.SetRotationZYAxis(zvec, yvec, 0.f);
					h.pic.SetPosition(c.vehicle[0].pos + h.pos.Vtrans(c.vehicle[0].mat) + yvec.Scale(0.005f));

					//h.pic.SetMatrix(Axis1((yvec*zvec), yvec, zvec, (c.vehicle[0].pos + h.pos.Vtrans(c.vehicle[0].mat) + yvec.Scale(0.005f))) *MGetScale(h.scale.get()));
				}
			}
		}
		//�e�p��
		Drawparts->Ready_Shadow(campos, [&map, &chara, &ads] {
			for (auto& c : chara) {
				c.usetank.obj.DrawModel();
				c.useplane.obj.DrawModel();
			}
		});
		{
			if (ads) {
				campos = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame1.first) + mine.vehicle[0].Gun_[0].gun_info.frame2.second.Vtrans(RotY(eye_yrad));
				camvec = campos - eyevec;
				camup = mine.yvec;
			}
			else {
				if (!mine.flight) {
					{
						camvec = mine.vehicle[0].pos + VGet(0.f, 3.f, 0.f);
						camvec.y(std::max(camvec.y(), 5.f));
					}
					{
						campos = camvec + eyevec.Scale(range);
						campos.y(std::max(campos.y(), 0.f));
						for (int i = 0; i < map_col.mesh_num(); i++) {
							auto hp = map_col.CollCheck_Line(camvec, campos, 0, i);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}
						{
							auto hp = carrier_col.CollCheck_Line(camvec, campos);
							if (hp.HitFlag == TRUE) {
								campos = camvec + (VECTOR_ref(hp.HitPosition) - camvec).Scale(0.9f);
							}
						}					}
					{
						camup = VGet(0.f, 1.f, 0.f);
						camup = camup.Vtrans((mine.zvec*mine.yvec).GetRotAxis(mine.xrad_shot));
						camup = camup.Vtrans(mine.zvec.GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans((mine.zvec*mine.yvec).GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans(mine.zvec.GetRotAxis(mine.xrad_shot));
					}
				}
				else {
					camvec = mine.vehicle[1].pos + VECTOR_ref(VGet(0.f, 6.f, 0.f)).Vtrans(mine.vehicle[1].mat);
					camvec.y(std::max(camvec.y(), 5.f));

					//campos = camvec + eyevec.Scale(range);
					//campos.y(std::max(campos.y(), 0.f));

					campos = camvec + (aimpos2-camvec).Norm().Scale(-range);
					//aimpos2
					camup = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(mine.vehicle[1].mat);

					//camup = VGet(0.f, 1.f, 0.f);
				}
			}
		}
		float dist = std::clamp((campos - aimingpos).size(), 300.f, 1000.f);
		//�`��
		Drawparts->SetDraw_Screen(SkyScreen);
		{
			//�J����
			SetCameraNearFar(1000.0f, 5000.0f);
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec((campos - camvec).get(), VGet(0, 0, 0), camup.get());
			SetFogEnable(FALSE);
			SetUseLighting(FALSE);
			sky.DrawModel();
			SetUseLighting(TRUE);
			SetFogEnable(TRUE);
		}
		Drawparts->SetDraw_Screen(FarScreen);
		SetCameraNearFar(dist, 6000.0f);
		{
			//�J����
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
			{
				SkyScreen.DrawGraph(0, 0, FALSE);
				Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex] {
					SetFogStartEnd(0.0f, 1000.f);
					SetFogColor(128, 192, 255);
					{
						DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, TRUE);
					}
					SetFogStartEnd(0.0f, 2000.f);
					SetFogColor(128, 128, 128);
					{
						map.DrawModel();
						carrier.DrawModel();
						for (auto& t : chara) {
							if ((!ads && t.id == 0) || t.id != 0) {
								t.usetank.obj.DrawModel();
							}
							//�e��
							for (auto& h : t.hit) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);
								}
							}
						}
						for (auto& t : chara) {
							t.useplane.obj.DrawModel();
						}
						for (auto& t : chara) {
							for (auto& be : t.p_burner) {
								be.effectobj.DrawModel();
							}
						}
					}
				});
				draw_bullets(Drawparts->GetColor(255, 255, 255));
			}
		}
		Drawparts->SetDraw_Screen(MainScreen);
		SetCameraNearFar( (mine.flight) ? (10.f) : ((ads) ? ((3.f + 197.f*(dist - 300.f) / (1000.f - 300.f))) : 3.f) , dist + 50.0f);
		{
			//�J����
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
			{
				Effekseer_Sync3DSetting();
				GraphFilter(FarScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 200);
				FarScreen.DrawGraph(0, 0, false);
				UpdateEffekseer3D(); //2.0ms

				Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex] {
					SetFogStartEnd(0.0f, 1000.f);
					SetFogColor(128, 192, 255);
					{
						DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, TRUE);
					}
					SetFogStartEnd(0.0f, 2000.f);
					SetFogColor(128, 128, 128);
					{
						map.DrawModel();
						carrier.DrawModel();
						for (auto& t : chara) {
							if ((!ads && t.id == 0) || t.id != 0) {
								t.usetank.obj.DrawModel();
							}
							//�e��
							for (auto& h : t.hit) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);
								}
							}
						}
						for (auto& t : chara) {
							t.useplane.obj.DrawModel();
						}
						for (auto& t : chara) {
							for (auto& be : t.p_burner) {
								be.effectobj.DrawModel();
							}
						}
					}
				});
				DrawEffekseer3D();
				draw_bullets(Drawparts->GetColor(255, 255, 255));
			}
		}
		Drawparts->SetDraw_Screen(NearScreen);
		SetCameraNearFar(0.01f, (mine.flight) ? (10.5f) : ((ads) ? ((3.f + 197.f*(dist - 300.f) / (1000.f - 300.f)) + 0.5f) : 3.5f));
		{
			//�J����
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
			{
				Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex] {
					DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, TRUE);
					map.DrawModel();
					carrier.DrawModel();
					for (auto& t : chara) {
						if ((!ads && t.id == 0) || t.id != 0) {
							t.usetank.obj.DrawModel();
						}
						//�e��
						for (auto& h : t.hit) {
							if (h.flug) {
								h.pic.DrawFrame(h.use);
							}
						}
					}
					for (auto& t : chara) {
						t.useplane.obj.DrawModel();
					}
					for (auto& t : chara) {
						for (auto& be : t.p_burner) {
							be.effectobj.DrawModel();
						}
					}
				});
				draw_bullets(Drawparts->GetColor(255, 255, 255));
			}
		}
		Drawparts->SetDraw_Screen(BufScreen);
		{
			MainScreen.DrawGraph(0, 0, false);
			NearScreen.DrawGraph(0, 0, true);
		}
		Drawparts->SetDraw_Screen(DX_SCREEN_BACK);
		{
			SetCameraNearFar(0.01f, 3000.0f);
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
		}
		//�Ə����W�擾
		if(mine.flight){
			VECTOR_ref startpos = mine.vehicle[1].pos;
			VECTOR_ref tmppos = startpos + VECTOR_ref(VGet(0.f, 0.f, -1000.f)).Vtrans(mine.vehicle[1].mat);
			{
				for (int i = 0; i < map_col.mesh_num(); i++) {
					auto hp = map_col.CollCheck_Line(startpos, tmppos, 0, i);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
				auto hp = carrier_col.CollCheck_Line(startpos, tmppos);
				if (hp.HitFlag == TRUE) {
					tmppos = hp.HitPosition;
				}
			}
			ref_col(mine.id, startpos, tmppos, 5.f);
			for (auto& t : chara) {
				if (t.vehicle[0].hit_check) {
					for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
						const auto hp = t.usetank.col.CollCheck_Line(startpos, tmppos, -1, i);
						if (hp.HitFlag == TRUE) {
							tmppos = hp.HitPosition;
						}
					}
				}
				if (t.vehicle[1].hit_check) {
					const auto hp = t.useplane.col.CollCheck_Line(startpos, tmppos, -1, -1);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
			}

			aimpos2 += (tmppos - aimpos2).Scale(1.f - powf(0.9f, 60.f / fps));
			aimposout = ConvWorldPosToScreenPos(aimpos2.get());
		}
		else{
			VECTOR_ref startpos = mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
			VECTOR_ref tmppos = startpos + (mine.usetank.obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - startpos).Norm().Scale(1000.f);
			{
				for (int i = 0; i < map_col.mesh_num(); i++) {
					auto hp = map_col.CollCheck_Line(startpos, tmppos, 0, i);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
				auto hp = carrier_col.CollCheck_Line(startpos, tmppos);
				if (hp.HitFlag == TRUE) {
					tmppos = hp.HitPosition;
				}
			}
			ref_col(mine.id, startpos, tmppos, 5.f);
			for (auto& t : chara) {
				if (t.vehicle[0].hit_check) {
					for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
						const auto hp = t.usetank.col.CollCheck_Line(startpos, tmppos, -1, i);
						if (hp.HitFlag == TRUE) {
							tmppos = hp.HitPosition;
						}
					}
				}
				if (t.vehicle[1].hit_check) {
					const auto hp = t.useplane.col.CollCheck_Line(startpos, tmppos, -1, -1);
					if (hp.HitFlag == TRUE) {
						tmppos = hp.HitPosition;
					}
				}
			}

			aimpos += (tmppos - aimpos).Scale(1.f - powf(0.9f, 60.f / fps));
			aimposout = ConvWorldPosToScreenPos(aimpos.get());
		}
		//�Ə����W�擾
		{
			aimingpos = campos + (camvec- campos).Norm().Scale(1000.f);
			{
				for (int i = 0; i < map_col.mesh_num(); i++) {
					auto hp = map_col.CollCheck_Line(campos, aimingpos, 0, i);
					if (hp.HitFlag == TRUE) {
						aimingpos = hp.HitPosition;
					}
				}
				auto hp = carrier_col.CollCheck_Line(campos, aimingpos);
				if (hp.HitFlag == TRUE) {
					aimingpos = hp.HitPosition;
				}
			}
			ref_col(mine.id, campos, aimingpos, 5.f);
			for (auto& t : chara) {
				if (t.vehicle[0].hit_check) {
					for (int i = 0; i < t.usetank.col.mesh_num(); i++) {
						const auto hp = t.usetank.col.CollCheck_Line(campos, aimingpos, -1, i);
						if (hp.HitFlag == TRUE) {
							aimingpos = hp.HitPosition;
						}
					}
				}
				if (t.vehicle[0].hit_check) {
					const auto hp = t.useplane.col.CollCheck_Line(campos, aimingpos, -1, -1);
					if (hp.HitFlag == TRUE) {
						aimingpos = hp.HitPosition;
					}
				}
			}
		}
		//UI
		{
			BufScreen.DrawGraph(0, 0, false);
			//�u���[��
			{
				GraphFilterBlt(BufScreen.get(), HighBrightScreen.get(), DX_GRAPH_FILTER_TWO_COLOR, 245, GetColor(0, 0, 0), 255, GetColor(128, 128, 128), 255);
				GraphFilterBlt(HighBrightScreen.get(), GaussScreen.get(), DX_GRAPH_FILTER_DOWN_SCALE, EXTEND);
				GraphFilter(GaussScreen.get(), DX_GRAPH_FILTER_GAUSS, 16, 1000);
				SetDrawMode(DX_DRAWMODE_BILINEAR);
				SetDrawBlendMode(DX_BLENDMODE_ADD, 64);
				GaussScreen.DrawExtendGraph(0, 0, dispx, dispy, false);
				GaussScreen.DrawExtendGraph(0, 0, dispx, dispy, false);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
			}

			//for (auto& l : wall) {
			//	DrawLine3D(VGet(l.vehicle[0].pos.x(), 2.f, l.vehicle[0].pos.z()), VGet(l.pos[1].x(), 2.f, l.pos[1].z()), Drawparts->GetColor(255, 0, 0));
			//}
			UIparts->draw(aimposout, mine, ads);
			Debugparts->end_way();
			Debugparts->debug(10, 10, fps, float(GetNowHiPerformanceCount() - waits) / 1000.f);
		}
		Drawparts->Screen_Flip(waits);
	}
	return 0; // �\�t�g�̏I��
}
