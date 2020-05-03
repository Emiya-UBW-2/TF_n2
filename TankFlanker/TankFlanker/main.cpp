#define NOMINMAX
#include "sub.hpp"
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	auto Drawparts = std::make_unique<DXDraw>("TankFlanker", 90.f); /*�ėp�N���X*/
	auto UIparts = std::make_unique<UI>();/*UI*/
	auto Debugparts = std::make_unique<DeBuG>(60); /*�ėp�N���X*/
	auto Hostpassparts = std::make_unique<HostPassEffect>();/*UI*/
	auto world = std::make_unique<b2World>(b2Vec2(0.0f, 0.0f)); /* ���̂�ێ�����уV�~�����[�g���郏�[���h�I�u�W�F�N�g���\�z*/
	//���_
	VECTOR_ref eyevec;
	//�J����
	VECTOR_ref campos, camvec, camup;
	std::array<VECTOR_ref, 2> aimpos;
	VECTOR_ref aimposout;
	//�X�N���[���n���h��
	GraphHandle BufScreen = GraphHandle::Make(dispx, dispy);
	GraphHandle SkyScreen = GraphHandle::Make(dispx, dispy);
	//�n��
	MV1 map, map_col;
	//��
	MV1 tree_model;
	//���
	MV1 carrier;
	MV1 carrier_col;
	VECTOR_ref car_pos = VGet(0.f, 0.f, -1500.f);
	float car_yrad = 0.f;
	VECTOR_ref car_pos_add;
	float car_yrad_add = 0.f;
	std::vector < hit::frames > wire;
	std::vector<hit::frames> catapult;
	//��
	MV1 sky;
	//�C
	VERTEX3D Vertex[6];
	{
		// ����̒��_�̏����Z�b�g
		Vertex[0].pos = VGet(-20000.f, 0.f, -20000.f);
		Vertex[0].dif = GetColorU8(0, 192, 255, 245);
		// �E��̒��_�̏����Z�b�g
		Vertex[1].pos = VGet(20000.f, 0.f, -20000.f);
		Vertex[1].dif = GetColorU8(0, 192, 255, 245);
		// �����̒��_�̏����Z�b�g
		Vertex[2].pos = VGet(-20000.f, 0.f, 20000.f);
		Vertex[2].dif = GetColorU8(0, 192, 255, 245);
		// �E���̒��_�̏����Z�b�g
		Vertex[3].pos = VGet(20000.f, 0.f, 20000.f);
		Vertex[3].dif = GetColorU8(0, 192, 255, 245);
		//
		Vertex[4] = Vertex[2];
		Vertex[5] = Vertex[1];
	}
	//�e��
	MV1 hit_pic;
	//��s�@�G�t�F�N�g
	MV1 plane_effect;
	//����
	bool ads = false;
	bool locktrt = false;
	int Rot = 0;
	float ratio = 1.f;
	float range = 0.f;
	float range_p = 0.f;
	uint8_t change_vehicle = 0;
	//�L����
	std::vector<hit::Chara> chara;
	//�f�[�^//=========================================
	SetUseASyncLoadFlag(TRUE);
	//�e��
	MV1::Load("data/hit/model.mv1", &hit_pic);
	//��s�@�G�t�F�N�g
	MV1::Load("data/plane_effect/model.mv1", &plane_effect);
	//�e��
	std::vector<hit::Ammos> Ammos;
	//���p
	std::vector<hit::Tanks> tank;
	find_folders("data/tank/*", &tank);
	hit::set_tanks_pre(&tank);
	//��s�@
	std::vector<hit::Planes> plane;
	find_folders("data/plane/*", &plane);
	hit::set_planes_pre(&plane);
	SetUseASyncLoadFlag(FALSE);
	UIparts->load_window("�ԗ����f��");
	hit::set_ammos(&Ammos);		//�e��
	hit::set_tanks(&tank);		//���p
	hit::set_planes(&plane);	//��s�@
	//�R�R����J��Ԃ��ǂݍ���//-------------------------------------------------------------------
	chara.clear();
	//�L�����I��
	if (1)
	{
		chara.resize(chara.size() + 1);
		UIparts->select_window(&chara[0], &tank, &plane);
	}
	else {
		chara.back().vehicle[0].use_id = 0;		//���
		chara.back().vehicle[1].use_id = 0;		//��s�@
		chara.back().vehicle[0].cam_sel = 0;
		chara.back().vehicle[1].cam_sel = 0;
	}
	//
	SetUseASyncLoadFlag(TRUE);
	//��
	MV1::Load("data/tree/model.mv1", &tree_model);
	MV1::Load("data/carrier/model.mv1", &carrier);	//���
	MV1::Load("data/map/model.mv1", &map);	//map
	MV1::Load("data/map/col.mv1", &map_col);//map�R���W����
	MV1::Load("data/sky/model.mv1", &sky);	//��
	SetUseASyncLoadFlag(FALSE);
	UIparts->load_window("�}�b�v���f��");
	//���\�[�X�ǂݍ��݌�
	//���
	{
		for (int i = 0; i < carrier.frame_num(); i++) {
			std::string p = carrier.frame_name(i);
			if (p.find("ܲ԰", 0) != std::string::npos) {
				wire.resize(wire.size() + 1);
				wire.back().first = i;
				wire.back().second = carrier.frame(wire.back().first);
			}
			else if (p.find("������", 0) != std::string::npos) {
				catapult.resize(catapult.size() + 1);
				catapult.back().first = i;
				catapult.back().second = carrier.frame(catapult.back().first + 2) - carrier.frame(catapult.back().first);
			}
		}
		carrier.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
		MV1::Load("data/carrier/col.mv1", &carrier_col);
		carrier_col.SetMatrix(RotY(car_yrad)*car_pos.Mtrans());
		carrier_col.SetupCollInfo(32, 32, 32);
	}
	//map
	{
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		Drawparts->Set_Shadow(14, VGet(100.f, 100.f, 100.f), map.mesh_minpos(0), VGet(0.0f, -0.5f, 0.5f), [&map, &carrier] {
			map.DrawModel();
			carrier.DrawModel();
		});
	}
	//col
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
	//
	{
		//���
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		//��s�@
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
	}
	for (int i = 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 0;
		chara.back().vehicle[0].pos = VGet(10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		chara.back().vehicle[0].cam_sel = GetRand(5);
		//��s�@
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].cam_sel = GetRand(5);
	}
	for (int i = 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 1;
		chara.back().vehicle[0].pos = VGet(0.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		chara.back().vehicle[0].cam_sel = GetRand(5);
		//��s�@
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].cam_sel = GetRand(5);
	}
	for (int i = 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 2;
		chara.back().vehicle[0].pos = VGet(-10.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		chara.back().vehicle[0].cam_sel = GetRand(5);
		//��s�@
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].cam_sel = GetRand(5);
	}
	for (int i = 0; i < 6; i++) {
		chara.resize(chara.size() + 1);
		//���
		chara.back().vehicle[0].use_id = 3;
		chara.back().vehicle[0].pos = VGet(-20.f, 1.81f, -2.48f + float(i * 14) - 300.f);
		chara.back().vehicle[0].yrad = deg2rad(270.f);
		chara.back().vehicle[0].cam_sel = GetRand(5);
		//��s�@
		chara.back().vehicle[1].use_id = 0;
		auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
		chara.back().vehicle[1].pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
		chara.back().vehicle[1].mat = RotY(atan2f(-pp.x(), -pp.z()));
		chara.back().vehicle[1].cam_sel = GetRand(5);
	}
	fill_id(chara);
	for (auto& c : chara) {
		//����
		std::fill(c.key.begin(), c.key.end(), false);
		//�G�t�F�N�g
		fill_id(c.effcs);
		//���
		{
			auto& veh = c.vehicle[0];

			veh.use_id = std::clamp<size_t>(veh.use_id, 0, tank.size() - 1);
			c.usetank.into(tank[veh.use_id]);//�f�[�^���
			veh.obj = tank[veh.use_id].obj.Duplicate();
			veh.col = tank[veh.use_id].col.Duplicate();
			//�]��
			c.wheel_normal = VGet(0.f, 1.f, 0.f);//�]�ւ̖@���Y
			//�C
			{
				veh.Gun_.resize(c.usetank.gunframe.size());
				for (int i = 0; i < veh.Gun_.size(); i++) {
					auto& g = veh.Gun_[i];
					g.gun_info = c.usetank.gunframe[i];
					g.rounds = g.gun_info.rounds;
					//�g�p�C�e
					g.Spec.resize(g.Spec.size() + 1);
					for (auto& pa : Ammos) {
						if (pa.name.find(g.gun_info.useammo[0]) != std::string::npos) {
							g.Spec.back() = pa;
							break;
						}
					}
					for (auto& p : g.bullet) {
						p.color = Drawparts->GetColor(255, 255, 172);
						p.spec = g.Spec[0];
					}
				}
			}
			//�R���W����
			for (int i = 0; i < veh.col.mesh_num(); i++) {
				veh.col.SetPosition(VGet(0.f, -100.f, 0.f));
				veh.col.SetupCollInfo(8, 8, 8, -1, i);
			}
			//���W���[�����Ƃ̓����蔻��
			c.hitssort.resize(veh.col.mesh_num());
			//�����蔻���u���Ă���
			veh.hitres.resize(veh.col.mesh_num());
			//�q�b�g�|�C���g
			veh.HP = c.usetank.HP;
			veh.HP_m.resize(veh.col.mesh_num());
			for (auto& h : veh.HP_m) {
				h = 100;
			}
			//�e��
			for (auto& h : veh.hit) {
				h.flug = false;
				h.pic = hit_pic.Duplicate();
				h.use = 0;
				h.scale = VGet(1.f, 1.f, 1.f);
				h.pos = VGet(1.f, 1.f, 1.f);
			}
			//����
			if (c.usetank.camog.size() > 0) {
				veh.cam_sel %= c.usetank.camog.size();
				MV1SetTextureGraphHandle(veh.obj.get(), c.usetank.camo_tex, c.usetank.camog[veh.cam_sel], FALSE);
			}
		}
		//��s�@
		{
			auto& veh = c.vehicle[1];

			veh.use_id = std::clamp<size_t>(veh.use_id, 0, plane.size() - 1);
			c.useplane.into(plane[veh.use_id]);
			veh.obj = plane[veh.use_id].obj.Duplicate();
			veh.col = plane[veh.use_id].col.Duplicate();

			c.changegear = true;
			c.landing = false;
			{
				MV1AttachAnim(veh.obj.get(), 0);//�_�~�[
				c.p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
				c.p_anime_geardown.second = 1.f;
				MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//��
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					c.p_animes_rudder[i].first = MV1AttachAnim(veh.obj.get(), 2 + i);
					c.p_animes_rudder[i].second = 0.f;
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
			}
			//�C
			{
				veh.Gun_.resize(c.useplane.gunframe.size());
				for (int i = 0; i < veh.Gun_.size(); i++) {
					auto& g = veh.Gun_[i];
					//��C
					g.gun_info = c.useplane.gunframe[i];
					g.rounds = g.gun_info.rounds;

					g.Spec.resize(g.Spec.size() + 1);
					for (auto& pa : Ammos) {
						if (pa.name.find(g.gun_info.useammo[0]) != std::string::npos) {
							g.Spec.back() = pa;
							break;
						}
					}
					for (auto& a : g.bullet) {
						a.color = Drawparts->GetColor(255, 255, 172);
						a.spec = g.Spec[0];
					}
				}

			}
			//�����蔻���u���Ă���
			veh.hitres.resize(veh.col.mesh_num());
			//�q�b�g�|�C���g
			veh.HP = c.useplane.HP;
			veh.HP_m.resize(veh.col.mesh_num());
			for (auto& h : veh.HP_m) {
				h = 100;
			}
			//�R���W����
			for (int i = 0; i < veh.col.mesh_num(); i++) {
				veh.col.SetPosition(VGet(0.f, -100.f, 0.f));
				veh.col.SetupCollInfo(8, 8, 8, -1, i);
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
			//�e��
			for (auto& h : veh.hit) {
				h.flug = false;
				h.pic = hit_pic.Duplicate();
				h.use = 0;
				h.scale = VGet(1.f, 1.f, 1.f);
				h.pos = VGet(1.f, 1.f, 1.f);
			}
			//
			carrier.SetFrameLocalMatrix(catapult[0].first + 2, RotX(deg2rad(-75))*catapult[0].second.Mtrans());
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

			//����
			if (c.useplane.camog.size() > 0) {
				veh.cam_sel %= c.useplane.camog.size();
				MV1SetTextureGraphHandle(veh.obj.get(), c.useplane.camo_tex, c.useplane.camog[veh.cam_sel], FALSE);
			}
		}
		c.mode = 0;
	}
	for (auto& c : chara) {
		for (auto& veh : c.vehicle) {
			for (int i = 0; i < veh.obj.material_num(); ++i) {
				MV1SetMaterialSpcColor(veh.obj.get(), i, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
				MV1SetMaterialSpcPower(veh.obj.get(), i, 50.0f);
			}
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
	//�ǂ��Z�b�g����
	std::vector<hit::wallPats> wall;
	std::vector<hit::treePats> tree;
	{
		MV1SetupReferenceMesh(map_col.get(), 0, FALSE);
		MV1_REF_POLYGONLIST p = MV1GetReferenceMesh(map_col.get(), 0, FALSE);

		for (int i = 0; i < p.PolygonNum; i++) {
			if (p.Polygons[i].MaterialIndex == 2) {
				//��
				{
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
			else if (p.Polygons[i].MaterialIndex == 3) {
				//��
				{
					tree.resize(tree.size() + 1);
					tree.back().pos = VECTOR_ref(p.Vertexs[p.Polygons[i].VIndex[0]].Position) + p.Vertexs[p.Polygons[i].VIndex[1]].Position + p.Vertexs[p.Polygons[i].VIndex[2]].Position;
					tree.back().pos = tree.back().pos.Scale(1.f / 3.f);
					tree.back().scale = VGet(15.f / 10.f, 15.f / 10.f, 15.f / 10.f);
					tree.back().obj = tree_model.Duplicate();
					tree.back().obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				}
			}
		}
	}
	for (auto&w : wall) {
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
					for (auto& a : g.bullet) {
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
	auto shoot = [](hit::Chara* c, const float& fps) {
		auto& veh = c->vehicle[c->mode];
		for (int i = 0; i < veh.Gun_.size(); i++) {
			auto& cg = veh.Gun_[i];
			if (c->key[(i == 0) ? 0 : 1] && cg.loadcnt == 0) {
				auto& u = cg.bullet[cg.usebullet];
				++cg.usebullet %= cg.bullet.size();
				//�R�R�����ω�
				u.spec = cg.Spec[0];
				u.spec.speed *= float(75 + GetRand(50)) / 100.f;
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
				set_effect(&c->effcs[ef_fire], veh.obj.frame(cg.gun_info.frame3.first), u.vec, u.spec.caliber / 0.1f);
			}
			cg.loadcnt = std::max(cg.loadcnt - 1.f / fps, 0.f);
			cg.fired = std::max(cg.fired - 1.f / fps, 0.f);
		}
	};
	//�K�v�Ȏ��ɓ����蔻������t���b�V������(��)
	auto ref_col = [&chara](const size_t& id, const VECTOR_ref& startpos, const VECTOR_ref& endpos, const float& distance) {
		for (auto& t : chara) {
			for (auto& veh : t.vehicle) {
				if (id == t.id || (Segment_Point_MinLength(startpos.get(), endpos.get(), veh.pos.get()) > distance)) {
					continue;
				}
				if (!veh.hit_check) {
					veh.col.SetMatrix(veh.mat*veh.pos.Mtrans());
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = true;
				}
			}
		}
	};
	//�ʐM
	{
		//IPDATA IPData;
		//IPData.d1 = 127;
		//IPData.d2 = 0;
		//IPData.d3 = 0;
		//IPData.d4 = 0;
		//int port = 19190;
		//ConnectNetWork(IPData, port);
	}
	//�J�n
	auto& mine = chara[0];
	ads = false;
	locktrt = false;
	Rot = 0;
	ratio = 1.f;
	range_p = 5.f;
	eyevec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[mine.mode].mat);
	campos = mine.vehicle[mine.mode].pos + VGet(0.f, 3.f, 0.f) + eyevec.Scale(range);
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
	float totals = 0.f;
	while (ProcessMessage() == 0) {
		const auto fps = GetFPS();
		const auto waits = GetNowHiPerformanceCount();
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { break; }
		Debugparts->put_way();
		//�����蔻�胊�t���b�V���t���O
		for (auto& c : chara) {
			for (auto& veh : c.vehicle) {
				if (veh.hit_check) {
					veh.col.SetMatrix(VECTOR_ref(VGet(0.f, -100.f, 0.f)).Mtrans());
					for (int i = 0; i < veh.col.mesh_num(); i++) {
						veh.col.RefreshCollInfo(-1, i);
					}
					veh.hit_check = false;
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
			{
				switch (mine.mode) {
				case 0:
					Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 7);
					switch (Rot) {
					case 2:
						range_p = 1.f;
						break;
					case 1:
						range_p = 7.5f;
						break;
					case 0:
						range_p = 15.f;
						break;
					}
					ratio = 1.f;
					for (int i = 3; i < Rot; i++) {
						ratio *= 5.f;
					}
					break;
				case 1:
					Rot = std::clamp(Rot + GetMouseWheelRotVol(), 0, 2);
					switch (Rot) {
					case 2:
						range_p = 1.f;
						break;
					case 1:
						range_p = 15.f;
						break;
					case 0:
						range_p = 30.f;
						break;
					}
					ratio = 1.f;
					break;
				default:
					break;
				}
				if (ads != (Rot >= 3)) {
					ads = (Rot >= 3);
				}
				if (ads) {
					range_p = 1.f;
				}
				easing_set(&range, range_p, (ads) ? 0.f : 0.95f, fps);
			}
			//�C������
			{
				if ((GetMouseInput() & MOUSE_INPUT_RIGHT) != 0 || mine.mode != 0) {//�C�����b�N
					mine.view_yrad = 0.f;
					mine.view_xrad = 0.f;
				}
				else {
					VECTOR_ref vec_a;
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
							for (int i = 0; i < c.vehicle[0].col.mesh_num(); i++) {
								const auto hp = c.vehicle[0].col.CollCheck_Line(campos, endpos, -1, i);
								if (hp.HitFlag == TRUE) {
									endpos = hp.HitPosition;
								}
							}
						}
						vec_a = (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first) - endpos).Norm();
					}
					{
						auto vec_z = mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
						float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
						float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
						float cost = (vec_a.z() * vec_z.x() - vec_a.x()*vec_z.z()) / (a_hyp * z_hyp);
						mine.view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f;//cos�擾2D
						mine.view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
					}
				}
			}
			//�L�[
			{
				mine.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);//�ˌ�
				mine.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0);//�}�V���K��
				mine.key[2] = (CheckHitKey(KEY_INPUT_W) != 0);
				mine.key[3] = (CheckHitKey(KEY_INPUT_S) != 0);
				mine.key[4] = (CheckHitKey(KEY_INPUT_D) != 0);
				mine.key[5] = (CheckHitKey(KEY_INPUT_A) != 0);
				//��s���݂̂̑���
				if (mine.mode == 1) {
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
				//��s�ւ̈ڍs
				change_vehicle = std::clamp<uint8_t>(change_vehicle + 1, 0, int((CheckHitKey(KEY_INPUT_P) != 0) ? 2 : 0));
				for (int i = 0; i < mine.vehicle.size(); i++) {
					if (mine.mode != i) {
						if (change_vehicle == 1) {
							mine.mode = i;
							eyevec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[mine.mode].mat);
							mine.vehicle[mine.mode].add = VGet(0.f, 0.f, 0.f);
							change_vehicle = 2;
						}
					}
				}
			}
			//�}�E�X�Ǝ��_�p�x�������N
			{
				int mousex, mousey;
				GetMousePoint(&mousex, &mousey);
				SetMousePoint(dispx / 2, dispy / 2);
				{
					if (ads) {
						float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2)*0.1f / ratio);
						float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2)*0.1f / ratio);
						x = std::clamp(x, deg2rad(-20), deg2rad(10));
						eyevec = VGet(cos(x)*sin(y), sin(x), cos(x)*cos(y));
					}
					else {
						float y = atan2f(eyevec.x(), eyevec.z()) + deg2rad(float(mousex - dispx / 2)*0.1f);
						float x = atan2f(eyevec.y(), std::hypotf(eyevec.x(), eyevec.z())) + deg2rad(float(mousey - dispy / 2)*0.1f);
						x = std::clamp(x, deg2rad(-25), deg2rad(89));
						eyevec = VGet(cos(x)*sin(y), sin(x), cos(x)*cos(y));
					}
				}
			}
		}
		{
			//���̃L�[���͂������Ŏ擾(�z�X�g)
		}
		//���f
		for (auto& c : chara) {
			//��ԉ��Z
			{
				auto& veh = c.vehicle[0];
				VECTOR_ref yvec, zvec;
				//�C������
				{
					float limit = deg2rad(c.usetank.turret_rad_limit) / fps;
					veh.Gun_[0].gun_info.yrad += std::clamp(c.view_yrad, -limit, limit);//veh.Gun_[0].gun_info.yrad = std::clamp(veh.Gun_[0].gun_info.yrad + std::clamp(view_yrad / 5.f, -limit, limit),deg2rad(-30.0)+yrad,deg2rad(30.0)+yrad);//�ˊE����
					for (auto& g : veh.Gun_) {
						g.gun_info.xrad = std::clamp(g.gun_info.xrad + std::clamp(c.view_xrad, -limit, limit), deg2rad(-10), deg2rad(20));
					}
				}
				//���f
				for (auto& f : veh.Gun_) {
					veh.obj.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					veh.col.SetFrameLocalMatrix(f.gun_info.frame1.first, RotY(f.gun_info.yrad)*f.gun_info.frame1.second.Mtrans());
					if (f.gun_info.frame2.first >= 0) {
						veh.obj.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
						veh.col.SetFrameLocalMatrix(f.gun_info.frame2.first, RotX(f.gun_info.xrad)*f.gun_info.frame2.second.Mtrans());
					}
					if (f.gun_info.frame3.first >= 0) {
						veh.obj.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//���R�C��
						veh.col.SetFrameLocalMatrix(f.gun_info.frame3.first, VECTOR_ref(VGet(0.f, 0.f, f.fired*0.5f)).Mtrans()*f.gun_info.frame3.second.Mtrans());//���R�C��
					}
				}
				//�]��
				{
					auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(veh.mat);
					for (auto& f : c.usetank.wheelframe) {
						MATRIX_ref tmp;
						veh.obj.frame_reset(f.frame.first);
						auto startpos = veh.obj.frame(f.frame.first);
						auto hp = map_col.CollCheck_Line(startpos + y_vec.Scale((-f.frame.second.y()) + 2.f), startpos + y_vec.Scale((-f.frame.second.y()) - 0.3f), 0, 0);
						if (hp.HitFlag == TRUE) {
							tmp = VECTOR_ref(VGet(0.f, hp.HitPosition.y + y_vec.y()*f.frame.second.y() - startpos.y(), 0.f)).Mtrans();
						}
						else {
							tmp = VECTOR_ref(VGet(0.f, -0.3f, 0.f)).Mtrans();
						}

						veh.obj.SetFrameLocalMatrix(f.frame.first, RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right)*tmp*f.frame.second.Mtrans());
					}
					for (auto& f : c.usetank.wheelframe_nospring) {
						veh.obj.SetFrameLocalMatrix(f.frame.first, RotX((f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right)*f.frame.second.Mtrans());
					}
				}
				easing_set(
					&c.wheel_normal,
					((veh.obj.frame(c.usetank.square[0]) - veh.obj.frame(c.usetank.square[3])) * (veh.obj.frame(c.usetank.square[1]) - veh.obj.frame(c.usetank.square[2]))).Norm(),
					0.95f, fps);
				//�ړ�
				auto hp = map_col.CollCheck_Line(veh.pos + VGet(0.f, 2.f, 0.f), veh.pos - VGet(0.f, 0.1f, 0.f), 0, 0);
				auto isfloat = (veh.pos.y() == -c.usetank.down_in_water);
				//Z�AY�x�N�g���擾
				{
					zvec = VGet(sinf(veh.yrad), 0.f, cosf(veh.yrad));
					if (c.usetank.isfloat && isfloat) {
						yvec = VGet(0.f, 1.f, 0.f);
					}
					else {
						yvec = c.wheel_normal;
						zvec = zvec.Vtrans(MGetRotVec2(VGet(0.f, 1.f, 0.f), yvec.get()));
					}
				}
				if (hp.HitFlag == TRUE || (c.usetank.isfloat && isfloat)) {
					//�O�i���
					{
						const auto old = veh.speed_add + veh.speed_sub;
						if (c.key[2] && !c.mode == 1) {
							veh.speed_add = (veh.speed_add < (c.usetank.flont_speed_limit / 3.6f)) ? (veh.speed_add + (0.03f / 3.6f)*(420.f / fps)) : veh.speed_add;
							veh.speed_sub = (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.1f / 3.6f)*(420.f / fps)) : veh.speed_sub;
						}
						if (c.key[3] && !c.mode == 1) {
							veh.speed_sub = (veh.speed_sub > (c.usetank.back_speed_limit / 3.6f)) ? (veh.speed_sub - (0.03f / 3.6f)*(420.f / fps)) : veh.speed_sub;
							veh.speed_add = (veh.speed_add > 0.f) ? (veh.speed_add - (0.1f / 3.6f)*(420.f / fps)) : veh.speed_add;
						}
						if (!(c.key[2] && !c.mode == 1) && !(c.key[3] && !c.mode == 1)) {
							veh.speed_add = (veh.speed_add > 0.f) ? (veh.speed_add - (0.05f / 3.6f)*(420.f / fps)) : 0.f;
							veh.speed_sub = (veh.speed_sub < 0.f) ? (veh.speed_sub + (0.05f / 3.6f)*(420.f / fps)) : 0.f;
						}
						veh.speed = (old + ((veh.speed_add + veh.speed_sub) - old)*0.1f) / fps;
						veh.add = zvec.Scale(-veh.speed);
					}
					//����
					{
						veh.yradadd_left = (c.key[4] && !c.mode == 1) ?
							std::max(veh.yradadd_left - deg2rad(0.5f*(420.f / fps)), deg2rad(-c.usetank.body_rad_limit)) :
							std::min(veh.yradadd_left + deg2rad(0.3f*(420.f / fps)), 0.f);
						veh.yradadd_right = (c.key[5] && !c.mode == 1) ?
							std::min(veh.yradadd_right + deg2rad(0.5f*(420.f / fps)), deg2rad(c.usetank.body_rad_limit)) :
							std::max(veh.yradadd_right - deg2rad(0.3f*(420.f / fps)), 0.f);
						veh.yradadd = (veh.yradadd_left + veh.yradadd_right) / fps;
						veh.yrad += veh.yradadd;
					}
					//����
					{
						const auto xradold = veh.xradadd;
						veh.xradadd = deg2rad(-(veh.speed / (420.f / fps)) / ((0.1f / 3.6f) / fps)*30.f);
						easing_set(&veh.xrad, std::clamp(veh.xradadd - xradold, deg2rad(-15.f), deg2rad(15.f)), 0.995f, fps);
						veh.xrad = std::clamp(veh.xrad, deg2rad(-7.5f), deg2rad(7.5f));
						MATRIX_ref avm = (zvec*yvec).GetRotAxis(veh.xrad);
						yvec = yvec.Vtrans(avm);
						zvec = zvec.Vtrans(avm);
						auto zradold = veh.zradadd;
						veh.zradadd = deg2rad(-veh.yradadd / (deg2rad(5.f) / fps)*30.f);
						veh.zrad += ((veh.zradadd - zradold) - veh.zrad)*0.005f;
						MATRIX_ref bvm = zvec.GetRotAxis(veh.zrad);
						yvec = yvec.Vtrans(bvm);
						zvec = zvec.Vtrans(bvm);
					}
					if (hp.HitFlag == TRUE) {
						auto yp = veh.pos.y();
						easing_set(&yp, hp.HitPosition.y, 0.9f, fps);
						veh.pos.y(yp);
					}
				}
				else {
					veh.add.yadd(-9.8f / powf(fps, 2.f));
				}
				//�ˌ�����
				{
					easing_set(&c.xrad_shot, deg2rad(-veh.Gun_[0].fired*veh.Gun_[0].Spec[0].caliber*50.f)*cos(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
					MATRIX_ref avm = (zvec*yvec).GetRotAxis(c.xrad_shot);
					easing_set(&c.zrad_shot, deg2rad(-veh.Gun_[0].fired*veh.Gun_[0].Spec[0].caliber*50.f)*sin(veh.Gun_[0].gun_info.yrad), 0.85f, fps);
					MATRIX_ref bvm = zvec.GetRotAxis(c.zrad_shot);

					yvec = yvec.Vtrans(avm*bvm);
					zvec = zvec.Vtrans(avm*bvm);
				}
				//�s��
				veh.mat = Axis1(yvec*zvec, yvec, zvec);
				//����
				if (c.usetank.isfloat) {
					veh.pos.y(std::max(veh.pos.y(), -c.usetank.down_in_water));
				}
				//�]��
				veh.wheel_Left -= veh.speed *2.f - veh.yradadd * 5.f;
				veh.wheel_Right -= veh.speed *2.f + veh.yradadd * 5.f;
				//�ˌ�
				shoot(&c, fps);
				//
			}
			{
				auto& veh = c.vehicle[1];
				//��s�@
				float rad_spec = deg2rad(c.useplane.body_rad_limit*(c.useplane.mid_speed_limit / veh.speed));
				if (veh.speed < c.useplane.min_speed_limit) {
					rad_spec = deg2rad(c.useplane.body_rad_limit*(std::clamp(veh.speed, 0.f, c.useplane.min_speed_limit) / c.useplane.min_speed_limit));
				}
				//�s�b�`
				easing_set(&veh.xradadd_right, ((c.key[2] && c.mode == 1) ? -(c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
				easing_set(&veh.xradadd_left, ((c.key[3] && c.mode == 1) ? (c.key[12] ? rad_spec / 12.f : rad_spec / 4.f) : 0.f), 0.95f, fps);
				//���[��
				easing_set(&veh.zradadd_right, ((c.key[4] && c.mode == 1) ? (c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
				easing_set(&veh.zradadd_left, ((c.key[5] && c.mode == 1) ? -(c.key[12] ? rad_spec / 3.f : rad_spec) : 0.f), 0.95f, fps);
				//���[
				easing_set(&veh.yradadd_left, ((c.key[6] && c.mode == 1) ? -(c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
				easing_set(&veh.yradadd_right, ((c.key[7] && c.mode == 1) ? (c.key[12] ? rad_spec / 24.f : rad_spec / 8.f) : 0.f), 0.95f, fps);
				//�X���b�g��
				easing_set(&veh.speed_add, (((c.key[8] && c.mode == 1) && veh.speed < c.useplane.max_speed_limit) ? (0.5f / 3.6f) : 0.f), 0.95f, fps);
				easing_set(&veh.speed_sub, (c.key[9] && c.mode == 1) ? ((veh.speed > c.useplane.min_speed_limit) ? (-0.5f / 3.6f) : ((veh.speed > 0.f) ? (-0.2f / 3.6f) : 0.f)) : 0.f, 0.95f, fps);
				//�X�s�[�h
				veh.speed += (veh.speed_add + veh.speed_sub)*60.f / fps;
				{
					auto tmp = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(veh.mat);
					auto tmp2 = sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
					veh.speed += (((std::abs(tmp2) > sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f)*60.f / fps;	//����
				}
				//���W�n���f
				{
					auto t_mat = veh.mat;
					veh.mat *= VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(t_mat).GetRotAxis((veh.xradadd_right + veh.xradadd_left) / fps);
					veh.mat *= VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(t_mat).GetRotAxis((veh.zradadd_right + veh.zradadd_left) / fps);
					veh.mat *= VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(t_mat).GetRotAxis((veh.yradadd_left + veh.yradadd_right) / fps);
				}
				//
				c.landing_cnt = std::min<uint8_t>(c.landing_cnt + 1, (c.key[13] && c.mode == 1) ? 2 : 0);
				if (c.landing_cnt == 1) {
					c.landing ^= 1;
				}
				//�r
				c.changegear_cnt = std::min<uint8_t>(c.changegear_cnt + 1, (c.key[10] && c.mode == 1) ? 2 : 0);
				if (c.changegear_cnt == 1) {
					c.changegear ^= 1;
				}
				easing_set(&c.p_anime_geardown.second, float(c.changegear), 0.95f, fps);
				MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
				//��
				for (int i = 0; i < c.p_animes_rudder.size(); i++) {
					easing_set(&c.p_animes_rudder[i].second, float(c.key[i + 2] && c.mode == 1), 0.95f, fps);
					MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
				}
				//
				{
					//
					if (veh.speed >= c.useplane.min_speed_limit) {
						easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f, fps);
					}
					else {
						veh.add.yadd(-9.8f / powf(fps, 2.f));
					}

					//���̓��C��-����
					{
						veh.obj.frame_reset(c.useplane.hook.first);
						veh.obj.SetFrameLocalMatrix(c.useplane.hook.first, RotX(deg2rad(c.p_landing_per))*c.useplane.hook.second.Mtrans());
						easing_set(&c.p_landing_per, (c.landing) ? 20.f : 0.f, 0.95f, fps);
						if (c.landing) {
							bool to = false;
							for (auto& wi : wire) {
								carrier.frame_reset(wi.first);
								if ((veh.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).size() <= 30.f) {
									VECTOR_ref vec1 = (veh.obj.frame(c.useplane.hook.first + 1) - veh.obj.frame(c.useplane.hook.first)).Norm();
									VECTOR_ref vec2 = (carrier.frame(wi.first) - veh.obj.frame(c.useplane.hook.first)).Norm();
									if (vec1%vec2 >= 0) {
										to = true;
										carrier.SetFrameLocalMatrix(wi.first, ((veh.obj.frame(c.useplane.hook.first + 1) - carrier.frame(wi.first)).Mtrans()*veh.mat.Inverse()) *wi.second.Mtrans());
										break;
									}
								}
							}
							if (to && veh.speed > 0.f) {
								veh.speed += -2.5f / 3.6f;
							}
						}
					}

					if (c.p_anime_geardown.second >= 0.5f) {
						for (auto& w : c.useplane.wheelframe) {
							easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
							auto tmp = veh.obj.frame(int(w.frame.first + 1));
							{
								auto hp = map_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(veh.mat), tmp, 0, 0);
								if (hp.HitFlag == TRUE) {
									veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(veh.mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(veh.mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(veh.mat);

										auto y_vec2 = y_vec;
										easing_set(&y_vec2, hp.Normal, 0.95f, fps);
										auto normal = y_vec2;

										veh.mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);
									}
									w.gndsmksize = std::clamp(veh.speed*3.6f / 50.f, 0.1f, 1.f);
									if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
										veh.speed += -0.5f / 3.6f;
									}
									if (veh.speed <= 0.f) {
										easing_set(&veh.speed, 0.f, 0.9f, fps);
									}
								}
							}
							{
								auto hp = carrier_col.CollCheck_Line(tmp + VECTOR_ref(VGet(0.f, 0.98f, 0.f)).Vtrans(veh.mat), tmp);
								if (hp.HitFlag == TRUE) {
									veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
									{
										auto x_vec = VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(veh.mat);
										auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(veh.mat);
										auto z_vec = VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(veh.mat);

										auto y_vec2 = y_vec;
										easing_set(&y_vec2, hp.Normal, 0.95f, fps);
										auto normal = y_vec2;

										veh.mat = Axis1(
											x_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											y_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get())),
											z_vec.Vtrans(MGetRotVec2(y_vec.get(), normal.get()))
										);

									}
									veh.add += car_pos_add + ((veh.pos - car_pos).Vtrans(RotY(car_yrad_add)) - (veh.pos - car_pos));
									veh.mat *= RotY(car_yrad_add);

									w.gndsmksize = std::clamp(veh.speed*3.6f / 50.f, 0.1f, 1.f);
									if (veh.speed >= 0.f && (c.key[11] && c.mode == 1)) {
										veh.speed += -1.0f / 3.6f;
									}
									if (veh.speed <= 0.f) {
										easing_set(&veh.speed, 0.f, 0.9f, fps);
									}

									if (c.key[14]) {
										easing_set(&veh.speed, c.useplane.mid_speed_limit, 0.90f, fps);
									}
								}
							}
						}
						auto y_vec = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(veh.mat);

						//�]��
						veh.wheel_Left -= veh.speed / 20.f;// -veh.yradadd * 5.f;
						veh.wheel_Right -= veh.speed / 20.f;// +veh.yradadd * 5.f;

						for (auto& f : c.useplane.wheelframe_nospring) {


							veh.obj.SetFrameLocalMatrix(f.frame.first, 

								VECTOR_ref(VGet(0.f, 0.f, 0.f)).Vtrans(MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)).GetRotAxis(
									(f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right
								)

								//RotX(c.wheel_Right)
								*f.frame.second.Mtrans());
						}
					}
					else {
						for (auto& w : c.useplane.wheelframe) {
							easing_set(&w.gndsmksize, 0.01f, 0.9f, fps);
						}
					}
					veh.pos += veh.add + VECTOR_ref(VGet(0.f, 0.f, -veh.speed / fps)).Vtrans(veh.mat);
				}
				//�ˌ�
				shoot(&c, fps);
				//����
				if (c.useplane.isfloat) {
					veh.pos.y(std::max(veh.pos.y(), -c.useplane.down_in_water));
				}
				//�ǂ̓����蔻��
				VECTOR_ref p_0 = veh.pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(veh.mat);
				VECTOR_ref p_1 = veh.pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.maxpos.z())).Vtrans(veh.mat);
				VECTOR_ref p_2 = veh.pos + VECTOR_ref(VGet(c.useplane.maxpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(veh.mat);
				VECTOR_ref p_3 = veh.pos + VECTOR_ref(VGet(c.useplane.minpos.x(), 0.f, c.useplane.minpos.z())).Vtrans(veh.mat);
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
					c.mode = 0;//��ԂȂǂ̃��[�h�ɂ���
					carrier.SetFrameLocalMatrix(catapult[0].first + 2, RotX(deg2rad(-75))*catapult[0].second.Mtrans());
					auto pp = carrier.frame(catapult[0].first + 1) - carrier.frame(catapult[0].first);
					veh.pos = carrier.frame(catapult[0].first) + VGet(0.f, 3.f, 0.f) + pp.Norm().Scale(6.f);
					veh.mat = RotY(atan2f(-pp.x(), -pp.z()));

					veh.xradadd_right = 0.f;
					veh.xradadd_left = 0.f;
					veh.yradadd_left = 0.f;
					veh.yradadd_right = 0.f;
					veh.zradadd_right = 0.f;
					veh.zradadd_left = 0.f;
					veh.speed_add = 0.f;
					veh.speed_sub = 0.f;
					veh.speed = 0.f;// c.useplane.min_speed_limit;
					veh.add = VGet(0.f, 0.f, 0.f);
					c.p_anime_geardown.second = 1.f;
					c.landing = false;
				}
			}
			/*effect*/
			{
				for (auto& t : c.effcs) {
					if (t.id != ef_smoke1 && t.id != ef_smoke2 && t.id != ef_smoke3) {
						set_pos_effect(&t, Drawparts->get_effHandle(int(t.id)));
					}
				}
				for (auto& t : c.usetank.wheelframe) {
					t.gndsmksize = 0.1f + std::abs(c.vehicle[0].speed) / ((c.usetank.flont_speed_limit / 3.6f) / fps)*0.6f;
					t.gndsmkeffcs.handle.SetPos(c.vehicle[0].obj.frame(t.frame.first) + VECTOR_ref(VGet(0.f, -t.frame.second.y(), 0.f)).Vtrans(c.vehicle[0].mat));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}
				for (auto& t : c.useplane.wheelframe) {
					t.gndsmkeffcs.handle.SetPos(c.vehicle[1].obj.frame(int(t.frame.first + 1)));
					t.gndsmkeffcs.handle.SetScale(t.gndsmksize);
				}
				//c.effcs[ef_smoke1].handle.SetPos(c.vehicle[0].obj.frame(c.ptr->engineframe));
				//c.effcs[ef_smoke2].handle.SetPos(c.vehicle[0].obj.frame(c.ptr->smokeframe[0]));
				//c.effcs[ef_smoke3].handle.SetPos(c.vehicle[0].obj.frame(c.ptr->smokeframe[1]));
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
			float spdrec = c.spd;
			easing_set(&c.spd, std::hypot(c.mine.body->GetLinearVelocity().x, c.mine.body->GetLinearVelocity().y) * ((c.spd > 0) ? 1.f : -1.f), 0.99f, fps);
			c.vehicle[0].speed = c.spd - spdrec;
		}
		//�e����,�e��
		for (auto& c : chara) {
			//�e����
			for (auto& veh : c.vehicle) {
				for (auto& g : veh.Gun_) {
					for (auto& a : g.bullet) {
						float size = 3.f;
						for (int z = 0; z < int(size); z++) {
							if (a.flug) {
								a.repos = a.pos;
								a.pos += a.vec.Scale(a.spec.speed / fps / size);
								a.pos.yadd(a.yadd / size);
								ref_col(c.id, a.pos, a.repos, 5.f);
								//����
								{
									bool ground_hit = false;
									VECTOR_ref normal;
									//��ԈȊO�ɓ�����
									{
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
									}
									//��s�@�ɂ�����
									auto hitplane = hit::p_get_reco(c, chara, a);
									//��Ԃɓ�����
									auto hittank = hit::get_reco(c, chara, a);
									//���̌㏈��
									switch (a.spec.type) {
									case 0://AP
										if (!(hittank || hitplane)) {
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
													a.spec.penetration /= 2.f;
												}
											}
										}
										if (a.flug) {
											a.spec.penetration -= 1.0f / fps / size;
											a.spec.speed -= 5.f / fps / size;
										}
										break;
									case 1://HE
										if (!(hittank || hitplane)) {
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
										if (a.flug) {
											a.spec.speed -= 5.f / fps / size;
										}
										break;
									default:
										break;
									}
								}

								//����(3�b�������A�X�s�[�h��0�ȉ��A�ђʂ�0�ȉ�)
								if (a.cnt >= 3.f || a.spec.speed < 100.f || a.spec.penetration <= 0.f) {
									a.flug = false;
								}
							}
						}
						a.yadd += -9.8f / powf(fps, 2.f);
						a.cnt += 1.f / fps;
					}
				}
			}
			//�e��
			for (auto& veh : c.vehicle) {
				for (auto& h : veh.hit) {
					if (h.flug) {
						auto y_vec = h.y_vec.Vtrans(veh.mat);
						auto z_vec = h.z_vec.Vtrans(veh.mat);

						h.pic.SetScale(h.scale);
						h.pic.SetRotationZYAxis(z_vec, y_vec, 0.f);
						h.pic.SetPosition(veh.pos + h.pos.Vtrans(veh.mat) + y_vec.Scale(0.005f));

						//h.pic.SetMatrix(Axis1((y_vec*z_vec), y_vec, z_vec, (veh.pos + h.pos.Vtrans(veh.mat) + y_vec.Scale(0.005f))) *SetScale(h.scale.get()));
					}
				}
			}
		}
		//�؃Z�b�g
		{
			for (auto& l : tree) {
				l.obj.SetMatrix(SetScale(l.scale)*l.pos.Mtrans());
			}
		}
		{
			//���̍��W�������ŏo��(�z�X�g)
		}
		{
			//�z�X�g����̍��W�������œ���
		}
		//���f���ɔ��f
		for (auto& c : chara) {
			c.vehicle[0].obj.SetMatrix(c.vehicle[0].mat*c.vehicle[0].pos.Mtrans());
			c.vehicle[1].obj.SetMatrix(c.vehicle[1].mat*c.vehicle[1].pos.Mtrans());
			for (auto& be : c.p_burner) {
				be.effectobj.SetMatrix(SetScale(VGet(1.f, 1.f, std::clamp(c.vehicle[1].speed / c.useplane.mid_speed_limit, 0.1f, 1.f)))*be.frame.second.Mtrans()  * c.vehicle[1].mat *c.vehicle[1].pos.Mtrans());
			}
		}
		//�e�p��
		Drawparts->Ready_Shadow(campos, [&map, &chara, &carrier, &tree] {
			carrier.DrawModel();
			for (auto& c : chara) {
				c.vehicle[0].obj.DrawModel();
				c.vehicle[1].obj.DrawModel();
			}
			for (auto& l : tree) {
				l.obj.DrawModel();
			}
		});
		{
			if (ads) {
				campos = mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame1.first) + mine.vehicle[0].Gun_[0].gun_info.frame2.second.Vtrans(RotY(atan2f(eyevec.x(), eyevec.z())));
				camvec = campos - eyevec;
				camup = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(mine.vehicle[0].mat);
			}
			else {
				if (mine.mode == 0) {
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
						camup = camup.Vtrans(VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.xrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(1.f, 0.f, 0.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.zrad_shot));
						camup = camup.Vtrans(VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(mine.vehicle[0].mat).GetRotAxis(mine.xrad_shot));
					}
				}
				else {
					camvec = mine.vehicle[1].pos + VECTOR_ref(VGet(0.f, 6.f, 0.f)).Vtrans(mine.vehicle[1].mat);
					camvec.y(std::max(camvec.y(), 5.f));

					if ((GetMouseInput() & MOUSE_INPUT_RIGHT) == 0) {
						eyevec = (camvec - aimpos[0]).Norm();
						campos = camvec + eyevec.Scale(range);
						camup = VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(mine.vehicle[1].mat);

					}
					else {
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
						}
						camup = VGet(0.f, 1.f, 0.f);
					}
				}
			}
		}
		{
			float dist = 0.f;
			{
				VECTOR_ref aimingpos = campos + (camvec - campos).Norm().Scale(1000.f);
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
				dist = std::clamp((campos - aimingpos).size(), 300.f, 1000.f);
			}
			float neardist = 1.f;
			switch (mine.mode) {
			case 0:
				neardist = (ads ? (1.5f + 198.5f*(dist - 300.f) / (1000.f - 300.f)) : 1.5f);
				break;
			case 1:
				neardist = 10.f;
				break;
			default:
				break;
			}
			//��
			{
				Drawparts->SetDraw_Screen(SkyScreen);
				SetCameraNearFar(1000.0f, 5000.0f);
				SetupCamera_Perspective(deg2rad(90 / 2) / ratio);
				SetCameraPositionAndTargetAndUpVec((campos - camvec).get(), VGet(0, 0, 0), camup.get());
				SetFogEnable(FALSE);
				SetUseLighting(FALSE);
				sky.DrawModel();
				SetUseLighting(TRUE);
				SetFogEnable(TRUE);
			}
			//��ʑ̐[�x�`��
			Hostpassparts->dof(
				&BufScreen,
				SkyScreen,
				[&Drawparts, &map, &carrier, &chara, &ads, &Vertex, &draw_bullets, &tree] {
				Drawparts->Draw_by_Shadow([&map, &carrier, &chara, &ads, &Vertex, &tree] {
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 192, 255);
					{
						DrawPolygon3D(Vertex, 2, DX_NONE_GRAPH, TRUE);
					}
					SetFogStartEnd(0.0f, 3000.f);
					SetFogColor(128, 128, 128);
					{
						map.DrawModel();
						carrier.DrawModel();
						//���
						for (auto& t : chara) {
							if ((!ads && t.id == 0) || t.id != 0) {
								t.vehicle[0].obj.DrawModel();
							}
							for (auto& h : t.vehicle[0].hit) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);//�e��
								}
							}
						}
						//�퓬�@
						for (auto& t : chara) {
							t.vehicle[1].obj.DrawModel();
							for (auto& be : t.p_burner) {
								be.effectobj.DrawModel();//�o�[�i�[
							}
							for (auto& h : t.vehicle[1].hit) {
								if (h.flug) {
									h.pic.DrawFrame(h.use);//�e��
								}
							}
						}
						for (auto& l : tree) {
							l.obj.DrawModel();
						}
					}
				});
				draw_bullets(Drawparts->GetColor(255, 255, 255));
			},
				campos, camvec, camup, deg2rad(90 / 2) / ratio,
				dist,
				neardist
				);
		}
		//
		Drawparts->SetDraw_Screen(DX_SCREEN_BACK);
		//�Ə����W�擾
		{
			SetCameraNearFar(0.01f, 5000.0f);
			SetupCamera_Perspective(deg2rad(45) / ratio);
			SetCameraPositionAndTargetAndUpVec(campos.get(), camvec.get(), camup.get());
			switch (mine.mode) {
			case 0://���
			{
				VECTOR_ref startpos = mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame2.first);
				VECTOR_ref tmppos = startpos + (mine.vehicle[0].obj.frame(mine.vehicle[0].Gun_[0].gun_info.frame3.first) - startpos).Norm().Scale(1000.f);
				{
					for (int i = 0; i < map_col.mesh_num(); i++) {
						auto hp2 = map_col.CollCheck_Line(startpos, tmppos, 0, i);
						if (hp2.HitFlag == TRUE) {
							tmppos = hp2.HitPosition;
						}
					}
					auto hp2 = carrier_col.CollCheck_Line(startpos, tmppos);
					if (hp2.HitFlag == TRUE) {
						tmppos = hp2.HitPosition;
					}
				}
				ref_col(mine.id, startpos, tmppos, 5.f);
				for (auto& t : chara) {
					if (t.vehicle[0].hit_check) {
						for (int i = 0; i < t.vehicle[0].col.mesh_num(); i++) {
							const auto hp2 = t.vehicle[0].col.CollCheck_Line(startpos, tmppos, -1, i);
							if (hp2.HitFlag == TRUE) {
								tmppos = hp2.HitPosition;
							}
						}
					}
					if (t.vehicle[1].hit_check) {
						const auto hp2 = t.vehicle[1].col.CollCheck_Line(startpos, tmppos, -1, -1);
						if (hp2.HitFlag == TRUE) {
							tmppos = hp2.HitPosition;
						}
					}
				}
				easing_set(&aimpos[1], tmppos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos[1].get());
			}
			break;
			case 1:
			{
				VECTOR_ref startpos = mine.vehicle[1].pos;
				VECTOR_ref tmppos = startpos + VECTOR_ref(VGet(0.f, 0.f, -1000.f)).Vtrans(mine.vehicle[1].mat);
				for (int i = 0; i < map_col.mesh_num(); i++) {
					auto hp1 = map_col.CollCheck_Line(startpos, tmppos, 0, i);
					if (hp1.HitFlag == TRUE) {
						tmppos = hp1.HitPosition;
					}
				}
				auto hp1 = carrier_col.CollCheck_Line(startpos, tmppos);
				if (hp1.HitFlag == TRUE) {
					tmppos = hp1.HitPosition;
				}
				easing_set(&aimpos[0], tmppos, 0.9f, fps);
				aimposout = ConvWorldPosToScreenPos(aimpos[0].get());
			}
			break;
			default:
				break;
			};
		}
		//�`��
		{
			//�w�i
			BufScreen.DrawGraph(0, 0, false);
			//�u���[��
			switch (mine.mode) {
			case 0://���
				Hostpassparts->bloom(BufScreen, 64);
				break;
			case 1:
				Hostpassparts->bloom(BufScreen, 255);
				break;
			default:
				break;
			};
			/*
			for (auto& l : wall) {
				DrawLine3D(VGet(l.vehicle[0].pos.x(), 2.f, l.vehicle[0].pos.z()), VGet(l.pos[1].x(), 2.f, l.pos[1].z()), Drawparts->GetColor(255, 0, 0));
			}
			*/
			/*
			for (auto& l : tree) {
				DrawLine3D(VGet(l.pos.x(), l.pos.y(), l.pos.z()), VGet(l.pos.x(), l.pos.y() + 5.f, l.pos.z()), Drawparts->GetColor(255, 0, 0));
			}
			*/
			UIparts->draw(aimposout, mine, ads, fps);

			Debugparts->end_way();

			Debugparts->debug(10, 10, fps, totals);
		}
		Drawparts->Screen_Flip(waits);
		totals = float(GetNowHiPerformanceCount() - waits) / 1000.f;
	}
	return 0; // �\�t�g�̏I��
}
