#pragma once

class GRASS {
private:
	int grasss = 250;				/*grassの数*/
	std::vector<VERTEX3D> grassver; /*grass*/
	std::vector<DWORD> grassind;    /*grass*/
	int VerBuf, IndexBuf;			/*grass*/
	MV1 grass;						/*grassモデル*/
	GraphHandle graph;				/*画像ハンドル*/
	int IndexNum, VerNum;			/*grass*/
	int vnum, pnum;					/*grass*/
	MV1_REF_POLYGONLIST RefMesh;	/*grass*/
public:
	GRASS(void) {
	}
	void set_grass_ready_before(void) {
		SetUseASyncLoadFlag(FALSE);
		graph = GraphHandle::Load("data/model/cloud/cloud.png");		 /*grass*/
		MV1::Load("data/model/cloud/model.mv1", &grass, false);		/*grass*/
		SetUseASyncLoadFlag(FALSE);
		return;
	}
	void set_grass_cancel(void) {
		SetASyncLoadFinishDeleteFlag(graph.get());										 /*grass*/
		SetASyncLoadFinishDeleteFlag(grass.get());		/*grass*/
	}
	bool set_grass_ready(const char* buf, const float x_max = 10.f, const float z_max = 10.f, const float x_min = -10.f, const float z_min = -10.f) {
		/*grass*/
		MV1SetupReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの作成*/
		RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/
		IndexNum = RefMesh.PolygonNum * 3 * grasss; /*インデックスの数を取得*/
		VerNum = RefMesh.VertexNum * grasss;	/*頂点の数を取得*/
		grassver.resize(VerNum);   /*頂点データとインデックスデータを格納するメモリ領域の確保*/
		grassind.resize(IndexNum); /*頂点データとインデックスデータを格納するメモリ領域の確保*/

		vnum = 0;
		pnum = 0;

		int grass_pos = LoadSoftImage(buf);
		int xs = 0, ys = 0;
		GetSoftImageSize(grass_pos, &xs, &ys);

		for (int i = 0; i < grasss; ++i) {

			float x_t = (float)(GetRand(int((x_max - x_min)) * 100) - int(x_max - x_min) * 50) / 100.0f;
			float z_t = (float)(GetRand(int((z_max - z_min)) * 100) - int(z_max - z_min) * 50) / 100.0f;
			int _r_, _g_, _b_, _a_;
			while (1) {
				GetPixelSoftImage(grass_pos, int((x_t - x_min) / (x_max - x_min)*float(xs)), int((z_t - z_min) / (z_max - z_min)*float(ys)), &_r_, &_g_, &_b_, &_a_);
				if (_r_ <= 128) {
					break;
				}
				else {
					x_t = (float)(GetRand(int((x_max - x_min)) * 100) - int(x_max - x_min) * 50) / 100.0f;
					z_t = (float)(GetRand(int((z_max - z_min)) * 100) - int(z_max - z_min) * 50) / 100.0f;
				}
			}
			VECTOR_ref tmpvect = VGet(x_t, float(2400-500+GetRand(1000)), z_t);
			//
			MV1SetMatrix(grass.get(), MMult(MMult(MGetRotY(deg2rad(GetRand(90))), MGetScale(VGet(144.f, 72.f, 144.f))), MGetTranslate(tmpvect.get())));
			//上省
			MV1RefreshReferenceMesh(grass.get(), -1, TRUE);       /*参照用メッシュの更新*/
			RefMesh = MV1GetReferenceMesh(grass.get(), -1, TRUE); /*参照用メッシュの取得*/
			for (int j = 0; j < RefMesh.VertexNum; ++j) {
				auto& g = grassver[j + vnum];
				g.pos = RefMesh.Vertexs[j].Position;
				g.norm = RefMesh.Vertexs[j].Normal;
				g.dif = RefMesh.Vertexs[j].DiffuseColor;
				g.spc = RefMesh.Vertexs[j].SpecularColor;
				g.u = RefMesh.Vertexs[j].TexCoord[0].u;
				g.v = RefMesh.Vertexs[j].TexCoord[0].v;
				g.su = RefMesh.Vertexs[j].TexCoord[1].u;
				g.sv = RefMesh.Vertexs[j].TexCoord[1].v;
			}
			for (size_t j = 0; j < size_t(RefMesh.PolygonNum); ++j) {
				for (size_t k = 0; k < std::size(RefMesh.Polygons[j].VIndex); ++k)
					grassind[j * 3 + k + pnum] = WORD(RefMesh.Polygons[j].VIndex[k] + vnum);
			}
			vnum += RefMesh.VertexNum;
			pnum += RefMesh.PolygonNum * 3;
		}
		VerBuf = CreateVertexBuffer(VerNum, DX_VERTEX_TYPE_NORMAL_3D);
		IndexBuf = CreateIndexBuffer(IndexNum, DX_INDEX_TYPE_32BIT);
		SetVertexBufferData(0, grassver.data(), VerNum, VerBuf);
		SetIndexBufferData(0, grassind.data(), IndexNum, IndexBuf);
		return true;
	}
	void draw_grass(void) {
		SetFogStartEnd(0.0f, 500.f);
		SetFogColor(192,192,192);

		SetDrawAlphaTest(DX_CMP_GREATER, 128);
		SetUseLighting(FALSE);
		{
			DrawPolygonIndexed3D_UseVertexBuffer(VerBuf, IndexBuf, graph.get(), TRUE);
		}
		SetUseLighting(TRUE);
		SetDrawAlphaTest(-1, 0);

	}
	void delete_grass(void) {
		graph.Dispose();
		grass.Dispose();
		grassver.clear();
		grassind.clear();
	}

};


