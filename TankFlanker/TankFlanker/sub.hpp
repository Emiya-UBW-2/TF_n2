#pragma once

#define NOMINMAX
#include"DXLib_ref.h"
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <memory>
#include<optional>
#include "Box2D/Box2D.h"



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

namespace std {
	template <>
	struct default_delete<b2Body> {
		void operator()(b2Body* body) const {
			body->GetWorld()->DestroyBody(body);
		}
	};
};
//�v���P
class hit{
public:
	struct ammo {
		std::string name;
		int16_t type;
		float caliber;
		float penetration;
		float speed;
	};

	typedef std::pair<int, VECTOR_ref> frames;
	struct gun {
		int type;
		frames frame1;
		frames frame2;
		frames frame3;
		float xrad = 0.f, yrad = 0.f;
		std::string name;
		float load_time = 0.f;
		std::vector<std::string> useammo;
	};
	struct foot {
		frames frame;
		EffectS gndsmkeffcs;
		float gndsmksize;
	};
	class Tanks {
	public:
		std::string name;
		bool isfloat;
		float down_in_water;
		MV1 obj;
		std::vector<gun> gunframe;
		std::vector<foot> wheelframe;
		MV1 col;
		std::array<int, 4> square;
		float flont_speed_limit;/*�O�i���x(km/h)*/
		float back_speed_limit;/*��ޑ��x(km/h)*/
		float body_rad_limit;/*�ԑ̋쓮���x(�x/�b)*/
		float turret_rad_limit;/*�C���쓮���x(�x/�b)*/
		std::vector<std::pair<size_t, float>> armer_mesh;	  /*���bID*/	/**/
		std::vector<size_t> space_mesh;	  /*���bID*/	/**/
		std::vector<size_t> module_mesh;	  /*���bID*/	/**/
		VECTOR_ref minpos, maxpos;

		void into(const Tanks& t) {
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->square = t.square;


			this->name = t.name;
			this->obj = t.obj.Duplicate();
			this->col = t.col.Duplicate();
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
		}
	};
	class Planes {
	public:
		std::string name;
		MV1 obj;
		MV1 col;
		std::vector<foot> wheelframe;
		float max_speed_limit;/*�ō����x(km/h)*/
		float mid_speed_limit;/*���s���x(km/h)*/
		float min_speed_limit;/*�������x(km/h)*/
		float rad_limit;/*���񑬓x(�x/�b)*/
		VECTOR_ref minpos, maxpos;
		std::vector<gun> gunframe;
		std::vector<frames> burner;
		frames hook;

