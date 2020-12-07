#pragma once

#define NOMINMAX
#include <D3D11.h>
#include <array>
#include<iostream>
#include <fstream>
#include <memory>
#include <optional>
#include <vector>
#include "DXLib_ref/DXLib_ref.h"

#define FRAME_RATE 90.f
#define ADS 2

enum Effect {
	ef_fire = 0, //���C��
	ef_reco = 1, //����a���e
	ef_reco2 = 2, //�����a���e
	ef_gndhit = 3, //����a���e
	ef_gndhit2 = 4, //�����a���e
	ef_bomb = 5, //���j����
	ef_smoke1 = 6, //�~�T�C����
	ef_smoke2 = 7, //�e�̋O��
	ef_gndsmoke = 8,//�n�ʂ̋O��
	ef_size = 9
};

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
	};
	struct Hit {		      /**/
		bool flug{ false };   /*�e���t���O*/
		int use{ 0 };	      /*�g�p�t���[��*/
		MV1 pic;	      /*�e�����f��*/
		VECTOR_ref pos;	      /*���W*/
		MATRIX_ref mat;	      /**/
		void clear() {
			this->flug = false;
			this->use = 0;
			this->pic.Dispose();
			this->pos = VGet(0, 0, 0);
			this->mat.clear();
		}
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
		static void set_vehicles(std::vector<Mainclass::Vehcs>* vehcs) {
			using namespace std::literals;
			//����
			for (auto& t : *vehcs) {
				//���e�X�g
				t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				//
				GetGraphSize(t.ui_pic.get(), &t.pic_x, &t.pic_y);
			}
			//�ŗL
			for (auto& t : (*vehcs)) {
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
		}
	};
public:
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float count = 0.f;
		unsigned int color = 0;
		Mainclass::Ammos spec;
		float yadd = 0.f;
		VECTOR_ref pos, repos, vec;
	};
	struct ef_guns {
		EffectS first;
		ammos* second = nullptr;
		bool n_l;
		bool flug;
		float count = -1.f;
	};
	struct CAMS {
		DXDraw::cam_info cam;
		int Rot = 0;//
	};
private:
	class Guns {							      /**/
	public:
		size_t usebullet{};					      /*�g�p�e*/
		std::array<ammos, 64> bullet;				      /*�m�ۂ���e*/
		float loadcnt{ 0 };					      /*���Ă�J�E���^�[*/
		float fired{ 0.f };					      /*���ސ�*/
		int16_t rounds{ 0 };					      /*�e��*/
		gun_frame gun_info;						      /**/
		std::vector<Mainclass::Ammos> Spec;				      /**/
		void clear() {
			this->usebullet = 0;
			this->loadcnt = 0.f;
			this->fired = 0.f;
			this->rounds = 0;
			this->Spec.clear();
		}
	};								      /**/
	class pair_hit {							      /**/
	public:
		size_t first = 0;
		float second = 0.f;
		void clear() {
			this->first = 0;
			this->second = 0.f;
		}
	};
	class vehicles {
	public:
		Vehcs use_veh;							      /*�ŗL�l*/
		MV1 obj;							      /**/
		MV1 col;							      /**/
		bool hit_check = false;						      //�����蔻�����邩�`�F�b�N
		size_t use_id = 0;						      //�g�p����ԗ�(�@��)
		uint16_t HP = 0;						      /*�̗�*/
		uint16_t KILL = 0;						      /*�̗�*/
		int KILL_ID = -1;						      /*�̗�*/
		uint16_t DEATH = 0;						      /*�̗�*/
		int DEATH_ID = -1;						      /*�̗�*/
		VECTOR_ref pos;							      //�ԑ̍��W
		MATRIX_ref mat;							      //�ԑ̉�]�s��
		MATRIX_ref mat_start;					      //�ԑ̉�]�s��(�����z�u)
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
		void init() {
			this->obj.Dispose();
			this->col.Dispose();
			this->hit_check = false;
			this->HP = 0;
			this->KILL = 0;						      /*�̗�*/
			this->KILL_ID = -1;						      /*�̗�*/
			this->DEATH = 0;						      /*�̗�*/
			this->DEATH_ID = -1;						      /*�̗�*/
			this->speed_add = 0.f;
			this->speed_sub = 0.f;
			this->speed = 0.f;
			this->add.clear();
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
			for (auto& h : this->hit_obj) { h.clear(); }
			for (auto& cg : this->Gun_) { cg.clear(); }
			this->Gun_.clear();
			for (auto& h : this->hitssort) { h.clear(); }
			this->hitssort.clear();
		}
		void spawn(const VECTOR_ref& pos_, const MATRIX_ref& mat_) {
			this->HP = this->use_veh.HP;
			this->pos = pos_;
			this->mat = mat_;
			this->mat_start = this->mat;
			this->xradadd_right = 0.f;
			this->xradadd_left = 0.f;
			this->yradadd_left = 0.f;
			this->yradadd_right = 0.f;
			this->zradadd_right = 0.f;
			this->zradadd_left = 0.f;
			this->speed_add = 0.f;
			this->speed_sub = 0.f;
			this->speed = 0.f;
			this->add.clear();
		}
	};
