#pragma once
class Mapclass:Mainclass {
private:
	MV1 map, map_col;					    //地面
	MV1 tree_model, tree_far;				    //木
	MV1 sky;	  //空
	MV1 sea;	  //海
	std::vector<treePats> tree;	//壁をセット
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
	}

	void set_map() {
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

};
