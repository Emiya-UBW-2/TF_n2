#pragma once

#define NOMINMAX
#include <D3D11.h>
#include <array>
#include <fstream>
#include <memory>
#include <optional>
#include <vector>
#include "DXLib_ref/DXLib_ref.h"
constexpr auto veh_all = 3;//�Ԏ�
void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor, float scale = 1.f) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
	efh->scale = scale;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y(), std::hypot(efh->nor.x(), efh->nor.z())), atan2(-efh->nor.x(), -efh->nor.z()), 0);
		efh->handle.SetScale(efh->scale);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}

//�v���P
class Mainclass {
private:
	struct gun_frame {
		int type = 0;
		frames frame1;
		frames frame2;
		frames frame3;
		float xrad = 0.f, yrad = 0.f;
		std::string name;
		float load_time = 0.f;
		std::vector<std::string> useammo;
		uint16_t rounds = 0;
	};
	struct foot_frame {
		frames frame;
		EffectS gndsmkeffcs;
		float gndsmksize = 1.f;
	};
	struct Hit {		      /**/
		bool flug{ false };   /*�e���t���O*/
		int use{ 0 };	      /*�g�p�t���[��*/
		MV1 pic;	      /*�e�����f��*/
		VECTOR_ref pos;	      /*���W*/
		MATRIX_ref mat;	      /**/
	};								      /**/
public:
	//�e��
	class Ammos {
	public:
		std::string name_a;
		int16_t type_a = 0;
		float caliber_a = 0.f;
		float pene_a = 0.f;
		float speed_a = 0.f;
		uint16_t damage_a = 0;

		static void set_ammos(std::vector<Ammos>* Ammo_) {
			auto& a = *Ammo_;
			int mdata = FileRead_open("data/ammo/ammo.txt", FALSE);
			while (true) {
				a.resize(a.size() + 1);
				a.back().name_a = getparams::_str(mdata);
				a.back().type_a = uint16_t(getparams::_ulong(mdata)); //ap=0,he=1
				a.back().caliber_a = getparams::_float(mdata);
				a.back().pene_a = getparams::_float(mdata);
				a.back().speed_a = getparams::_float(mdata);
				a.back().damage_a = uint16_t(getparams::_ulong(mdata));
				if (getparams::get_str(mdata).find("end") != std::string::npos) {
					break;
				}
			}
			FileRead_close(mdata);
		}
	};
	//���p
	class Vehcs {
	public:
		//����
		std::string name;				  /**/
		MV1 obj, col;					  /**/
		VECTOR_ref minpos, maxpos;			  /**/
		std::vector<gun_frame> gunframe;			  /**/
		std::vector<foot_frame> wheelframe;			  /**/
		std::vector<foot_frame> wheelframe_nospring;		  /*�U���։�]*/
		uint16_t HP = 0;					  /**/
		std::vector<std::pair<size_t, float>> armer_mesh; /*���bID*/
		std::vector<size_t> space_mesh;			  /*���bID*/
		std::vector<size_t> module_mesh;		  /*���bID*/
		int camo_tex = 0;				  /**/
		std::vector<int> camog;				  /**/
		bool isfloat = false;			  /*�������ǂ���*/
		float down_in_water = 0.f;			  /*���ޔ���ӏ�*/
		float max_speed_limit = 0.f;			  /*�ō����x(km/h)*/
		float mid_speed_limit = 0.f;			  /*���s���x(km/h)*/
		float min_speed_limit = 0.f;			  /*�������x(km/h)*/
		float flont_speed_limit = 0.f;			  /*�O�i���x(km/h)*/
		float back_speed_limit = 0.f;			  /*��ޑ��x(km/h)*/
		float body_rad_limit = 0.f;			  /*���񑬓x(�x/�b)*/
		float turret_rad_limit = 0.f;			  /*�C���쓮���x(�x/�b)*/
		frames fps_view;//�R�b�N�s�b�g
		GraphHandle ui_pic;//�V���G�b�g
		int pic_x, pic_y;//�T�C�Y
		//���
		std::array<int, 4> square{ 0 };//���p�̎l��
		std::array<std::vector<frames>, 2> b2upsideframe; /*���я�*/
		std::array<std::vector<frames>, 2> b2downsideframe; /*���я�*/
		std::vector<frames> burner;//�A�t�^�[�o�[�i�[
		frames hook;//���̓t�b�N