class Mapclass:Mainclass {
private:
	//マップ
	struct treePats {
		MV1 obj, obj_far;
		MATRIX_ref mat;
		VECTOR_ref pos;
	};
	//マップ
	std::array<int, 4> cloudpic_b;
	std::array<GraphHandle, 4> cloudpic;
	MV1 cloud;
	struct cloudParts {
		GraphHandle pic;
		VECTOR_ref pos;
	};
	MV1 map, map_col;					    //地面
	MV1 tree_model, tree_far;				    //木
	MV1 sky;	  //空
	MV1 sea;	  //海
	std::vector<treePats> tree;	//壁をセット
	std::list<cloudParts> clouds;
public:
	Mapclass() {
	}

	~Mapclass() {

	}

	void set_map_pre() {
		MV1::Load("data/map_new/model.mv1", &map, true);		   //map
		MV1::Load("data/map_new/col.mv1", &map_col, true);		   //mapコリジョン
		MV1::Load("data/model/tree/model.mv1", &tree_model, true); //木
		MV1::Load("data/model/tree/model2.mv1", &tree_far, true); //木
		MV1::Load("data/model/sky/model.mv1", &sky, true);	 //空
		MV1::Load("data/model/sea/model.mv1", &sea, true);	 //海

		MV1::Load("data/model/cloud/model.mv1", &cloud, true);	 //雲

		GraphHandle::LoadDiv("data/model/cloud/cloud.png", 4, 2, 2, 256, 256, &cloudpic_b[0]);
		for (int i = 0; i < 4; i++) {
			cloudpic[i] = cloudpic_b[i];
		}
	}

	void set_map() {
		map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);

		cloud.material_AlphaTestAll(true, DX_CMP_GREATER, 128);

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

				}
				else if (p.Polygons[i].MaterialIndex == 3) {
					//木
					tree.resize(tree.size() + 1);
					tree.back().mat = MATRIX_ref::Scale(VGet(15.f / 10.f, 15.f / 10.f, 15.f / 10.f));
					tree.back().pos = (VECTOR_ref(p.Vertexs[p.Polygons[i].VIndex[0]].Position) + p.Vertexs[p.Polygons[i].VIndex[1]].Position + p.Vertexs[p.Polygons[i].VIndex[2]].Position) * (1.f / 3.f);

					tree.back().obj = tree_model.Duplicate();
					tree.back().obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
					tree.back().obj_far = tree_far.Duplicate();
					tree.back().obj_far.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				}
			}
		}

	}

	void delete_map() {
		map.Dispose();		   //map
		map_col.Dispose();		   //mapコリジョン
		tree_model.Dispose(); //木
		tree_far.Dispose(); //木
		sky.Dispose();	 //空
		sea.Dispose();	 //海
		for (auto&t : tree) {
			t.obj.Dispose();
			t.obj_far.Dispose();
		}
		tree.clear();
	}

	void map_settree() {
		for (auto& l : tree) {
			l.obj.SetMatrix(l.mat * MATRIX_ref::Mtrans(l.pos));
			l.obj_far.SetMatrix(l.mat * MATRIX_ref::Mtrans(l.pos));
		}
	}
	void map_drawtree() {
		for (auto& l : tree) {
			l.obj.DrawModel();
			//l.obj_far.DrawModel();
		}
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
		SetFogStartEnd(0.0f, 25000.f);
		SetFogColor(128, 192, 255);
		{
			sea.SetPosition(VGet(campos.x(), -5.f, campos.z()));
			sea.SetScale(VGet(10.f, 10.f, 10.f));
			sea.DrawModel();
		}
	}
	//空描画
	void sky_draw(void) {
		SetCameraNearFar(1000.0f, 5000.0f);
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		{
			sky.DrawModel();
		}
		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
	}

	void cloud_draw(void) {
		/*
		SetDrawAlphaTest(DX_CMP_GREATER, 128);
		SetUseLighting(FALSE);
		for (int i = 0; i < 2500; i++) {
			cloud.SetPosition(VGet((i / 100) * 20, 45, (i % 100) * 20));
			cloud.DrawModel();
		}
		SetUseLighting(TRUE);
		SetDrawAlphaTest(-1, 0);
		*/
	}
};
