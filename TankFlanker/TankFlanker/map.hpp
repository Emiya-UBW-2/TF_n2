#pragma once

class Mapclass:Mainclass {
private:
	//マップ
	MV1 map, map_col;					    //地面
	MV1 sky;	  //空
	MV1 sea;	  //海
	//雲
	int clouds = 125;				/*grassの数*/
	std::vector<VERTEX3D> cloudver; /*grass*/
	std::vector<DWORD> cloudind;    /*grass*/
	int VerBuf, IndexBuf;			/*grass*/
	MV1 cloud;						/*grassモデル*/
	GraphHandle cloud_pic;				/*画像ハンドル*/
	int IndexNum, VerNum;			/*grass*/
	int vnum, pnum;					/*grass*/
	MV1_REF_POLYGONLIST RefMesh;	/*grass*/

	//海
	int PixelShaderHandle;
	int VertexShaderHandle;
	int vscbhandle;
	int pscbhandle;
	FLOAT4 *f4;
	float g_fTime;
	int OldTime;

public:
	Mapclass() {
	}

	~Mapclass() {

	}

	void set_map_pre() {
		MV1::Load("data/map_new/model.mv1", &map, true);		   //map
		MV1::Load("data/map_new/col.mv1", &map_col, true);		   //mapコリジョン
		MV1::Load("data/model/sky/model.mv1", &sky, true);	 //空
		MV1::Load("data/model/sea/model.mv1", &sea, true);	 //海

		SetUseASyncLoadFlag(TRUE);
		cloud_pic = GraphHandle::Load("data/model/cloud/cloud.png");		 /*grass*/
		SetUseASyncLoadFlag(FALSE);
		MV1::Load("data/model/cloud/model.mv1", &cloud, true);		/*grass*/

		vscbhandle = CreateShaderConstantBuffer(sizeof(float) * 4);
		VertexShaderHandle = LoadVertexShader("shader/NormalMesh_DirLight_PhongVS.vso");	// 頂点シェーダーを読み込む
		pscbhandle = CreateShaderConstantBuffer(sizeof(float) * 4);
		PixelShaderHandle = LoadPixelShader("shader/NormalMesh_DirLight_PhongPS.pso");	// ピクセルシェーダーを読み込む
		g_fTime = 0.f;
		OldTime = GetNowCount();
	}