		std::vector<frames> wire;
		std::vector<frames> catapult;
		//
		void into(const Vehcs& t) {
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->wheelframe_nospring.clear();
			for (auto& p : t.wheelframe_nospring) {
				this->wheelframe_nospring.resize(this->wheelframe_nospring.size() + 1);
				this->wheelframe_nospring.back().frame = p.frame;
			}
			this->name = t.name;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
			this->HP = t.HP;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->camo_tex = t.camo_tex;
			this->camog = t.camog;
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->max_speed_limit = t.max_speed_limit;
			this->mid_speed_limit = t.mid_speed_limit;
			this->min_speed_limit = t.min_speed_limit;
			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->square = t.square;
			this->b2upsideframe = t.b2upsideframe;
			this->b2downsideframe = t.b2downsideframe;
			this->burner = t.burner;
			this->hook = t.hook;
			this->fps_view = t.fps_view;

			this->wire = t.wire;
			this->catapult = t.catapult;

			this->ui_pic = t.ui_pic.Duplicate();
			this->pic_x = t.pic_x;
			this->pic_y = t.pic_y;
		}
		//���O�ǂݍ���
		static void set_vehicles_pre(const char* name, std::vector<Mainclass::Vehcs>* veh_, const bool& Async) {
			WIN32_FIND_DATA win32fdt;
			HANDLE hFind;
			hFind = FindFirstFile((std::string(name) + "*").c_str(), &win32fdt);
			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
						veh_->resize(veh_->size() + 1);
						veh_->back().name = win32fdt.cFileName;
					}
				} while (FindNextFile(hFind, &win32fdt));
			} //else{ return false; }
			FindClose(hFind);
			for (auto& t : *veh_) {
				MV1::Load(std::string(name) + t.name + "/model.mv1", &t.obj, Async);
				MV1::Load(std::string(name) + t.name + "/col.mv1", &t.col, Async);
				t.ui_pic = GraphHandle::Load(std::string(name) + t.name + "/pic.png");
			}
		}
		//���C���ǂݍ���
		template <size_t N>
		static void set_vehicles(std::array<std::vector<Mainclass::Vehcs>, N>* vehcs) {
			using namespace std::literals;
			//����
			for (auto& veh : (*vehcs)) {
				for (auto& t : veh) {
					//���e�X�g
					t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
					//
					GetGraphSize(t.ui_pic.get(), &t.pic_x, &t.pic_y);
				}
			}
			//�ŗL
			for (auto& t : (*vehcs)[0]) {
				t.down_in_water = 0.f;
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					auto p = t.obj.mesh_maxpos(i).y();
					if (t.down_in_water < p) {
						t.down_in_water = p;
					}
				}
				t.down_in_water /= 2.f;
				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("�]��", 0) != std::string::npos) {
						t.wheelframe.resize(t.wheelframe.size() + 1);
						t.wheelframe.back().frame = { i,t.obj.frame(i) };
					}
					else if ((p.find("��", 0) != std::string::npos) && (p.find("�]��", 0) == std::string::npos)) {
						t.wheelframe_nospring.resize(t.wheelframe_nospring.size() + 1);
						t.wheelframe_nospring.back().frame = { i,t.obj.frame(i) };
					}
					else if (p.find("����", 0) != std::string::npos) {
						t.gunframe.resize(t.gunframe.size() + 1);
						auto& b = t.gunframe.back();
						b.frame1 = { i,t.obj.frame(i) };
						auto p2 = t.obj.frame_parent(b.frame1.first);
						if (p2 >= 0) {
							b.frame1.second -= t.obj.frame(int(p2)); //�e�����鎞�����Ƃ�
						}
						if (t.obj.frame_child_num(b.frame1.first) > 0) {
							if (t.obj.frame_name(b.frame1.first + 1).find("�p", 0) != std::string::npos) {
								b.frame2 = { b.frame1.first + 1,t.obj.frame(b.frame1.first + 1) - t.obj.frame(b.frame1.first) };
								if (t.obj.frame_child_num(b.frame1.first) > 0) {
									b.frame3 = { b.frame2.first + 1,t.obj.frame(b.frame2.first + 1) - t.obj.frame(b.frame2.first) };
								}
								else {
									b.frame3.first = -1;
								}
							}
						}
						else {
							b.frame2.first = -1;
						}
					}
					else if (p.find("min", 0) != std::string::npos) {
						t.minpos = t.obj.frame(i);
					}
					else if (p.find("max", 0) != std::string::npos) {
						t.maxpos = t.obj.frame(i);
					}
					else if (p.find("�QD����", 0) != std::string::npos || p.find("2D����", 0) != std::string::npos) { //2D����
						t.b2upsideframe[0].clear();
						t.b2upsideframe[1].clear();
						for (int z = 0; z < t.obj.frame_child_num(i); z++) {
							if (t.obj.frame(i + 1 + z).x() > 0) {
								t.b2upsideframe[0].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
							else {
								t.b2upsideframe[1].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
						}
						std::sort(t.b2upsideframe[0].begin(), t.b2upsideframe[0].end(), [](const frames& x, const frames& y) { return x.second.z() < y.second.z(); }); //�\�[�g
						std::sort(t.b2upsideframe[1].begin(), t.b2upsideframe[1].end(), [](const frames& x, const frames& y) { return x.second.z() < y.second.z(); }); //�\�[�g
					}
					else if (p.find("���ѐݒu��", 0) != std::string::npos) { //2D����
						t.b2downsideframe[0].clear();
						t.b2downsideframe[1].clear();
						for (int z = 0; z < t.obj.frame_child_num(i); z++) {
							if (t.obj.frame(i + 1 + z).x() > 0) {
								t.b2downsideframe[0].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
							else {
								t.b2downsideframe[1].emplace_back(i + 1 + z, t.obj.frame(i + 1 + z));
							}
						}
					}
					else if (p.find("���_", 0) != std::string::npos) {
						t.fps_view.first = i;
						t.fps_view.second = t.obj.frame(t.fps_view.first);
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
				//���b
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) {
						t.armer_mesh.emplace_back(i, std::stof(getparams::getright(p.c_str())));//���b
					}
					else if (p.find("space", 0) != std::string::npos) {
						t.space_mesh.emplace_back(i);//��ԑ��b
					}
					else {
						t.module_mesh.emplace_back(i);//���W���[��
					}
				}
				//����
				{
					t.camo_tex = -1;
					for (int i = 0; i < MV1GetTextureNum(t.obj.get()); i++) {
						std::string p = MV1GetTextureName(t.obj.get(), i);
						if (p.find("b.", 0) != std::string::npos || p.find("B.", 0) != std::string::npos) {
							t.camo_tex = i;
							break;
						}
					}
					SetUseTransColor(FALSE);
					WIN32_FIND_DATA win32fdt;
					HANDLE hFind;
					hFind = FindFirstFile(("data/tank/"s + t.name + "/B*.jpg").c_str(), &win32fdt);
					if (hFind != INVALID_HANDLE_VALUE) {
						do {
							if (win32fdt.cFileName[0] != '.') {
								t.camog.resize(t.camog.size() + 1);
								t.camog.back() = MV1LoadTexture(("data/tank/"s + t.name + "/" + win32fdt.cFileName).c_str());
							}
						} while (FindNextFile(hFind, &win32fdt));
					} //else{ return false; }
					FindClose(hFind);
					SetUseTransColor(TRUE);
				}
				//data
				{
					int mdata = FileRead_open(("data/tank/" + t.name + "/data.txt").c_str(), FALSE);
					char mstr[64]; /*tank*/
					t.isfloat = getparams::_bool(mdata);
					t.flont_speed_limit = getparams::_float(mdata);
					t.back_speed_limit = getparams::_float(mdata);
					t.body_rad_limit = getparams::_float(mdata);
					t.turret_rad_limit = getparams::_float(mdata);
					t.HP = uint16_t(getparams::_ulong(mdata));
					FileRead_gets(mstr, 64, mdata);
					for (auto& g : t.gunframe) {
						g.name = getparams::getright(mstr);
						g.load_time = getparams::_float(mdata);
						g.rounds = uint16_t(getparams::_ulong(mdata));
						while (true) {
							FileRead_gets(mstr, 64, mdata);
							if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
								break;
							}
							g.useammo.emplace_back(getparams::getright(mstr));
						}
					}
					FileRead_close(mdata);
				}
			}
			for (auto& t : (*vehcs)[1]) {
				//
				t.down_in_water = 0.f;
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					auto p = t.obj.mesh_minpos(i).y();
					if (t.down_in_water > p) {
						t.down_in_water = p;
					}
				}
				//t.down_in_water /= 2.f;
				//�ő�ŏ����擾
				for (int i = 0; i < t.obj.mesh_num(); i++) {
					if (t.maxpos.x() < t.obj.mesh_maxpos(i).x()) {
						t.maxpos.x(t.obj.mesh_maxpos(i).x());
					}
					if (t.maxpos.z() < t.obj.mesh_maxpos(i).z()) {
						t.maxpos.z(t.obj.mesh_maxpos(i).z());
					}
					if (t.minpos.x() > t.obj.mesh_minpos(i).x()) {
						t.minpos.x(t.obj.mesh_minpos(i).x());
					}
					if (t.minpos.z() > t.obj.mesh_minpos(i).z()) {
						t.minpos.z(t.obj.mesh_minpos(i).z());
					}
				}
				//�t���[��
				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("�r", 0) != std::string::npos) {
						if (p.find("�n�b�`", 0) == std::string::npos) {
							t.wheelframe.resize(t.wheelframe.size() + 1);
							t.wheelframe.back().frame.first = i;
							t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);

							t.wheelframe_nospring.resize(t.wheelframe_nospring.size() + 1);
							t.wheelframe_nospring.back().frame.first = t.wheelframe.back().frame.first + 1;
							t.wheelframe_nospring.back().frame.second = t.obj.frame(t.wheelframe_nospring.back().frame.first) - t.wheelframe.back().frame.second;
						}
					}
					else if (p.find("����", 0) != std::string::npos) {
						t.gunframe.resize(t.gunframe.size() + 1);
						t.gunframe.back().frame1.first = i;
						t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
						auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
						if (p2 >= 0) {
							t.gunframe.back().frame1.second -= t.obj.frame(int(p2)); //�e�����鎞�����Ƃ�
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
					else if (p.find("�o�[�i�[", 0) != std::string::npos) {
						t.burner.resize(t.burner.size() + 1);
						t.burner.back().first = i;
						t.burner.back().second = t.obj.frame(t.burner.back().first);
					}
					else if (p.find("�t�b�N", 0) != std::string::npos) {
						t.hook.first = i;
						t.hook.second = t.obj.frame(t.hook.first);
					}
					else if (p.find("���_", 0) != std::string::npos) {
						t.fps_view.first = i;
						t.fps_view.second = t.obj.frame(t.fps_view.first);
					}
				}
				//���b�V��
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) { //���b
						t.armer_mesh.resize(t.armer_mesh.size() + 1);
						t.armer_mesh.back().first = i;
						t.armer_mesh.back().second = std::stof(getparams::getright(p.c_str())); //���b�l
					}
					else if (p.find("space", 0) != std::string::npos) {		    //��ԑ��b
						t.space_mesh.resize(t.space_mesh.size() + 1);
						t.space_mesh.back() = i;
					}
					else { //���W���[��
						t.module_mesh.resize(t.module_mesh.size() + 1);
						t.module_mesh.back() = i;
					}
				}
				//����
				{
					t.camo_tex = -1;
					for (int i = 0; i < MV1GetTextureNum(t.obj.get()); i++) {
						std::string p = MV1GetTextureName(t.obj.get(), i);
						if (p.find("b.", 0) != std::string::npos || p.find("B.", 0) != std::string::npos) {
							t.camo_tex = i;
							break;
						}
					}

					//t.camo.resize(t.camo.size() + 1);
					{
						SetUseTransColor(FALSE);
						WIN32_FIND_DATA win32fdt;
						HANDLE hFind;
						hFind = FindFirstFile(("data/plane/"s + t.name + "/B*.jpg").c_str(), &win32fdt);
						if (hFind != INVALID_HANDLE_VALUE) {
							do {
								if (win32fdt.cFileName[0] != '.') {
									t.camog.resize(t.camog.size() + 1);
									t.camog.back() = LoadGraph(("data/plane/"s + t.name + "/" + win32fdt.cFileName).c_str());
								}
							} while (FindNextFile(hFind, &win32fdt));
						} //else{ return false; }
						FindClose(hFind);
						SetUseTransColor(TRUE);
					}
				}
				//�f�[�^�擾
				{
					int mdata = FileRead_open(("data/plane/" + t.name + "/data.txt").c_str(), FALSE);
					char mstr[64]; /*tank*/
					t.isfloat = getparams::_bool(mdata);
					t.max_speed_limit = getparams::_float(mdata) / 3.6f;
					t.mid_speed_limit = getparams::_float(mdata) / 3.6f;
					t.min_speed_limit = getparams::_float(mdata) / 3.6f;
					t.body_rad_limit = getparams::_float(mdata);
					t.HP = uint16_t(getparams::_ulong(mdata));
					FileRead_gets(mstr, 64, mdata);
					for (auto& g : t.gunframe) {
						g.name = getparams::getright(mstr);
						g.load_time = getparams::_float(mdata);
						g.rounds = uint16_t(getparams::_ulong(mdata));
						while (true) {
							FileRead_gets(mstr, 64, mdata);
							if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
								break;
							}
							g.useammo.resize(g.useammo.size() + 1);
							g.useammo.back() = getparams::getright(mstr);
						}
					}
					FileRead_close(mdata);
				}
			}
			for (auto& t : (*vehcs)[2]) {

				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("ܲ԰", 0) != std::string::npos) {
						t.wire.resize(t.wire.size() + 1);
						t.wire.back().first = i;
						t.wire.back().second = t.obj.frame(t.wire.back().first);
					}
					else if (p.find("������", 0) != std::string::npos) {
						t.catapult.resize(t.catapult.size() + 1);
						t.catapult.back().first = i;
						t.catapult.back().second = t.obj.frame(t.catapult.back().first + 2) - t.obj.frame(t.catapult.back().first);
					}
				}
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) { //���b
						t.armer_mesh.resize(t.armer_mesh.size() + 1);
						t.armer_mesh.back().first = i;
						t.armer_mesh.back().second = std::stof(getparams::getright(p.c_str())); //���b�l
					}
					else if (p.find("space", 0) != std::string::npos) {		    //��ԑ��b
						t.space_mesh.resize(t.space_mesh.size() + 1);
						t.space_mesh.back() = i;
					}
					else { //���W���[��
						t.module_mesh.resize(t.module_mesh.size() + 1);
						t.module_mesh.back() = i;
					}
				}


				VECTOR_ref size;
				for (int i = 0; i < t.col.mesh_num(); i++) {
					VECTOR_ref sizetmp = t.col.mesh_maxpos(i) - t.col.mesh_minpos(i);
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

				/*
				for (int i = 0; i < t.col.mesh_num(); i++) {
					t.col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, i);
				}
				*/
				//t.col.SetupCollInfo(int(size.x() / 5.f), int(size.y() / 5.f), int(size.z() / 5.f), 0, 0);
				//
				//t.obj.SetFrameLocalMatrix(t.catapult[0].first + 2, MATRIX_ref::RotX(deg2rad(-75)) * MATRIX_ref::Mtrans(t.catapult[0].second));

								//�f�[�^�擾
				{
					int mdata = FileRead_open(("data/carrier/" + t.name + "/data.txt").c_str(), FALSE);
					char mstr[64]; /*tank*/
					t.isfloat = getparams::_bool(mdata);
					t.max_speed_limit = getparams::_float(mdata) / 3.6f;
					t.mid_speed_limit = getparams::_float(mdata) / 3.6f;
					t.min_speed_limit = getparams::_float(mdata) / 3.6f;
					t.body_rad_limit = getparams::_float(mdata);
					t.HP = uint16_t(getparams::_ulong(mdata));
					FileRead_gets(mstr, 64, mdata);
					for (auto& g : t.gunframe) {
						g.name = getparams::getright(mstr);
						g.load_time = getparams::_float(mdata);
						g.rounds = uint16_t(getparams::_ulong(mdata));
						while (true) {
							FileRead_gets(mstr, 64, mdata);
							if (std::string(mstr).find(("useammo" + std::to_string(g.useammo.size()))) == std::string::npos) {
								break;
							}
							g.useammo.resize(g.useammo.size() + 1);
							g.useammo.back() = getparams::getright(mstr);
						}
					}
					FileRead_close(mdata);
				}
			}
		}
	};
