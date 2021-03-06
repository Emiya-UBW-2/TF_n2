#pragma once

class Mapclass :Mainclass {
private:
	//�}�b�v
	MV1 map, map_col;				//�n��
	MV1 sky;						//��
	MV1 sea;						//�C
	//�_
	int clouds = 125;				/*cloud�̐�*/
	std::vector<VERTEX3D> cloudver; /*cloud*/
	std::vector<DWORD> cloudind;    /*cloud*/
	int VerBuf, IndexBuf;			/*cloud*/
	MV1 cloud;						/*cloud���f��*/
	GraphHandle cloud_pic;			/*�摜�n���h��*/
	int IndexNum, VerNum;			/*cloud*/
	int vnum, pnum;					/*cloud*/
	MV1_REF_POLYGONLIST RefMesh;	/*cloud*/
	//���z
	GraphHandle sun_pic;			/*�摜�n���h��*/
	VECTOR_ref sun_pos;
	//�C
	int PixelShaderHandle;
	int VertexShaderHandle;
	int vscbhandle;
	int pscbhandle;
	float g_fTime;
	int OldTime;
public:
	void set_map_pre() {
		MV1::Load("data/map_new/model.mv1", &this->map, true);		//map
		MV1::Load("data/map_new/col.mv1", &this->map_col, true);	//map�R���W����
		MV1::Load("data/model/sky/model.mv1", &this->sky, true);	//��
		MV1::Load("data/model/sea/model.mv1", &this->sea, true);	//�C

		SetUseASyncLoadFlag(TRUE);
		this->sun_pic = GraphHandle::Load("data/sun.png");					/*sun*/
		this->cloud_pic = GraphHandle::Load("data/model/cloud/cloud.png");	/*cloud*/
		SetUseASyncLoadFlag(FALSE);
		MV1::Load("data/model/cloud/model.mv1", &this->cloud, true);		/*cloud*/

		this->vscbhandle = CreateShaderConstantBuffer(sizeof(float) * 4);
		this->VertexShaderHandle = LoadVertexShader("shader/NormalMesh_DirLight_PhongVS.vso");	// ���_�V�F�[�_�[��ǂݍ���
		this->pscbhandle = CreateShaderConstantBuffer(sizeof(float) * 4);
		this->PixelShaderHandle = LoadPixelShader("shader/NormalMesh_DirLight_PhongPS.pso");	// �s�N�Z���V�F�[�_�[��ǂݍ���
		this->g_fTime = 0.f;
		this->OldTime = GetNowCount();
	}
	void set_map(const char* buf, const VECTOR_ref& ray, const float x_max = 10.f, const float z_max = 10.f, const float x_min = -10.f, const float z_min = -10.f) {
		this->sun_pos = ray.Norm() * -1500.f;
		this->map.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
		VECTOR_ref size;
		for (int i = 0; i < this->map_col.mesh_num(); i++) {
			VECTOR_ref sizetmp = this->map_col.mesh_maxpos(i) - this->map_col.mesh_minpos(i);
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
		for (int i = 0; i < this->map_col.mesh_num(); i++) {
			this->map_col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, i);
		}
		{
			MV1SetupReferenceMesh(this->map_col.get(), 0, FALSE);
			MV1_REF_POLYGONLIST p = MV1GetReferenceMesh(this->map_col.get(), 0, FALSE);

			for (int i = 0; i < p.PolygonNum; i++) {
				if (p.Polygons[i].MaterialIndex == 2) {
				}
				else if (p.Polygons[i].MaterialIndex == 3) {
				}
			}
		}
		{
			/*cloud*/
			MV1SetupReferenceMesh(this->cloud.get(), -1, TRUE); /*�Q�Ɨp���b�V���̍쐬*/
			this->RefMesh = MV1GetReferenceMesh(this->cloud.get(), -1, TRUE); /*�Q�Ɨp���b�V���̎擾*/
			this->IndexNum = this->RefMesh.PolygonNum * 3 * this->clouds; /*�C���f�b�N�X�̐����擾*/
			this->VerNum = this->RefMesh.VertexNum * this->clouds;	/*���_�̐����擾*/
			this->cloudver.resize(VerNum);   /*���_�f�[�^�ƃC���f�b�N�X�f�[�^���i�[���郁�����̈�̊m��*/
			this->cloudind.resize(IndexNum); /*���_�f�[�^�ƃC���f�b�N�X�f�[�^���i�[���郁�����̈�̊m��*/

			this->vnum = 0;
			this->pnum = 0;

			int grass_pos = LoadSoftImage(buf);
			int xs = 0, ys = 0;
			GetSoftImageSize(grass_pos, &xs, &ys);
			for (int i = 0; i < this->clouds; ++i) {

				float x_t = (float)(GetRand(int(x_max - x_min) * 100) - int(x_max - x_min) * 50) / 100.0f;
				float z_t = (float)(GetRand(int(z_max - z_min) * 100) - int(z_max - z_min) * 50) / 100.0f;
				int _r_, _g_, _b_, _a_;
				while (1) {
					GetPixelSoftImage(grass_pos, int((x_t - x_min) / (x_max - x_min)*float(xs)), int((z_t - z_min) / (z_max - z_min)*float(ys)), &_r_, &_g_, &_b_, &_a_);
					if (_r_ <= 128) {
						break;
					}
					else {
						x_t = (float)(GetRand(int(x_max - x_min) * 100) - int(x_max - x_min) * 50) / 100.0f;
						z_t = (float)(GetRand(int(z_max - z_min) * 100) - int(z_max - z_min) * 50) / 100.0f;
					}
				}
				//
				MV1SetMatrix(this->cloud.get(), MMult(MMult(MGetRotY(deg2rad(GetRand(90))), MGetScale(VGet(144.f, 72.f, 144.f))), MGetTranslate(VGet(x_t, float(2400 + GetRand(1500)), z_t))));
				//���
				MV1RefreshReferenceMesh(this->cloud.get(), -1, TRUE);       /*�Q�Ɨp���b�V���̍X�V*/
				this->RefMesh = MV1GetReferenceMesh(this->cloud.get(), -1, TRUE); /*�Q�Ɨp���b�V���̎擾*/
				for (int j = 0; j < this->RefMesh.VertexNum; ++j) {
					auto& g = this->cloudver[j + vnum];
					g.pos = this->RefMesh.Vertexs[j].Position;
					g.norm = this->RefMesh.Vertexs[j].Normal;
					g.dif = this->RefMesh.Vertexs[j].DiffuseColor;
					g.spc = this->RefMesh.Vertexs[j].SpecularColor;
					g.u = this->RefMesh.Vertexs[j].TexCoord[0].u;
					g.v = this->RefMesh.Vertexs[j].TexCoord[0].v;
					g.su = this->RefMesh.Vertexs[j].TexCoord[1].u;
					g.sv = this->RefMesh.Vertexs[j].TexCoord[1].v;
				}
				for (size_t j = 0; j < size_t(this->RefMesh.PolygonNum); ++j) {
					for (size_t k = 0; k < std::size(this->RefMesh.Polygons[j].VIndex); ++k)
						cloudind[j * 3 + k + this->pnum] = WORD(this->RefMesh.Polygons[j].VIndex[k] + this->vnum);
				}
				this->vnum += this->RefMesh.VertexNum;
				this->pnum += this->RefMesh.PolygonNum * 3;
			}
			DeleteSoftImage(grass_pos);

			this->VerBuf = CreateVertexBuffer(this->VerNum, DX_VERTEX_TYPE_NORMAL_3D);
			this->IndexBuf = CreateIndexBuffer(this->IndexNum, DX_INDEX_TYPE_32BIT);
			SetVertexBufferData(0, this->cloudver.data(), this->VerNum, this->VerBuf);
			SetIndexBufferData(0, this->cloudind.data(), this->IndexNum, this->IndexBuf);
		}
	}
	void delete_map() {
		this->map.Dispose();		   //map
		this->map_col.Dispose();		   //map�R���W����
		this->sky.Dispose();	 //��
		this->sea.Dispose();	 //�C
		this->cloud_pic.Dispose();
		this->cloud.Dispose();
		this->cloudver.clear();
		this->cloudind.clear();
		this->sun_pic.Dispose();
		DeleteShader(this->VertexShaderHandle);	// ���_�V�F�[�_�[��ǂݍ���
		DeleteShader(this->PixelShaderHandle);	// �s�N�Z���V�F�[�_�[��ǂݍ���

	}
	auto& map_get() { return this->map; }
	auto& map_col_get() { return this->map_col; }
	auto map_col_line(const VECTOR_ref& startpos, const VECTOR_ref& endpos, const int&  i) {
		return this->map_col.CollCheck_Line(startpos, endpos, 0, i);
	}
	bool map_col_line_nearest(const VECTOR_ref& startpos, VECTOR_ref* endpos) {
		bool p = false;
		for (int i = 0; i < this->map_col_get().mesh_num(); i++) {
			auto hp = this->map_col_line(startpos, *endpos, i);
			if (hp.HitFlag == TRUE) {
				*endpos = hp.HitPosition;
				p = true;
			}
		}
		return p;
	}
	void sea_draw_set() {
		auto Time = GetNowCount();	// ���݂̎��Ԃ𓾂�
		this->g_fTime += float(Time - this->OldTime) / 1000.0f*0.5f;
		this->OldTime = Time;				// ���݂̎��Ԃ�ۑ�
	}
	void sea_draw() {
		SetFogStartEnd(12500.0f, 20000.f);
		SetFogColor(126, 168, 193);
		SetUseVertexShader(this->VertexShaderHandle);	// �g�p���钸�_�V�F�[�_�[���Z�b�g
		SetUsePixelShader(this->PixelShaderHandle);	// �g�p����s�N�Z���V�F�[�_�[���Z�b�g
		{
			FLOAT4 *f4;
			//
			f4 = (FLOAT4 *)GetBufferShaderConstantBuffer(this->vscbhandle);		// ���_�V�F�[�_�[�p�̒萔�o�b�t�@�̃A�h���X���擾
			f4->x = this->g_fTime;
			UpdateShaderConstantBuffer(this->vscbhandle);						// ���_�V�F�[�_�[�p�̒萔�o�b�t�@���X�V���ď������񂾓��e�𔽉f����
			SetShaderConstantBuffer(this->vscbhandle, DX_SHADERTYPE_VERTEX, 4);	// ���_�V�F�[�_�[�̒萔�o�b�t�@��萔�o�b�t�@���W�X�^�S�ɃZ�b�g
			//
			f4 = (FLOAT4 *)GetBufferShaderConstantBuffer(this->pscbhandle);		// �s�N�Z���V�F�[�_�[�p�̒萔�o�b�t�@�̃A�h���X���擾
			f4->x = this->g_fTime;
			UpdateShaderConstantBuffer(this->pscbhandle);						// �s�N�Z���V�F�[�_�[�p�̒萔�o�b�t�@���X�V���ď������񂾓��e�𔽉f����
			SetShaderConstantBuffer(this->pscbhandle, DX_SHADERTYPE_PIXEL, 3);	// �s�N�Z���V�F�[�_�[�p�̒萔�o�b�t�@��萔�o�b�t�@���W�X�^�R�ɃZ�b�g
		}
		MV1SetUseOrigShader(TRUE);
		this->sea.DrawModel();
		MV1SetUseOrigShader(FALSE);
	}
	//��`��
	void sky_draw(void) {
		SetCameraNearFar(1000.f, 5000.f);
		SetFogEnable(FALSE);
		SetUseLighting(FALSE);
		{
			this->sky.DrawModel();
			DrawBillboard3D(this->sun_pos.get(), 0.5f, 0.5f, 100.f, 0.f, this->sun_pic.get(), TRUE);
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
			DrawPolygonIndexed3D_UseVertexBuffer(this->VerBuf, this->IndexBuf, this->cloud_pic.get(), TRUE);
		}
		SetUseLighting(TRUE);
		SetDrawAlphaTest(-1, 0);
	}
	void draw() {
		SetFogStartEnd(30000.0f, 60000.f);
		SetFogColor(128, 128, 128);
		{
			this->map_get().DrawModel();
		}
		//�C
		this->sea_draw();
		//�_
		this->cloud_draw();//2
	}
};