public:
	//�}�b�v
	struct treePats {
		MV1 obj, obj_far;
		MATRIX_ref mat;
		VECTOR_ref pos;
	};
	//player
	typedef std::pair<int, float> p_animes;
	class Chara {
	public:
		class sendstat {

		public:
			MATRIX_ref v_mat;
			VECTOR_ref pos;
			MATRIX_ref mat;
			std::vector<Guns> Gun_;						      /**/
			float speed;
			struct eff_buf {
				bool flug{ false };				 /**/
				VECTOR_ref pos;					 /**/
				VECTOR_ref nor;					 /**/
				float scale = 1.f;				 /**/
			};
			std::array<eff_buf, ef_size> effcs_; /*effect*/
			std::array<ef_guns, 8> effcs_missile_; /*effect*/
			std::array<ef_guns, 12> effcs_gun_;    /*effect*/

			std::array<float, 3> gndsmkeffcs_; /*effect*/

			//���̂��v��
			sendstat(void) {
			}
			//���̂��v��
			sendstat(const sendstat& p) {
			}

			void get_data(Chara& data) {
				auto& veh = data.vehicle;

				this->speed = veh.speed;
				this->v_mat = veh.obj.GetMatrix();
				this->mat = veh.mat;
				this->pos = veh.pos;
				this->Gun_ = veh.Gun_;
				for (int i = 0; i < ef_size; i++) {
					this->effcs_[i].flug = data.effcs[i].flug;
					this->effcs_[i].pos = data.effcs[i].pos;
					this->effcs_[i].nor = data.effcs[i].nor;
					this->effcs_[i].scale = data.effcs[i].scale;
				}
				for (int i = 0; i < 8; i++) {
					effcs_missile_[i].first.flug = data.effcs_missile[i].first.flug;
					effcs_missile_[i].first.pos = data.effcs_missile[i].first.pos;
					effcs_missile_[i].first.nor = data.effcs_missile[i].first.nor;
					effcs_missile_[i].first.scale = data.effcs_missile[i].first.scale;
					effcs_missile_[i].flug = data.effcs_missile[i].flug;
					effcs_missile_[i].n_l = data.effcs_missile[i].n_l;
					effcs_missile_[i].count = data.effcs_missile[i].count;
				}
				for (int i = 0; i < 12; i++) {
					effcs_gun_[i].first.flug = data.effcs_gun[i].first.flug;
					effcs_gun_[i].first.pos = data.effcs_gun[i].first.pos;
					effcs_gun_[i].first.nor = data.effcs_gun[i].first.nor;
					effcs_gun_[i].first.scale = data.effcs_gun[i].first.scale;
					effcs_gun_[i].flug = data.effcs_gun[i].flug;
					effcs_gun_[i].n_l = data.effcs_gun[i].n_l;
					effcs_gun_[i].count = data.effcs_gun[i].count;
				}
				{
					int i = 0;
					for (auto& t : veh.use_veh.wheelframe) {
						gndsmkeffcs_[i] = t.gndsmkeffcs.scale;
						i++;
					}
				}
			}
			void put_data(Chara& data) {
				auto& veh = data.vehicle;

				veh.speed = this->speed;
				veh.obj.SetMatrix(this->v_mat);
				veh.mat = this->mat;
				veh.pos = this->pos;
				{
					veh.Gun_ = this->Gun_;
					int i = 0, j = 0;
					for (auto& p : veh.Gun_) {
						auto& t = this->Gun_[i];
						j = 0;
						for (auto& a : p.bullet) {
							auto& b = t.bullet[j];
							a.flug = b.flug;
							a.pos = b.pos;
							a.repos = b.repos;
							a.spec.caliber_a = b.spec.caliber_a;
							a.color = b.color;
							j++;
						}
						i++;
					}
				}
				for (int i = 0; i < ef_size; i++) {
					data.effcs[i].flug = this->effcs_[i].flug;
					data.effcs[i].pos = this->effcs_[i].pos;
					data.effcs[i].nor = this->effcs_[i].nor;
					data.effcs[i].scale = this->effcs_[i].scale;
				}
				for (int i = 0; i < 8; i++) {
					data.effcs_missile[i].first.flug = effcs_missile_[i].first.flug;
					data.effcs_missile[i].first.pos = effcs_missile_[i].first.pos;
					data.effcs_missile[i].first.nor = effcs_missile_[i].first.nor;
					data.effcs_missile[i].first.scale = effcs_missile_[i].first.scale;
					data.effcs_missile[i].flug = effcs_missile_[i].flug;
					data.effcs_missile[i].n_l = effcs_missile_[i].n_l;
					data.effcs_missile[i].count = effcs_missile_[i].count;
				}
				for (int i = 0; i < 12; i++) {
					data.effcs_gun[i].first.flug = effcs_gun_[i].first.flug;
					data.effcs_gun[i].first.pos = effcs_gun_[i].first.pos;
					data.effcs_gun[i].first.nor = effcs_gun_[i].first.nor;
					data.effcs_gun[i].first.scale = effcs_gun_[i].first.scale;
					data.effcs_gun[i].flug = effcs_gun_[i].flug;
					data.effcs_gun[i].n_l = effcs_gun_[i].n_l;
					data.effcs_gun[i].count = effcs_gun_[i].count;
				}
				{
					int i = 0;
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.scale = gndsmkeffcs_[i];
						i++;
					}
				}
			}

			void write(std::ofstream& fout) {
				{
					fout.write((char *)&this->speed, sizeof(this->speed));
					fout.write((char *)&this->v_mat, sizeof(this->v_mat));
					fout.write((char *)&this->mat, sizeof(this->mat));
					fout.write((char *)&this->pos, sizeof(this->pos));
					for (auto& p : this->Gun_) {
						for (auto& a : p.bullet) {
							fout.write((char *)&a.flug, sizeof(a.flug));
							fout.write((char *)&a.pos, sizeof(a.pos));
							fout.write((char *)&a.repos, sizeof(a.repos));
							fout.write((char *)&a.spec.caliber_a, sizeof(a.spec.caliber_a));
							fout.write((char *)&a.color, sizeof(a.color));
						}
					}
					for (auto& e : this->effcs_) {
						fout.write((char *)&e.flug, sizeof(e.flug));
						fout.write((char *)&e.pos, sizeof(e.pos));
						fout.write((char *)&e.nor, sizeof(e.nor));
						fout.write((char *)&e.scale, sizeof(e.scale));
					}
					for (auto& e : this->effcs_missile_) {
						fout.write((char *)&e.first.flug, sizeof(e.first.flug));
						fout.write((char *)&e.first.pos, sizeof(e.first.pos));
						fout.write((char *)&e.first.nor, sizeof(e.first.nor));
						fout.write((char *)&e.first.scale, sizeof(e.first.scale));
						fout.write((char *)&e.flug, sizeof(e.flug));
						fout.write((char *)&e.n_l, sizeof(e.n_l));
						fout.write((char *)&e.count, sizeof(e.count));
					}
					for (auto& e : this->effcs_gun_) {
						fout.write((char *)&e.first.flug, sizeof(e.first.flug));
						fout.write((char *)&e.first.pos, sizeof(e.first.pos));
						fout.write((char *)&e.first.nor, sizeof(e.first.nor));
						fout.write((char *)&e.first.scale, sizeof(e.first.scale));
						fout.write((char *)&e.flug, sizeof(e.flug));
						fout.write((char *)&e.n_l, sizeof(e.n_l));
						fout.write((char *)&e.count, sizeof(e.count));
					}
					for (auto& e : this->gndsmkeffcs_) {
						fout.write((char *)&e, sizeof(e));
					}
				}
			}

			void read(std::ifstream& fout) {
				{
					fout.read((char *)&this->speed, sizeof(this->speed));
					fout.read((char *)&this->v_mat, sizeof(this->v_mat));
					fout.read((char *)&this->mat, sizeof(this->mat));
					fout.read((char *)&this->pos, sizeof(this->pos));
					this->Gun_.clear();
					this->Gun_.resize(5);
					for (auto& p : this->Gun_) {
						for (auto& a : p.bullet) {
							fout.read((char *)&a.flug, sizeof(a.flug));
							fout.read((char *)&a.pos, sizeof(a.pos));
							fout.read((char *)&a.repos, sizeof(a.repos));
							fout.read((char *)&a.spec.caliber_a, sizeof(a.spec.caliber_a));
							fout.read((char *)&a.color, sizeof(a.color));
						}
					}
					for (auto& e : this->effcs_) {
						fout.read((char *)&e.flug, sizeof(e.flug));
						fout.read((char *)&e.pos, sizeof(e.pos));
						fout.read((char *)&e.nor, sizeof(e.nor));
						fout.read((char *)&e.scale, sizeof(e.scale));
					}
					for (auto& e : this->effcs_missile_) {
						fout.read((char *)&e.first.flug, sizeof(e.first.flug));
						fout.read((char *)&e.first.pos, sizeof(e.first.pos));
						fout.read((char *)&e.first.nor, sizeof(e.first.nor));
						fout.read((char *)&e.first.scale, sizeof(e.first.scale));
						fout.read((char *)&e.flug, sizeof(e.flug));
						fout.read((char *)&e.n_l, sizeof(e.n_l));
						fout.read((char *)&e.count, sizeof(e.count));
					}
					for (auto& e : this->effcs_gun_) {
						fout.read((char *)&e.first.flug, sizeof(e.first.flug));
						fout.read((char *)&e.first.pos, sizeof(e.first.pos));
						fout.read((char *)&e.first.nor, sizeof(e.first.nor));
						fout.read((char *)&e.first.scale, sizeof(e.first.scale));
						fout.read((char *)&e.flug, sizeof(e.flug));
						fout.read((char *)&e.n_l, sizeof(e.n_l));
						fout.read((char *)&e.count, sizeof(e.count));
					}
					for (auto& e : this->gndsmkeffcs_) {
						fout.read((char *)&e, sizeof(e));
					}
				}
			}
		};
		std::list<sendstat> rep;
		//====================================================
		std::array<EffectS, ef_size> effcs; /*effect*/
		std::array<ef_guns, 8> effcs_missile; /*effect*/
		std::array<ef_guns, 12> effcs_gun;    /*effect*/
		size_t missile_effcnt = 0;
		size_t gun_effcnt = 0;

		//����֘A//==================================================
		std::array<bool, 18> key{ false };    //�L�[
		float view_xrad = 0.f, view_yrad = 0.f; //�C������p�x�N�g��
		//���//==================================================
		int hitbuf = 0;		       /*�g�p�e��*/
		//��s�@//==================================================
		p_animes p_anime_geardown;		    //�ԗփA�j���[�V����
		switchs changegear; //�M�A�A�b�v�X�C�b�`
		float p_landing_per = 0.f;		    //���̓t�b�N
		std::array<p_animes, 6> p_animes_rudder;      //���_�[�A�j���[�V����
		std::vector<frames> p_burner;		    //�o�[�i�[
		//���ʍ�//==================================================
		vehicles vehicle;
		VECTOR_ref winpos;
		SoundHandle se_cockpit;
		SoundHandle se_gun;
		SoundHandle se_hit;
		//�Z�b�g
		void set_human(const std::vector<Mainclass::Vehcs>& vehcs, const std::vector<Ammos>& Ammo_, const MV1& hit_pic) {
			auto& c = *this;
			{
				std::fill(c.key.begin(), c.key.end(), false); //����
				//����
				{
					auto& veh = c.vehicle;
					{
						veh.init();
						veh.use_id = std::min<size_t>(veh.use_id, vehcs.size() - 1);
						veh.use_veh.into(vehcs[veh.use_id]);
						veh.obj = vehcs[veh.use_id].obj.Duplicate();
						veh.col = vehcs[veh.use_id].col.Duplicate();
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
							h.mat.clear();
							h.pos.clear();
						}
						for (int j = 0; j < veh.obj.material_num(); ++j) {
							MV1SetMaterialSpcColor(veh.obj.get(), j, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
							MV1SetMaterialSpcPower(veh.obj.get(), j, 50.0f);
						}
						//�C
						veh.Gun_.resize(veh.use_veh.gunframe.size());
						for (auto& cg : veh.Gun_) {
							size_t index = &cg - &veh.Gun_[0];
							cg.gun_info = veh.use_veh.gunframe[index];
							cg.rounds = cg.gun_info.rounds;
							//�g�p�C�e
							cg.Spec.resize(cg.Spec.size() + 1);
							for (auto& pa : Ammo_) {
								if (pa.name_a.find(cg.gun_info.useammo[0]) != std::string::npos) {
									cg.Spec.back() = pa;
									break;
								}
							}
							for (auto& p : cg.bullet) {
								p.color = GetColor(255, 255, 172);
								p.spec = cg.Spec[0];
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
				}
			}
		}
		//�e��
		bool get_reco(std::vector<Chara>& tgts, ammos& c) {
			if (c.flug) {
				bool is_hit = false;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//�������g�͏Ȃ�
					if (this == &t) {
						continue;
					}
					//�Ƃ肠���������������ǂ����T��
					is_hit = false;
					{
						auto& veh_t = t.vehicle;
						//���W���[��
						for (auto& m : veh_t.use_veh.module_mesh) {
							veh_t.hitres[m] = veh_t.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh_t.hitres[m].HitFlag) {
								veh_t.hitssort[m] = { m, (c.repos - veh_t.hitres[m].HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hitssort[m] = { m, (std::numeric_limits<float>::max)() };
							}
						}
						//��ԑ��b
						for (auto& m : veh_t.use_veh.space_mesh) {
							veh_t.hitres[m] = veh_t.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh_t.hitres[m].HitFlag) {
								veh_t.hitssort[m] = { m, (c.repos - veh_t.hitres[m].HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hitssort[m] = { m, (std::numeric_limits<float>::max)() };
							}
						}
						//���b
						for (auto& m : veh_t.use_veh.armer_mesh) {
							veh_t.hitres[m.first] = veh_t.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
							if (veh_t.hitres[m.first].HitFlag) {
								veh_t.hitssort[m.first] = { m.first, (c.repos - veh_t.hitres[m.first].HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hitssort[m.first] = { m.first, (std::numeric_limits<float>::max)() };
							}
						}
						//�������ĂȂ�
						if (!is_hit) {
							continue;
						}
						//
						t.se_hit.play(DX_PLAYTYPE_BACK,TRUE);
						//�����蔻����߂����Ƀ\�[�g
						std::sort(veh_t.hitssort.begin(), veh_t.hitssort.end(), [](const pair_hit& x, const pair_hit& y) { return x.second < y.second; });
						//�_���[�W�ʂɓ͂��܂Ŕ���
						for (auto& tt : veh_t.hitssort) {
							//���b�ʂɓ�����Ȃ������Ȃ�X���[
							if (tt.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							//���������画��
							for (auto& a : veh_t.use_veh.armer_mesh) {
								if (tt.first != a.first) {
									continue;
								}
								hitnear = tt.first;
								//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
								{
									VECTOR_ref normal = veh_t.hitres[hitnear.value()].Normal;
									VECTOR_ref position = veh_t.hitres[hitnear.value()].HitPosition;

									VECTOR_ref vec_t = c.vec;
									//�e����
									c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
									c.vec = c.vec.Norm();
									c.pos = c.vec * (0.1f) + position;
									//�ђ�
									if (c.spec.pene_a > a.second * (1.0f / std::abs(vec_t.Norm().dot(normal)))) {
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										veh_t.HP = std::max<int16_t>(veh_t.HP - c.spec.damage_a, 0); //
										//���j���G�t�F�N�g
										if (veh_t.HP == 0) {
											this->vehicle.KILL++;
											this->vehicle.KILL_ID = (int)(&t - &tgts[0]);
											veh_t.DEATH++;
											veh_t.DEATH_ID = (int)(this - &tgts[0]);
											t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
										}
										//�e����
										c.flug = false;
										//�e��
										veh_t.hit_obj[t.hitbuf].use = 0;
									}
									//��ђ�
									else {
										switch (c.spec.type_a) {
										case 0: //AP
											//�͂���
											c.spec.pene_a /= 2.0f;
											break;
										case 1: //HE
											//��������
											c.flug = false;
											break;
										case 2: //�~�T�C��
											//��������
											c.flug = false;
											break;
										default:
											break;
										}
										//�e��
										veh_t.hit_obj[t.hitbuf].use = 1;
									}
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(c.pos, normal);
									}
									else {
										this->effcs[ef_reco2].set(c.pos, normal);
									}
									//�e���̃Z�b�g
									{
										float asize = c.spec.caliber_a * 100.f;
										auto scale = VGet(asize / std::abs(c.vec.Norm().dot(normal)), asize, asize);
										auto y_vec = MATRIX_ref::Vtrans(normal, veh_t.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));
										auto z_vec = MATRIX_ref::Vtrans(normal.cross(c.vec), veh_t.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180)));

										veh_t.hit_obj[t.hitbuf].mat = MATRIX_ref::Scale(scale)* MATRIX_ref::Axis1(y_vec.cross(z_vec), y_vec, z_vec);
										veh_t.hit_obj[t.hitbuf].pos = MATRIX_ref::Vtrans(position - veh_t.pos, veh_t.mat.Inverse() * MATRIX_ref::RotY(deg2rad(180))) + y_vec * 0.02f;
										veh_t.hit_obj[t.hitbuf].flug = true;
										++t.hitbuf %= veh_t.hit_obj.size();
									}
								}
							}
							//�͂����ꂽ
							if (hitnear.has_value()) {
								continue;
							}
							//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�ѓO�͂�1/2��
							for (auto& a : veh_t.use_veh.space_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(veh_t.hitres[tt.first].HitPosition) + VECTOR_ref(veh_t.hitres[tt.first].Normal) * (0.1f), veh_t.hitres[tt.first].Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(veh_t.hitres[tt.first].HitPosition) + VECTOR_ref(veh_t.hitres[tt.first].Normal) * (0.1f), veh_t.hitres[tt.first].Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//��������
										break;
									case 2: //�~�T�C��
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//��������
										break;
									default:
										break;
									}
								}
							}
							for (auto& a : veh_t.use_veh.module_mesh) {
								if (tt.first == a) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(veh_t.hitres[tt.first].HitPosition) + VECTOR_ref(veh_t.hitres[tt.first].Normal) * (0.1f), veh_t.hitres[tt.first].Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(veh_t.hitres[tt.first].HitPosition) + VECTOR_ref(veh_t.hitres[tt.first].Normal) * (0.1f), veh_t.hitres[tt.first].Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//��������
										break;
									case 2: //�~�T�C��
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//��������
										break;
									default:
										break;
									}
								}
							}
						}
					}
					if (hitnear.has_value()) {
						break;
					}
				}
				return (hitnear.has_value());
			}
			return false;
		}
	};
	//�R�b�N�s�b�g
	class cockpits {
	public:
		frames	stickx_f, sticky_f, stickz_f, compass_f, compass2_f, speed_f, speed2_f, spd3_f, spd2_f, spd1_f, cockpit_f, clock_h_f, clock_h2_f, clock_m_f, clock_m2_f, clock_s_f, clock_s2_f, subcompass_f, subcompass2_f
			, alt4_f, alt3_f, alt2_f, alt1_f
			, salt4_f, salt3_f, salt2_f, salt1_f
			, alt_1000_f, alt_1000_2_f, alt_100_f, alt_100_2_f
			, salt_1000_f, salt_1000_2_f, salt_100_f, salt_100_2_f
			, fuel_f, fuel_2_f
			, accel_f;
		MV1 cockpit;

		void set_() {
			MV1::Load("data/model/cockpit/model.mv1", &cockpit, false);
			//
			for (int i = 0; i < cockpit.frame_num(); i++) {
				std::string p = cockpit.frame_name(i);
				if (p.find("����", 0) != std::string::npos) {
					cockpit_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("�p���w����", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					compass_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					//�W���C���R���p�X
				}
				else if ((p.find("�p���w����", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					compass2_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					//�W���C���R���p�X
				}
				else if ((p.find("�\���R���p�X", 0) != std::string::npos)) {
					subcompass_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					subcompass2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
					//�R���p�X
				}
				else if (p.find("�X�e�B�b�N�c", 0) != std::string::npos) {
					stickx_f = { i,cockpit.frame(i) };
					stickz_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("�y�_��", 0) != std::string::npos) && (p.find("�E", 0) == std::string::npos) && (p.find("��", 0) == std::string::npos)) {
					sticky_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("�X���b�g��", 0) != std::string::npos)) {
					accel_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x�v", 0) != std::string::npos)) {
					speed_f = { i,cockpit.frame(i) };
					speed2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("���x100", 0) != std::string::npos)) {
					spd3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x010", 0) != std::string::npos)) {
					spd2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x001", 0) != std::string::npos)) {
					spd1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("���x1000", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt4_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0100", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0010", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0001", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("���x1000", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					salt4_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0100", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					salt3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0010", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					salt2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("���x0001", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					salt1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("���x�v", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt_1000_f = { i,cockpit.frame(i) };
					alt_1000_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("���x���j", 0) != std::string::npos) && (p.find("�\��", 0) == std::string::npos)) {
					alt_100_f = { i,cockpit.frame(i) };
					alt_100_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("���x�v", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos) && (p.find("���j", 0) == std::string::npos)) {
					salt_1000_f = { i,cockpit.frame(i) };
					salt_1000_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("���x�v���j", 0) != std::string::npos) && (p.find("�\��", 0) != std::string::npos)) {
					salt_100_f = { i,cockpit.frame(i) };
					salt_100_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}

				else if ((p.find("�R���v", 0) != std::string::npos)) {
					fuel_f = { i,cockpit.frame(i) };
					fuel_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}

				else if ((p.find("���v", 0) != std::string::npos)) {
					clock_h_f = { i,cockpit.frame(i) };
					clock_h2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("���j", 0) != std::string::npos)) {
					clock_m_f = { i,cockpit.frame(i) };
					clock_m2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("�b�j", 0) != std::string::npos)) {
					clock_s_f = { i,cockpit.frame(i) };
					clock_s2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
			}
		}

		void ready_(Chara& c) {
			float px = (c.p_animes_rudder[1].second - c.p_animes_rudder[0].second)*deg2rad(30);
			float pz = (c.p_animes_rudder[2].second - c.p_animes_rudder[3].second)*deg2rad(30);
			float py = (c.p_animes_rudder[5].second - c.p_animes_rudder[4].second)*deg2rad(20);

			cockpit.SetFrameLocalMatrix(sticky_f.first, MATRIX_ref::RotY(py) * MATRIX_ref::Mtrans(sticky_f.second));
			cockpit.SetFrameLocalMatrix(stickz_f.first, MATRIX_ref::RotZ(pz) * MATRIX_ref::Mtrans(stickz_f.second));
			cockpit.SetFrameLocalMatrix(stickx_f.first, MATRIX_ref::RotX(px) * MATRIX_ref::Mtrans(stickx_f.second));
			cockpit.SetFrameLocalMatrix(compass_f.first, c.vehicle.mat.Inverse() * MATRIX_ref::Mtrans(compass_f.second));
			cockpit.SetFrameLocalMatrix(compass2_f.first, c.vehicle.mat.Inverse() * MATRIX_ref::Mtrans(compass2_f.second));

			{
				float accel_ = c.vehicle.speed;
				cockpit.SetFrameLocalMatrix(accel_f.first, MATRIX_ref::RotX(deg2rad(30.f - (std::clamp(accel_, 0.f, 800.f) / 400.f)*60.f)) * MATRIX_ref::Mtrans(accel_f.second));
			}
			//���x�v
			{
				{
					float spd_buf = c.vehicle.speed*3.6f;
					float spd = 0.f;
					if (spd_buf <= 400.f) {
						spd = 180.f*spd_buf / 440.f;
					}
					else {
						spd = 180.f*(400.f / 440.f + (spd_buf - 400.f) / 880.f);
					}
					cockpit.frame_reset(speed_f.first);
					cockpit.SetFrameLocalMatrix(speed_f.first, MATRIX_ref::RotAxis(speed2_f.second, -deg2rad(spd)) * MATRIX_ref::Mtrans(speed_f.second));
				}
				{
					float spd_buf = c.vehicle.speed*3.6f / 1224.f;

					cockpit.SetFrameLocalMatrix(spd3_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*1.f)) * MATRIX_ref::Mtrans(spd3_f.second));
					cockpit.SetFrameLocalMatrix(spd2_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*10.f)) * MATRIX_ref::Mtrans(spd2_f.second));
					cockpit.SetFrameLocalMatrix(spd1_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*spd_buf*100.f)) * MATRIX_ref::Mtrans(spd1_f.second));
				}
				{
					float alt_buf = c.vehicle.pos.y() / 1000.f;

					cockpit.SetFrameLocalMatrix(alt4_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*1.f)) * MATRIX_ref::Mtrans(alt4_f.second));
					cockpit.SetFrameLocalMatrix(alt3_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*10.f)) * MATRIX_ref::Mtrans(alt3_f.second));
					cockpit.SetFrameLocalMatrix(alt2_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*100.f)) * MATRIX_ref::Mtrans(alt2_f.second));
					cockpit.SetFrameLocalMatrix(alt1_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*1000.f)) * MATRIX_ref::Mtrans(alt1_f.second));
				}
				{
					float alt_buf = c.vehicle.pos.y() / 1000.f;

					cockpit.SetFrameLocalMatrix(salt4_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*1.f)) * MATRIX_ref::Mtrans(salt4_f.second));
					cockpit.SetFrameLocalMatrix(salt3_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*10.f)) * MATRIX_ref::Mtrans(salt3_f.second));
					cockpit.SetFrameLocalMatrix(salt2_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*100.f)) * MATRIX_ref::Mtrans(salt2_f.second));
					cockpit.SetFrameLocalMatrix(salt1_f.first, MATRIX_ref::RotX(-deg2rad(360.f / 10.f*alt_buf*1000.f)) * MATRIX_ref::Mtrans(salt1_f.second));
				}
				{
					float alt_buf = c.vehicle.pos.y();

					cockpit.frame_reset(alt_1000_f.first);
					cockpit.SetFrameLocalMatrix(alt_1000_f.first, MATRIX_ref::RotAxis(alt_1000_2_f.second, -deg2rad(360.f *alt_buf / 1000)) * MATRIX_ref::Mtrans(alt_1000_f.second));
					cockpit.frame_reset(alt_100_f.first);
					cockpit.SetFrameLocalMatrix(alt_100_f.first, MATRIX_ref::RotAxis(alt_100_2_f.second, -deg2rad(360.f *alt_buf / 100)) * MATRIX_ref::Mtrans(alt_100_f.second));
				}
				{
					float alt_buf = c.vehicle.pos.y();

					cockpit.frame_reset(salt_1000_f.first);
					cockpit.SetFrameLocalMatrix(salt_1000_f.first, MATRIX_ref::RotAxis(salt_1000_2_f.second, -deg2rad(360.f *alt_buf / 1000)) * MATRIX_ref::Mtrans(salt_1000_f.second));
					cockpit.frame_reset(salt_100_f.first);
					cockpit.SetFrameLocalMatrix(salt_100_f.first, MATRIX_ref::RotAxis(salt_100_2_f.second, -deg2rad(360.f *alt_buf / 100)) * MATRIX_ref::Mtrans(salt_100_f.second));
				}

				{
					float alt_buf = 1.f;

					cockpit.frame_reset(fuel_f.first);
					cockpit.SetFrameLocalMatrix(fuel_f.first, MATRIX_ref::RotAxis(fuel_2_f.second, -deg2rad( 300.f * alt_buf )) * MATRIX_ref::Mtrans(fuel_f.second));
				}

			}
			//���v
			{
				DATEDATA DateBuf;
				GetDateTime(&DateBuf);
				cockpit.frame_reset(clock_h_f.first);
				cockpit.SetFrameLocalMatrix(clock_h_f.first, MATRIX_ref::RotAxis(clock_h2_f.second, -deg2rad(360.f *DateBuf.Hour / 12 + 360.f / 12.f*DateBuf.Min / 60)) * MATRIX_ref::Mtrans(clock_h_f.second));
				cockpit.frame_reset(clock_m_f.first);
				cockpit.SetFrameLocalMatrix(clock_m_f.first, MATRIX_ref::RotAxis(clock_m2_f.second, -deg2rad(360.f *DateBuf.Min / 60 + 360.f / 60.f*DateBuf.Sec / 60)) * MATRIX_ref::Mtrans(clock_m_f.second));
				cockpit.frame_reset(clock_s_f.first);
				cockpit.SetFrameLocalMatrix(clock_s_f.first, MATRIX_ref::RotAxis(clock_s2_f.second, -deg2rad(360.f *DateBuf.Sec / 60)) * MATRIX_ref::Mtrans(clock_s_f.second));
			}
			//�R���p�X
			{
				VECTOR_ref tmp = c.vehicle.mat.zvec();
				tmp = VGet(tmp.x(), 0.f, tmp.z());
				tmp = tmp.Norm();

				cockpit.frame_reset(subcompass_f.first);
				cockpit.SetFrameLocalMatrix(subcompass_f.first, MATRIX_ref::RotAxis(subcompass2_f.second, std::atan2f(tmp.z(), -tmp.x())) * MATRIX_ref::Mtrans(subcompass_f.second));
			}
			cockpit.SetMatrix(c.vehicle.mat*MATRIX_ref::Mtrans(c.vehicle.obj.frame(c.vehicle.use_veh.fps_view.first) - MATRIX_ref::Vtrans(cockpit_f.second, c.vehicle.mat)));
		}
	};
	//
};