		void into(const Planes& t) {
			this->name = t.name;
			this->obj = t.obj.Duplicate();
			this->col = t.col.Duplicate();
			this->wheelframe.clear();
			for (auto& p : t.wheelframe) {
				this->wheelframe.resize(this->wheelframe.size() + 1);
				this->wheelframe.back().frame = p.frame;
			}
			this->max_speed_limit = t.max_speed_limit;
			this->mid_speed_limit = t.mid_speed_limit;
			this->min_speed_limit = t.min_speed_limit;
			this->rad_limit = t.rad_limit;
			this->minpos = t.minpos;
			this->maxpos = t.maxpos;
			this->gunframe = t.gunframe;
			this->burner = t.burner;
			this->hook = t.hook;
		}
	};
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		int color = 0;
		ammo spec;
		float yadd;
		VECTOR_ref pos, repos, vec;
	};
	struct b2Pats {
		std::unique_ptr<b2Body> body; /**/
		b2Fixture* playerfix;	 /**/
	};
	struct wallPats {
		hit::b2Pats b2;
		std::array<VECTOR_ref, 2> pos;
	};
	typedef std::pair<int, float> animes;
	struct Chara {
		size_t id;

		VECTOR_ref zvec;
		VECTOR_ref yvec;
		VECTOR_ref wheel_normal;
		//�G�t�F�N�g
		std::array<EffectS, efs_user> effcs; /*effect*/
		//����֘A
		bool flight = false;//�t���C�g(�`��)�t���O
		std::array<bool, 12> key;//�L�[
		float view_xrad = 0.f,view_yrad = 0.f;//�C������p�x�N�g��
		//====================================================
		std::vector<int16_t> HP;			   /*���C�t*/
		//���//==================================================
			Tanks usetank;
			float wheel_Left = 0.f,wheel_Right = 0.f;  //�]�։�]
			b2Pats mine;				   /*box2d*/
			float spd;				   /*box2d*/
			struct Hit {
				bool flug{ false };		   /*�e���t���O*/
				int use{ 0 };			   /*�g�p�t���[��*/
				MV1 pic;			   /*�e�����f��*/
				VECTOR_ref scale, pos, zvec, yvec; /*���W*/
			};
			int hitbuf;				   /*�g�p�e��*/
			std::array<Hit, 24> hit;		   /*�e��*/
			std::vector<MV1_COLL_RESULT_POLY> hitres;  /*�m��*/
			std::vector<pair> hitssort;		   /*�t���[���ɓ�����������*/
			float xradp_shot = 0.f, xrad_shot = 0.f;   //�ˌ�����x
			float zradp_shot = 0.f, zrad_shot = 0.f;   //�ˌ�����z
		//��s�@//==================================================
			Planes useplane;		      //
			animes p_anime_geardown;	      //�ԗփA�j���[�V����
			uint8_t changegear_cnt = 0;	      //�J�E���g
			bool changegear = false;	      //�X�C�b�`
			std::array<animes, 6> p_animes_rudder;//���_�[�A�j���[�V����
			struct burners{
				frames frame;
				MV1 effectobj;
			};
			std::vector<burners> p_burner;
		//���ʍ�
		struct vehicles {
			bool hit_check = false;		      //�����蔻�����邩�`�F�b�N
			size_t use_id=0;		      //�g�p����ԗ�(�@��)
			VECTOR_ref pos;			      //�ԑ̍��W
			MATRIX_ref mat;			      //�ԑ̉�]�s��
			VECTOR_ref add;			      //�ԑ̉����x
			struct Guns {
				std::array<ammos, 32> Ammo;   /*�m�ۂ���e*/
				float loadcnt{ 0 };	      /*���Ă�J�E���^�[*/
				float loadcnt_all{ 0 };	      /*���Ă�J�E���^�[*/
				size_t useammo{};	      /*�g�p�e*/
				float fired{ 0.f };	      /*����*/
				gun gun_info;		      /**/
				std::vector<ammo> Spec;	      /**/
			};
			std::vector<Guns> Gun_;
			float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;
			float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f;
			float yrad = 0.f, yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;
			float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f;
		};
		std::array<vehicles, 2> vehicle;//0=���,1=��s�@
	};
	//
	static bool get_reco(Chara& play, std::vector<Chara>& tgts, ammos& c) {
		//
		if (c.spec.type == 0) {
			bool is_hit;
			std::optional<size_t> hitnear;
			for (auto& t : tgts) {
				//�������g�͏Ȃ�
				if (play.id == t.id) {
					continue;
				}
				//�Ƃ肠���������������ǂ����T��
				is_hit = false;
				//���W���[��
				for (auto& m : t.usetank.module_mesh) {
					t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
					if (t.hitres[m].HitFlag) {
						t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
					}
				}
				//��ԑ��b
				for (auto& m : t.usetank.space_mesh) {
					t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
					if (t.hitres[m].HitFlag) {
						t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
					}
				}
				//���b
				for (auto& m : t.usetank.armer_mesh) {
					t.hitres[m.first] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
					if (t.hitres[m.first].HitFlag) {
						t.hitssort[m.first] = pair(m.first, (c.repos - t.hitres[m.first].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m.first] = pair(m.first, (std::numeric_limits<float>::max)());
					}
				}
				//�������ĂȂ�
				if (!is_hit) {
					continue;
				}
				//�����蔻����߂����Ƀ\�[�g
				std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
				//����
				{
					//�_���[�W�ʂɓ͂��܂Ŕ���
					for (auto& tt : t.hitssort) {
						//���b�ʂɓ�����Ȃ������Ȃ�X���[
						if (tt.second == (std::numeric_limits<float>::max)()) {
							break;
						}
						//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
						for (auto& a : t.usetank.armer_mesh) {
							if (tt.first == a.first) {
								hitnear = tt.first;
								//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
								{
									VECTOR_ref normal = t.hitres[hitnear.value()].Normal;
									VECTOR_ref position = t.hitres[hitnear.value()].HitPosition;
									if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm() % normal))) {
										//�ђ�
										//t.id;
										t.HP[0] = std::max<int16_t>(t.HP[0] - 1, 0); //
										//���j���G�t�F�N�g
										if (t.HP[0] == 0) {
											//set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
										}
										//�e����
										c.flug = false;
										c.vec += normal.Scale((c.vec % normal) * -2.0f);
										c.vec = c.vec.Norm();
										c.pos = c.vec.Scale(0.1f) + position;
										//�e��
										t.hit[t.hitbuf].use = 0;
									}
									else {
										//�͂���
										//�e����
										c.vec += normal.Scale((c.vec % normal) * -2.0f);
										c.vec = c.vec.Norm();
										c.pos = c.vec.Scale(0.1f) + position;
										c.spec.penetration /= 2.0f;
										//�e��
										t.hit[t.hitbuf].use = 1;
									}
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], c.pos, normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], c.pos, normal);
									}

									//�e���̃Z�b�g
									{
										float asize = c.spec.caliber * 100.f;
										t.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm() % normal), asize, asize);
										t.hit[t.hitbuf].pos = (position - t.vehicle[0].pos).Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].yvec = normal.Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].zvec = (normal * c.vec).Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].flug = true;
										++t.hitbuf %= t.hit.size();
									}
								}
								break;
							}
						}
						if (hitnear.has_value()) {
							break;
						}
						//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�ѓO�͂�1/2��
						for (auto& a : t.usetank.space_mesh) {
							if (tt.first == a) {
								if (c.spec.caliber >= 0.020f) {
									set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								else {
									set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
								c.spec.penetration /= 2.0f;
							}
						}
						for (auto& a : t.usetank.module_mesh) {
							if (tt.first == a) {
								if (c.spec.caliber >= 0.020f) {
									set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								else {
									set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
								c.spec.penetration /= 2.0f;
							}
						}
					}
				}
				if (hitnear.has_value())
					break;
			}
			return (hitnear.has_value());
		}
		if (c.spec.type == 1) {
			bool is_hit;
			std::optional<size_t> hitnear;
			for (auto& t : tgts) {
				//�������g�͏Ȃ�
				if (play.id == t.id) {
					continue;
				}
				//�Ƃ肠���������������ǂ����T��
				is_hit = false;
				//���W���[��
				for (auto& m : t.usetank.module_mesh) {
					t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
					if (t.hitres[m].HitFlag) {
						t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
					}
				}
				//��ԑ��b
				for (auto& m : t.usetank.space_mesh) {
					t.hitres[m] = t.usetank.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos).Scale(0.1f)), -1, int(m));
					if (t.hitres[m].HitFlag) {
						t.hitssort[m] = pair(m, (c.repos - t.hitres[m].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m] = pair(m, (std::numeric_limits<float>::max)());
					}
				}
				//���b
				for (auto& m : t.usetank.armer_mesh) {
					t.hitres[m.first] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
					if (t.hitres[m.first].HitFlag) {
						t.hitssort[m.first] = pair(m.first, (c.repos - t.hitres[m.first].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m.first] = pair(m.first, (std::numeric_limits<float>::max)());
					}
				}
				//�������ĂȂ�
				if (!is_hit) {
					continue;
				}
				//�����蔻����߂����Ƀ\�[�g
				std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
				//����
				{
					//�_���[�W�ʂɓ͂��܂Ŕ���
					for (auto& tt : t.hitssort) {
						//���b�ʂɓ�����Ȃ������Ȃ�X���[
						if (tt.second == (std::numeric_limits<float>::max)()) {
							break;
						}
						//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
						for (auto& a : t.usetank.armer_mesh) {
							if (tt.first == a.first) {
								hitnear = tt.first;
								//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
								{
									VECTOR_ref normal = t.hitres[hitnear.value()].Normal;
									VECTOR_ref position = t.hitres[hitnear.value()].HitPosition;
									if (c.spec.penetration > a.second * (1.0f / std::abs(c.vec.Norm() % normal))) {
										//�ђ�
										//t.id;
										t.HP[0] = std::max<int16_t>(t.HP[0] - 1, 0); //
										//���j���G�t�F�N�g
										if (t.HP[0] == 0) {
											//set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
										}
										//�e����
										c.flug = false;
										c.vec += normal.Scale((c.vec % normal) * -2.0f);
										c.vec = c.vec.Norm();
										c.pos = c.vec.Scale(0.1f) + position;
										//�e��
										t.hit[t.hitbuf].use = 0;
									}
									else {
										//��������
										//�e����
										c.flug = false;
										c.vec += normal.Scale((c.vec % normal) * -2.0f);
										c.vec = c.vec.Norm();
										c.pos = c.vec.Scale(0.1f) + position;
										//�e��
										t.hit[t.hitbuf].use = 1;
									}
									if (c.spec.caliber >= 0.020f) {
										set_effect(&play.effcs[ef_reco], c.pos, normal);
									}
									else {
										set_effect(&play.effcs[ef_reco2], c.pos, normal);
									}

									//�e���̃Z�b�g
									{
										float asize = c.spec.caliber * 100.f;
										t.hit[t.hitbuf].scale = VGet(asize / std::abs(c.vec.Norm() % normal), asize, asize);
										t.hit[t.hitbuf].pos = (position - t.vehicle[0].pos).Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].yvec = normal.Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].zvec = (normal * c.vec).Vtrans(t.vehicle[0].mat.Inverse());
										t.hit[t.hitbuf].flug = true;
										++t.hitbuf %= t.hit.size();
									}
								}
								break;
							}
						}
						if (hitnear.has_value()) {
							break;
						}
						//��ԑ��b�A���W���[���ɓ��������̂Ń��W���[����30�_���A�e�Ȃ�
						for (auto& a : t.usetank.space_mesh) {
							if (tt.first == a) {
								if (c.spec.caliber >= 0.020f) {
									set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								else {
									set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
								//��������
								c.flug = false;
							}
						}
						for (auto& a : t.usetank.module_mesh) {
							if (tt.first == a) {
								if (c.spec.caliber >= 0.020f) {
									set_effect(&play.effcs[ef_reco], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								else {
									set_effect(&play.effcs[ef_reco2], VECTOR_ref(t.hitres[tt.first].HitPosition) + VECTOR_ref(t.hitres[tt.first].Normal).Scale(0.1f), t.hitres[tt.first].Normal);
								}
								t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
								//��������
								c.flug = false;
							}
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
void find_folders(const char* name, std::vector<hit::Tanks>* doing) {
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	hFind = FindFirstFile(name, &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				doing->resize(doing->size() + 1);
				doing->back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}
void find_folders(const char* name, std::vector<hit::Planes>* doing) {
	WIN32_FIND_DATA win32fdt;
	HANDLE hFind;
	hFind = FindFirstFile(name, &win32fdt);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if ((win32fdt.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (win32fdt.cFileName[0] != '.')) {
				doing->resize(doing->size() + 1);
				doing->back().name = win32fdt.cFileName;
			}
		} while (FindNextFile(hFind, &win32fdt));
	} //else{ return false; }
	FindClose(hFind);
}

template <class T>
void fill_id(std::vector<T>& vect){
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
template <class T,size_t N >
void fill_id(std::array<T,N>& vect) {
	for (int i = 0; i < vect.size(); i++) {
		vect[i].id = i;
	}
}
//ok
class UI :hit
{
private:
	GraphHandle circle;
	GraphHandle aim;
	GraphHandle scope;
	GraphHandle CompassScreen;
	MV1 Compass;
	//font
	FontHandle font18;
public:
	UI() {
		circle = GraphHandle::Load("data/UI/battle_circle.bmp");
		aim = GraphHandle::Load("data/UI/battle_aim.bmp");
		scope = GraphHandle::Load("data/UI/battle_scope.png");

		CompassScreen = GraphHandle::Make(240, 240,true);
		MV1::Load("data/compass/model.mv1", &Compass);
		font18 = FontHandle::Create(18);
	}
	~UI() {

	}

	void draw(const VECTOR_ref& aimpos, const hit::Chara& chara, const bool& ads) {
		{
			auto scr = GetDrawScreen();
			auto fov = GetCameraFov();
			{
				SetDrawScreen(CompassScreen.get());
				ClearDrawScreen();
				SetupCamera_Ortho(2.f);
				SetCameraPositionAndTargetAndUpVec(VGet(0.f, 0.f, 3.f), VGet(0.f, 0.f, 0.f), VGet(0.f, -1.f, 0.f));
				SetCameraNearFar(0.1f, 6.0f);
				SetUseLighting(FALSE);
				Compass.SetRotationZYAxis(
					VECTOR_ref(VGet(0.f, 0.f, 1.f)).Vtrans(chara.vehicle[1].mat.Inverse()),
					VECTOR_ref(VGet(0.f, 1.f, 0.f)).Vtrans(chara.vehicle[1].mat.Inverse()),
					0.f
				);
				Compass.SetPosition(VGet(0.f, 0.f, 0.f));
				Compass.DrawModel();
				SetUseLighting(TRUE);
			}
			SetDrawScreen(scr);
			SetupCamera_Perspective(fov);
		}
		if (aimpos.z() >= 0.f && aimpos.z() <= 1.f) {
			circle.DrawExtendGraph(int(aimpos.x()) - x_r(128), int(aimpos.y()) - y_r(128), int(aimpos.x()) + x_r(128), int(aimpos.y()) + y_r(128), TRUE);
		}

		aim.DrawExtendGraph(dispx / 2 - x_r(128), dispy / 2 - y_r(128), dispx / 2 + x_r(128), dispy / 2 + y_r(128), TRUE);

		if (ads) {
			scope.DrawExtendGraph(0, 0, dispx, dispy, true);
		}

		int i = 0;
		if (!chara.flight) {
			for (auto& p : chara.vehicle[0].Gun_) {
				int xp = 20 + 30 - (30 * (i + 1) / int(chara.vehicle[0].Gun_.size()));
				int yp = 200 + i * 32;
				DrawBox(xp, yp, xp + 200, yp + 24, GetColor(128, 128, 128), TRUE);
				if (p.loadcnt != 0.f) {
					DrawBox(xp, yp, xp + 200 - int(200.f*p.loadcnt / p.loadcnt_all), yp + 24, GetColor(255, 0, 0), TRUE);
				}
				font18.DrawString(xp, yp, p.Ammo[p.useammo].spec.name, GetColor(255, 255, 255));
				i++;
			}
		}
		else {
			for (auto& p : chara.vehicle[1].Gun_) {
				int xp = 20 + 30 - (30 * (i + 1) / int(chara.vehicle[0].Gun_.size()));
				int yp = 200 + i * 32;

				DrawBox(xp, yp, xp + 200, yp + 18, GetColor(128, 128, 128), TRUE);
				if (p.loadcnt != 0.f) {
					DrawBox(xp, yp, xp + 200 - int(200.f*p.loadcnt / p.loadcnt_all), yp + 18, GetColor(255, 0, 0), TRUE);
				}
				font18.DrawString(xp, yp, p.Ammo[p.useammo].spec.name, GetColor(255, 255, 255));
				i++;
			}
		}

		CompassScreen.DrawGraph(400, 400, true);

		font18.DrawStringFormat(0, 300, GetColor(255, 255, 255), "%6.2f km/h", chara.vehicle[1].speed*3.6f);
	}
};
//