private:
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		unsigned int color = 0;
		Mainclass::Ammos spec;
		float yadd = 0.f;
		VECTOR_ref pos, repos, vec;
	};
	struct Guns {							      /**/
		size_t usebullet{};					      /*�g�p�e*/
		std::array<ammos, 64> bullet;				      /*�m�ۂ���e*/
		float loadcnt{ 0 };					      /*���Ă�J�E���^�[*/
		float fired{ 0.f };					      /*���ސ�*/
		int16_t rounds{ 0 };					      /*�e��*/
		gun_frame gun_info;						      /**/
		std::vector<Mainclass::Ammos> Spec;				      /**/
	};								      /**/
	struct ef_guns {
		EffectS first;
		ammos* second = nullptr;
		int cnt = -1;
	};
	typedef std::pair<size_t, float> pair_hit;
	class vehicles {
	public:
		Vehcs use_veh;							      /*�ŗL�l*/
		MV1 obj;							      /**/
		MV1 col;							      /**/
		bool hit_check = false;						      //�����蔻�����邩�`�F�b�N
		size_t use_id = 0;						      //�g�p����ԗ�(�@��)
		uint16_t HP = 0;						      /*�̗�*/
		VECTOR_ref pos;							      //�ԑ̍��W
		MATRIX_ref mat;							      //�ԑ̉�]�s��
		VECTOR_ref add;							      //�ԑ̉����x
		std::vector<Guns> Gun_;						      /**/
		float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;		      /**/
		float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f; /**/
		float yrad = 0.f, yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;	    /**/
		float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f; /**/
		std::vector<MV1_COLL_RESULT_POLY> hitres;			      /*�m��*/
		std::vector<int16_t> HP_m;					      /*���C�t*/
		std::array<Hit, 24> hit_obj;					      /*�e��*/
		size_t camo_sel = 0;						      /**/
		float wheel_Left = 0.f, wheel_Right = 0.f;			      //�]�։�]
		float wheel_Leftadd = 0.f, wheel_Rightadd = 0.f;		      //�]�։�]
		std::vector<pair_hit> hitssort;					      /*�t���[���ɓ�����������*/
	public:
		void reset() {
			//*
			this->obj.Dispose();
			this->col.Dispose();
			this->hit_check = false;
			this->HP = 0;
			this->speed_add = 0.f;
			this->speed_sub = 0.f;
			this->speed = 0.f;
			this->add = VGet(0.f, 0.f, 0.f);
			this->hitres.clear();
			this->HP_m.clear();
			this->wheel_Left = 0.f;
			this->wheel_Right = 0.f;
			this->wheel_Leftadd = 0.f;
			this->wheel_Rightadd = 0.f;
			this->xrad = 0.f;
			this->xradadd = 0.f;
			this->xradadd_left = 0.f;
			this->xradadd_right = 0.f;
			this->yrad = 0.f;
			this->yradadd = 0.f;
			this->yradadd_left = 0.f;
			this->yradadd_right = 0.f;
			this->zrad = 0.f;
			this->zradadd = 0.f;
			this->zradadd_left = 0.f;
			this->zradadd_right = 0.f;
			for (auto& h : this->hit_obj) {
				h.flug = false;
				h.use = 0;
				h.pic.Dispose();
				h.pos = VGet(0, 0, 0);
				h.mat = MGetIdent();
			}
			//this->use_veh;
			for (auto& g : this->Gun_) {
				g.fired = 0.f;
				g.loadcnt = 0.f;
				g.rounds = 0;
				g.usebullet = 0;
				//for (auto& a : g.bullet) {
				//}
				//g.gun_info;
				g.Spec.clear();
			}
			this->Gun_.clear();

			for (auto& h : this->hitssort) {
				h.first = 0;
				h.second = 0.f;
			}
			this->hitssort.clear();					      //�t���[���ɓ�����������
			//*/
		}
	};