	void set_map(const char* buf, const float x_max = 10.f, const float z_max = 10.f, const float x_min = -10.f, const float z_min = -10.f) {
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
				}
			}
		}
		{
			/*grass*/
			MV1SetupReferenceMesh(cloud.get(), -1, TRUE); /*参照用メッシュの作成*/
			RefMesh = MV1GetReferenceMesh(cloud.get(), -1, TRUE); /*参照用メッシュの取得*/
			IndexNum = RefMesh.PolygonNum * 3 * clouds; /*インデックスの数を取得*/
			VerNum = RefMesh.VertexNum * clouds;	/*頂点の数を取得*/
			cloudver.resize(VerNum);   /*頂点データとインデックスデータを格納するメモリ領域の確保*/
			cloudind.resize(IndexNum); /*頂点データとインデックスデータを格納するメモリ領域の確保*/

			vnum = 0;
			pnum = 0;

			int grass_pos = LoadSoftImage(buf);
			int xs = 0, ys = 0;
			GetSoftImageSize(grass_pos, &xs, &ys);
			for (int i = 0; i < clouds; ++i) {

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
				VECTOR_ref tmpvect = VGet(x_t, float(2400 + GetRand(1500)), z_t);
				//
				MV1SetMatrix(cloud.get(), MMult(MMult(MGetRotY(deg2rad(GetRand(90))), MGetScale(VGet(144.f, 72.f, 144.f))), MGetTranslate(tmpvect.get())));
				//上省
				MV1RefreshReferenceMesh(cloud.get(), -1, TRUE);       /*参照用メッシュの更新*/
				RefMesh = MV1GetReferenceMesh(cloud.get(), -1, TRUE); /*参照用メッシュの取得*/
				for (int j = 0; j < RefMesh.VertexNum; ++j) {
					auto& g = cloudver[j + vnum];
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
						cloudind[j * 3 + k + pnum] = WORD(RefMesh.Polygons[j].VIndex[k] + vnum);
				}
				vnum += RefMesh.VertexNum;
				pnum += RefMesh.PolygonNum * 3;
			}
			DeleteSoftImage(grass_pos);

			VerBuf = CreateVertexBuffer(VerNum, DX_VERTEX_TYPE_NORMAL_3D);
			IndexBuf = CreateIndexBuffer(IndexNum, DX_INDEX_TYPE_32BIT);
			SetVertexBufferData(0, cloudver.data(), VerNum, VerBuf);
			SetIndexBufferData(0, cloudind.data(), IndexNum, IndexBuf);
		}
	}

	void delete_map() {
		map.Dispose();		   //map
		map_col.Dispose();		   //mapコリジョン
		sky.Dispose();	 //空
		sea.Dispose();	 //海
		cloud_pic.Dispose();
		cloud.Dispose();
		cloudver.clear();
		cloudind.clear();
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
	void sea_draw_set() {/*const VECTOR_ref& campos*/
		{
			auto Time = GetNowCount();	// 現在の時間を得る
			g_fTime += float(Time - OldTime) / 1000.0f*0.5f;
			OldTime = Time;				// 現在の時間を保存
		}
		//sea.SetPosition(VGet(campos.x(), -5.f, campos.z()));
	}

	void sea_draw() {
		SetFogStartEnd(0.0f, 11500.f);
		SetFogColor(72, 164, 218);
		{
			SetUseVertexShader(VertexShaderHandle);	// 使用する頂点シェーダーをセット
			SetUsePixelShader(PixelShaderHandle);	// 使用するピクセルシェーダーをセット
			{
				//
				f4 = (FLOAT4 *)GetBufferShaderConstantBuffer(vscbhandle);			// 頂点シェーダー用の定数バッファのアドレスを取得
				f4->x = g_fTime;
				UpdateShaderConstantBuffer(vscbhandle);								// 頂点シェーダー用の定数バッファを更新して書き込んだ内容を反映する
				SetShaderConstantBuffer(vscbhandle, DX_SHADERTYPE_VERTEX, 4);		// 頂点シェーダーの定数バッファを定数バッファレジスタ４にセット
				//
				f4 = (FLOAT4 *)GetBufferShaderConstantBuffer(pscbhandle);			// ピクセルシェーダー用の定数バッファのアドレスを取得
				f4->x = g_fTime;
				UpdateShaderConstantBuffer(pscbhandle);								// ピクセルシェーダー用の定数バッファを更新して書き込んだ内容を反映する
				SetShaderConstantBuffer(pscbhandle, DX_SHADERTYPE_PIXEL, 3);		// ピクセルシェーダー用の定数バッファを定数バッファレジスタ３にセット
			}
			MV1SetUseOrigShader(TRUE);
			sea.DrawModel();
			MV1SetUseOrigShader(FALSE);
		}
	}
	//空描画
	void sky_draw(void) {
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		{
			sky.DrawModel();
		}
		SetUseLighting(TRUE);
		SetFogEnable(TRUE);
	}

	void cloud_draw(void) {
		SetFogStartEnd(0.0f, 500.f);
		SetFogColor(192, 192, 192);

		SetDrawAlphaTest(DX_CMP_GREATER, 128);
		SetUseLighting(FALSE);
		{
			DrawPolygonIndexed3D_UseVertexBuffer(VerBuf, IndexBuf, cloud_pic.get(), TRUE);
		}
		SetUseLighting(TRUE);
		SetDrawAlphaTest(-1, 0);
	}
};
