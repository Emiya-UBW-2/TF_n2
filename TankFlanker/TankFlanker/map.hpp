#pragma once
class Mapclass {
private:
	MV1 map, map_col;					    //�n��
	MV1 tree_model, tree_far;				    //��
	MV1 sky;	  //��
	MV1 sea;	  //�C
	GraphHandle SkyScreen;

public:
	Mapclass() {
		SkyScreen = GraphHandle::Make(dispx, dispy);    //��`��
	}

	~Mapclass() {

	}
	void set_map_pre() {
		MV1::Load("data/map/model.mv1", &map, true);		   //map
		MV1::Load("data/map/col.mv1", &map_col, true);		   //map�R���W����
		MV1::Load("data/model/tree/model.mv1", &tree_model, true); //��
		MV1::Load("data/model/tree/model2.mv1", &tree_far, true); //��
		MV1::Load("data/model/sky/model.mv1", &sky, true);	 //��
		MV1::Load("data/model/sea/model.mv1", &sea, true);	 //�C
	}

	void set_map(std::vector<Mainclass::wallPats>* wall, std::vector<Mainclass::treePats>* tree, std::unique_ptr<b2World>& world) {
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);

		VECTOR_ref size;
		for (int i = 0; i < map_col.mesh_num(); i++) {
			VECTOR_ref sizetmp = map_col.mesh_maxpos(i) - map_col.mesh_minpos(i);
			if (size.x() < sizetmp.x()) {
				size.x(sizetmp.x());
			}
			if (size.y() < sizetmp.y()) {
				size.y(sizetmp.y());
			}
			if (size.z() < sizetmp.z()) {
				size.z(sizetmp.z());
			}
		}
		for (int i = 0; i < map_col.mesh_num(); i++) {
			map_col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, i);
		}
		{
			MV1SetupReferenceMesh(map_col.get(), 0, FALSE);
			MV1_REF_POLYGONLIST p = MV1GetReferenceMesh(map_col.get(), 0, FALSE);

			for (int i = 0; i < p.PolygonNum; i++) {
				if (p.Polygons[i].MaterialIndex == 2) {
					//��
					{
						wall->resize(wall->size() + 1);
						wall->back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
						wall->back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
						if (b2DistanceSquared(b2Vec2(wall->back().pos[0].x(), wall->back().pos[0].z()), b2Vec2(wall->back().pos[1].x(), wall->back().pos[1].z())) <= 0.005f * 0.005f) {
							wall->pop_back();
						}

						wall->resize(wall->size() + 1);
						wall->back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[1]].Position;
						wall->back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
						if (b2DistanceSquared(b2Vec2(wall->back().pos[0].x(), wall->back().pos[0].z()), b2Vec2(wall->back().pos[1].x(), wall->back().pos[1].z())) <= 0.005f * 0.005f) {
							wall->pop_back();
						}

						wall->resize(wall->size() + 1);
						wall->back().pos[0] = p.Vertexs[p.Polygons[i].VIndex[2]].Position;
						wall->back().pos[1] = p.Vertexs[p.Polygons[i].VIndex[0]].Position;
						if (b2DistanceSquared(b2Vec2(wall->back().pos[0].x(), wall->back().pos[0].z()), b2Vec2(wall->back().pos[1].x(), wall->back().pos[1].z())) <= 0.005f * 0.005f) {
							wall->pop_back();
						}
					}
				}
				else if (p.Polygons[i].MaterialIndex == 3) {
					//��
					tree->resize(tree->size() + 1);
					tree->back().mat = MATRIX_ref::Scale(VGet(15.f / 10.f, 15.f / 10.f, 15.f / 10.f));
					tree->back().pos = (VECTOR_ref(p.Vertexs[p.Polygons[i].VIndex[0]].Position) + p.Vertexs[p.Polygons[i].VIndex[1]].Position + p.Vertexs[p.Polygons[i].VIndex[2]].Position) * (1.f / 3.f);
					tree->back().fall_flag = false;
					tree->back().fall_vec = VGet(0.f, 0.f, 1.f);
					tree->back().fall_rad = 0.f;

					tree->back().obj = tree_model.Duplicate();
					tree->back().obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
					tree->back().obj_far = tree_far.Duplicate();
					tree->back().obj_far.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				}
			}
		}

		for (auto& w : *wall) {
			std::array<b2Vec2, 2> vs;
			vs[0].Set(w.pos[0].x(), w.pos[0].z());
			vs[1].Set(w.pos[1].x(), w.pos[1].z());
			b2ChainShape chain;		// This a chain shape with isolated vertices
			chain.CreateChain(&vs[0], 2);
			b2FixtureDef fixtureDef;			       /*���I�{�f�B�t�B�N�X�`�����`���܂�*/
			fixtureDef.shape = &chain;			       /**/
			fixtureDef.density = 1.0f;			       /*�{�b�N�X���x���[���ȊO�ɐݒ肷��ƁA���I�ɂȂ�܂�*/
			fixtureDef.friction = 0.3f;			       /*�f�t�H���g�̖��C���I�[�o�[���C�h���܂�*/
			b2BodyDef bodyDef;				       /*�_�C�i�~�b�N�{�f�B���`���܂��B���̈ʒu��ݒ肵�A�{�f�B�t�@�N�g�����Ăяo���܂�*/
			bodyDef.type = b2_staticBody;			       /**/
			bodyDef.position.Set(0, 0);			       /**/
			bodyDef.angle = 0.f;				       /**/
			w.b2.body.reset(world->CreateBody(&bodyDef));	  /**/
			w.b2.playerfix = w.b2.body->CreateFixture(&fixtureDef); /*�V�F�C�v���{�f�B�ɒǉ����܂�*/
		}
	}
	void delete_map(std::vector<Mainclass::wallPats>* wall, std::vector<Mainclass::treePats>* tree) {
		map.Dispose();		   //map
		map_col.Dispose();		   //map�R���W����
		tree_model.Dispose(); //��
		tree_far.Dispose(); //��
		sky.Dispose();	 //��
		sea.Dispose();	 //�C
		for(auto&t : *tree){
			t.obj.Dispose();
			t.obj_far.Dispose();
		}
		for (auto& w : *wall) {
			delete w.b2.playerfix->GetUserData();
			w.b2.playerfix->SetUserData(NULL);
		}
		wall->clear();
		tree->clear();

	}

	auto& map_get() { return map; }

	auto& map_col_get() { return map_col; }

	auto map_col_line(const VECTOR_ref& startpos, const VECTOR_ref& endpos, const int&  i) {
		return map_col.CollCheck_Line(startpos, endpos, 0, i);
	}

	bool map_col_line_nearest(const VECTOR_ref& startpos, VECTOR_ref* endpos) {
		bool p = false;
		for (int i = 0; i < map_col_get().mesh_num(); i++) {
			auto hp = map_col_line(startpos, *endpos, i);
			if (hp.HitFlag == TRUE) {
				*endpos = hp.HitPosition;
				p = true;
			}
		}
		return p;
	}


	void sea_draw(const VECTOR_ref& campos) {
		SetFogStartEnd(0.0f, 6000.f);
		SetFogColor(128, 192, 255);
		{
			sea.SetPosition(VGet(campos.x(), 0.f, campos.z()));
			sea.DrawModel();
		}
	}
	//��`��
	GraphHandle& sky_draw(const VECTOR_ref& campos, const VECTOR_ref&camvec, const VECTOR_ref& camup, float fov) {
		SkyScreen.SetDraw_Screen(1000.0f, 5000.0f, fov, VECTOR_ref(campos) - camvec, VGet(0, 0, 0), camup);
		{
			SetFogEnable(FALSE);
			SetUseLighting(FALSE);
			sky.DrawModel();
			SetUseLighting(TRUE);
			SetFogEnable(TRUE);
		}
		return SkyScreen;
	}

};