public:
	//�}�b�v
	struct treePats {
		MV1 obj, obj_far;
		MATRIX_ref mat;
		VECTOR_ref pos;
		bool fall_flag = false;
		VECTOR_ref fall_vec;
		float fall_rad = 0.f;
	};
	//player
	typedef std::pair<int, float> p_animes;
	class Chara {
	public:
		//====================================================
		size_t id = 0;			     /**/
		std::array<EffectS, efs_user> effcs; /*effect*/
		std::array<ef_guns, 8> effcs_missile; /*effect*/
		std::array<ef_guns, 12> effcs_gun;    /*effect*/
		size_t missile_effcnt = 0;
		size_t gun_effcnt = 0;

		//����֘A//==================================================
		std::array<bool, 15> key{ false };    //�L�[
		float view_xrad = 0.f, view_yrad = 0.f; //�C������p�x�N�g��
		//���//==================================================
		int hitbuf = 0;		       /*�g�p�e��*/
		//��s�@//==================================================
		p_animes p_anime_geardown;		    //�ԗփA�j���[�V����
		switchs changegear; //�M�A�A�b�v�X�C�b�`
		switchs landing; //���̓t�b�N�X�C�b�`
		float p_landing_per = 0.f;		    //���̓t�b�N
		std::array<p_animes, 6> p_animes_rudder;      //���_�[�A�j���[�V����
		std::vector<frames> p_burner;		    //�o�[�i�[
		//���ʍ�//==================================================
		vehicles vehicle;

		//�Z�b�g
		template <size_t N>
		void set_human(const std::array<std::vector<Mainclass::Vehcs>, N>& vehcs, const std::vector<Ammos>& Ammo_, const MV1& hit_pic) {
			auto& c = *this;
			{
				std::fill(c.key.begin(), c.key.end(), false); //����
				fill_id(c.effcs);			      //�G�t�F�N�g
				//����
				{
					auto& veh = c.vehicle;
					{
						veh.reset();
						veh.use_id = std::min<size_t>(veh.use_id, vehcs[1].size() - 1);
						veh.use_veh.into(vehcs[1][veh.use_id]);
						veh.obj = vehcs[1][veh.use_id].obj.Duplicate();
						veh.col = vehcs[1][veh.use_id].col.Duplicate();
						//�R���W����
						for (int j = 0; j < veh.col.mesh_num(); j++) {
							veh.col.SetupCollInfo(8, 8, 8, -1, j);
						}
						veh.hitres.resize(veh.col.mesh_num());   //���W���[�����Ƃ̓����蔻�茋�ʂ��m��
						veh.hitssort.resize(veh.col.mesh_num()); //���W���[�����Ƃ̓����蔻�菇�����m��
						//�e��
						for (auto& h : veh.hit_obj) {
							h.flug = false;
							h.pic = hit_pic.Duplicate();
							h.use = 0;
							h.mat = MGetIdent();
							h.pos = VGet(0.f, 0.f, 0.f);
						}
						for (int j = 0; j < veh.obj.material_num(); ++j) {
							MV1SetMaterialSpcColor(veh.obj.get(), j, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
							MV1SetMaterialSpcPower(veh.obj.get(), j, 50.0f);
						}
						//�C
						{
							veh.Gun_.resize(veh.use_veh.gunframe.size());
							for (int j = 0; j < veh.Gun_.size(); j++) {
								auto& g = veh.Gun_[j];
								g.gun_info = veh.use_veh.gunframe[j];
								g.rounds = g.gun_info.rounds;
								//�g�p�C�e
								g.Spec.resize(g.Spec.size() + 1);
								for (auto& pa : Ammo_) {
									if (pa.name_a.find(g.gun_info.useammo[0]) != std::string::npos) {
										g.Spec.back() = pa;
										break;
									}
								}
								for (auto& p : g.bullet) {
									p.color = GetColor(255, 255, 172);
									p.spec = g.Spec[0];
								}
							}
						}
						//�q�b�g�|�C���g
						veh.HP = veh.use_veh.HP;
						//���W���[���ϋv
						veh.HP_m.resize(veh.col.mesh_num());
						for (auto& h : veh.HP_m) {
							h = 100;
						}
						//����
						if (veh.use_veh.camog.size() > 0) {
							veh.camo_sel %= veh.use_veh.camog.size();
							//GraphBlend(MV1GetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex), veh.use_veh.camog[veh.camo_sel], 255, DX_GRAPH_BLEND_NORMAL);
							MV1SetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex, veh.use_veh.camog[veh.camo_sel], FALSE);
						}
					}
				}
				//��s�@
				{
					auto& veh = c.vehicle;
					{
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
					//�G�t�F�N�g
					for (auto& be : veh.use_veh.burner) {
						c.p_burner.emplace_back(be);
					}
					c.changegear.first = true;
					c.changegear.second = 2;
					c.landing.first = false;
					c.landing.second = 2;
				}
			}
		}
		//�e��
		bool get_reco(std::vector<Chara>& tgts, ammos& c) {
			if (c.flug) {
				bool is_hit;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//�������g�͏Ȃ�
					if (this->id == t.id) {
						continue;
					}
					//�Ƃ肠���������������ǂ����T��
					is_hit = false;
					{
						auto& veh = t.vehicle;
						//���W���[��
						for (auto& m : veh.use_veh.module_mesh) {
							veh.hitres[m] = veh.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh.hitres[m].HitFlag) {
								veh.hitssort[m] = pair_hit(m, (c.repos - veh.hitres[m].HitPosition).size());
								is_hit = true;
							}
							else {
								veh.hitssort[m] = pair_hit(m, (std::numeric_limits<float>::max)());
							}
						}
						//��ԑ��b
						for (auto& m : veh.use_veh.space_mesh) {
							veh.hitres[m] = veh.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh.hitres[m].HitFlag) {
								veh.hitssort[m] = pair_hit(m, (c.repos - veh.hitres[m].HitPosition).size());
								is_hit = true;
							}
							else {
								veh.hitssort[m] = pair_hit(m, (std::numeric_limits<float>::max)());
							}
						}
						//���b
						for (auto& m : veh.use_veh.armer_mesh) {
							veh.hitres[m.first] = veh.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
							if (veh.hitres[m.first].HitFlag) {
								veh.hitssort[m.first] = pair_hit(m.first, (c.repos - veh.hitres[m.first].HitPosition).size());
								is_hit = true;
							}
							else {
								veh.hitssort[m.first] = pair_hit(m.first, (std::numeric_limits<float>::max)());
							}
						}
						//�������ĂȂ�
						if (!is_hit) {
							continue;
						}
						//�����蔻����߂����Ƀ\�[�g
						std::sort(veh.hitssort.begin(), veh.hitssort.end(), [](const pair_hit& x, const pair_hit& y) { return x.second < y.second; });
						//�_���[�W�ʂɓ͂��܂Ŕ���
						for (auto& tt : veh.hitssort) {
							//���b�ʂɓ�����Ȃ������Ȃ�X���[
							if (tt.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							switch (c.spec.type_a) {
							case 0: //AP
								//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
								for (auto& a : veh.use_veh.armer_mesh) {
									if (tt.first == a.first) {
										hitnear = tt.first;
										//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
										{
											VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
											VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//�ђ�
												//t.id;
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
												//���j���G�t�F�N�g
												if (veh.HP == 0) {
													set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
												}
												//�e����
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//�e��
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//�͂���
												//�e����
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												c.spec.pene_a /= 2.0f;
												//�e��
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//�e���̃Z�b�g
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
											}
										}
										break;
									}
								}
								if (hitnear.has_value()) {
									break;
								}
								//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�ѓO�͂�1/2��
								for (auto& a : veh.use_veh.space_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
									}
								}

								break;
							case 1: //HE
								//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
								for (auto& a : veh.use_veh.armer_mesh) {
									if (tt.first == a.first) {
										hitnear = tt.first;
										//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
										{
											VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
											VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//�ђ�
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
												//���j���G�t�F�N�g
												if (veh.HP == 0) {
													set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
												}
												//�e����
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//�e��
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//��������
												//�e����
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//�e��
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//�e���̃Z�b�g
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
											}
										}
										break;
									}
								}
								if (hitnear.has_value()) {
									break;
								}
								//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�e�Ȃ�
								for (auto& a : veh.use_veh.space_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//��������
										c.flug = false;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//��������
										c.flug = false;
									}
								}
								break;
							case 2: //�~�T�C��
								//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
								for (auto& a : veh.use_veh.armer_mesh) {
									if (tt.first == a.first) {
										hitnear = tt.first;
										//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
										{
											VECTOR_ref normal = veh.hitres[hitnear.value()].Normal;
											VECTOR_ref position = veh.hitres[hitnear.value()].HitPosition;
											if (c.spec.pene_a > a.second * (1.0f / std::abs(c.vec.Norm().dot(normal)))) {
												//�ђ�
												veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //

												veh.HP = std::max<int16_t>(veh.HP - c.spec.damage_a, 0); //
												//���j���G�t�F�N�g
												if (veh.HP == 0) {
													set_effect(&t.effcs[ef_bomb], veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
												}
												//�e����
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//�e��
												veh.hit_obj[t.hitbuf].use = 0;
											}
											else {
												//��������
												//�e����
												c.flug = false;
												c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
												c.vec = c.vec.Norm();
												c.pos = c.vec * (0.1f) + position;
												//�e��
												veh.hit_obj[t.hitbuf].use = 1;
											}
											if (c.spec.caliber_a >= 0.020f) {
												set_effect(&this->effcs[ef_reco], c.pos, normal);
											}
											else {
												set_effect(&this->effcs[ef_reco2], c.pos, normal);
											}

											//�e���̃Z�b�g
											{
												float asize = c.spec.caliber_a * 100.f;
												auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
												auto y_vec = MATRIX_ref::Vtrans(normal, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
												auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

												veh.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
												veh.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh.pos, veh.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
												veh.hit_obj[t.hitbuf].flug = true;
												++t.hitbuf %= veh.hit_obj.size();
											}
										}
										break;
									}
								}
								if (hitnear.has_value()) {
									break;
								}
								//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�e�Ȃ�
								for (auto& a : veh.use_veh.space_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//��������
										c.flug = false;
									}
								}
								for (auto& a : veh.use_veh.module_mesh) {
									if (tt.first == a) {
										if (c.spec.caliber_a >= 0.020f) {
											set_effect(&this->effcs[ef_reco], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										else {
											set_effect(&this->effcs[ef_reco2], VECTOR_ref(veh.hitres[tt.first].HitPosition) + VECTOR_ref(veh.hitres[tt.first].Normal) * (0.1f), veh.hitres[tt.first].Normal);
										}
										veh.HP_m[tt.first] = std::max<int16_t>(veh.HP_m[tt.first] - 30, 0); //
										//��������
										c.flug = false;
									}
								}
								break;
							default:
								break;
							}
						}
					}
					if (hitnear.has_value())
						break;
				}
				return (hitnear.has_value());
			}
			return false;
		}
	};
	//
};
//