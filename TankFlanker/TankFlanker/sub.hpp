#pragma once

#define NOMINMAX
#include"DXLib_ref.h"
#include <fstream>
#include <array>
#include <vector>
#include <D3D11.h>
#include <memory>
#include<optional>

void set_effect(EffectS* efh, VECTOR_ref pos, VECTOR_ref nor) {
	efh->flug = true;
	efh->pos = pos;
	efh->nor = nor;
}
void set_pos_effect(EffectS* efh, const EffekseerEffectHandle& handle) {
	if (efh->flug) {
		efh->handle = handle.Play3D();
		efh->handle.SetPos(efh->pos);
		efh->handle.SetRotation(atan2(efh->nor.y(), std::hypot(efh->nor.x(), efh->nor.z())), atan2(-efh->nor.x(), -efh->nor.z()), 0);
		efh->flug = false;
	}
	//IsEffekseer3DEffectPlaying(player[0].effcs[i].handle)
}

//�v���P
class hit{
public:
	class Tanks {
		struct gun {
			int type;
			int id;
			VECTOR_ref pos;
			int id2;
			VECTOR_ref pos2;
			int id3;
			VECTOR_ref pos3;
			float xrad = 0.f;
			float yrad = 0.f;
		};
		struct foot {
			bool LorR;
			int id;
			VECTOR_ref pos;
		};
		struct armers {
			size_t mesh = 0;
			float thickness = 0.f;
		};
	public:
		bool isfloat;
		float down_in_water;
		MV1 obj;
		std::vector<gun> gunframe;
		std::vector<foot> wheelframe;
		MV1 col;
		std::vector<gun> gunframe_col;
		std::array<int, 4> square;
		float flont_speed_limit;/*�O�i���x(km/h)*/
		float back_speed_limit;/*��ޑ��x(km/h)*/
		float body_rad_limit;/*�ԑ̋쓮���x(�x/�b)*/
		float turret_rad_limit;/*�C���쓮���x(�x/�b)*/

		std::vector<armers> armer_mesh;	  /*���bID*/	/**/
		std::vector<size_t> space_mesh;	  /*���bID*/	/**/
		std::vector<size_t> module_mesh;	  /*���bID*/	/**/

		void into(const Tanks& t) {
			this->isfloat = t.isfloat;
			this->down_in_water = t.down_in_water;
			this->obj = t.obj.Duplicate();
			this->gunframe = t.gunframe;
			this->wheelframe = t.wheelframe;
			this->col = t.col.Duplicate();
			this->square = t.square;

			this->flont_speed_limit = t.flont_speed_limit;
			this->back_speed_limit = t.back_speed_limit;
			this->body_rad_limit = t.body_rad_limit;
			this->turret_rad_limit = t.turret_rad_limit;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
			this->gunframe_col = t.gunframe_col;
		}
	};



	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float cnt = 0.f;
		int color = 0;
		float speed = 0.f, pene = 0.f;
		float yadd;
		VECTOR_ref pos, repos, vec;
	};
	struct Chara {
		size_t id;
		size_t useid;
		Tanks usetank;

		float wheel_Left = 0.f;
		float wheel_Right = 0.f;

		VECTOR_ref pos, add, zvec, yvec, yvect;
		float yrad = 0.f;
		float xradp = 0.f;
		float xrad = 0.f;
		float zradp = 0.f;
		float zrad = 0.f;

		float xradp_shot = 0.f;
		float xrad_shot = 0.f;
		float zradp_shot = 0.f;
		float zrad_shot = 0.f;

		float yadd_left;
		float yadd_right;
		float zadd_flont;
		float zadd_back;
		std::array<bool, 6> key;

		std::array<EffectS, efs_user> effcs; /*effect*/
		std::vector<EffectS> gndsmkeffcs;    /*effect*/
		std::vector<float> gndsmksize;       /*effect*/

		struct Guns {			       /**/
			std::array<ammos, 32> Ammo; /*�m�ۂ���e*/
			float loadcnt{ 0 };	      /*���Ă�J�E���^�[*/
			size_t useammo{};	      /*�g�p�e*/
			float fired{ 0.f };	    /*����*/
		};				       /**/
		std::vector<Guns> Gun;	    /*�e�A�C�S��*/

		float view_yrad;
		float view_xrad;
		//====================================================
		std::vector<pair> hitssort;	  /*������������*/
		int hitbuf;			     /*�g�p�e��*/
		std::vector<MV1_COLL_RESULT_POLY> hitres; /*�m��*/
		std::vector<int16_t> HP;	     /*���C�t*/
		bool hitadd{ false };		     /*�����t���O*/
		size_t hitid{ 0 };			     /*���Ă��G*/
		int recorad{ 0 };		      /*�e���p�x*/
		VECTOR_ref recovec;		      /*�e���x�N�g��*/
		MATRIX ps_m;	      /*�ԑ̍s��*/
		//�e��
		struct Hit {
			bool flug{ false }; /**/
			int use{ 0 };       /*�g�p�t���[��*/
			MV1 pic; /*�e�����f��*/
			VECTOR_ref scale, pos, zvec, yvec;	/*���W*/
		};
		std::array<Hit, 3> hit;		     /*�e��*/
		float recorange{ 0 };				/*�e���̋���*/
	};
	//
	static bool get_reco(Chara& play, std::vector<Chara>& tgts, ammos& c, size_t gun_s) {
		//
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
					t.hitres[m.mesh] = t.usetank.col.CollCheck_Line(c.repos, c.pos, -1, int(m.mesh));
					if (t.hitres[m.mesh].HitFlag) {
						t.hitssort[m.mesh] = pair(m.mesh, (c.repos - t.hitres[m.mesh].HitPosition).size());
						is_hit = true;
					}
					else {
						t.hitssort[m.mesh] = pair(m.mesh, (std::numeric_limits<float>::max)());
					}
				}
			//�������ĂȂ�
			if (!is_hit) {
				continue;
			}
			//�����蔻����߂����Ƀ\�[�g
			std::sort(t.hitssort.begin(), t.hitssort.end(), [](const pair& x, const pair& y) { return x.second < y.second; });
			//��C�̔���
			if (gun_s == 0) {
				//�_���[�W�ʂɓ͂��܂Ŕ���
				for (auto& tt : t.hitssort) {
					//���b�ʂɓ�����Ȃ������Ȃ�X���[
					if (tt.second == (std::numeric_limits<float>::max)()) {
						break;
					}
					//���b�ʂɓ��������̂�hitnear�ɑ�����ďI��
					for(auto& a : t.usetank.armer_mesh){
						if (tt.first == a.mesh) {
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							hitnear = tt.first;
							//�_���[�W�ʂɓ����������ɑ��b�l�ɏ��Ă邩�ǂ���
							if (hitnear.has_value()) {
								const auto k = hitnear.value();
								if (c.pene > a.thickness * (1.0f / abs(c.vec.Norm() % t.hitres[k].Normal))) {
									//�ђ�
									play.hitadd = true;
									play.hitid = t.id;
									t.HP[0] = std::max<int16_t>(t.HP[0] - 1, 0); //
									//���j���G�t�F�N�g
									if (t.HP[0] == 0) {
										//set_effect(&t.effcs[ef_bomb], t.usetank.obj.frame(t.ptr->engineframe), VGet(0, 0, 0));
									}
									//�e����
									c.flug = false;
									//�e��
									t.hit[t.hitbuf].use = 0;
								}
								else {
									//�͂���
									//�e����
									c.vec += VScale(t.hitres[k].Normal, (c.vec % t.hitres[k].Normal) * -2.0f);
									c.vec = c.vec.Norm();
									c.pos = c.vec.Scale(0.1f) + t.hitres[k].HitPosition;
									c.pene /= 2.0f;
									//�e��
									t.hit[t.hitbuf].use = 1;
								}
								//�e���̃Z�b�g
								{
									float asize = /*play.ptr->gun_[gun_s].ammosize*/0.105f * 100.f;
									t.hit[t.hitbuf].scale = VGet(asize / abs(c.vec.Norm() % t.hitres[k].Normal), asize, asize);
									t.hit[t.hitbuf].pos = VTransform(t.hitres[k].HitPosition, MInverse(t.ps_m));
									t.hit[t.hitbuf].zvec = VTransform(((VECTOR_ref(t.hitres[k].Normal)) + t.hitres[k].HitPosition).get(), MInverse(t.ps_m));
									t.hit[t.hitbuf].yvec = VTransform(((VECTOR_ref(t.hitres[k].Normal) * c.vec) + t.hitres[k].HitPosition).get(), MInverse(t.ps_m));
									t.hit[t.hitbuf].flug = true;
									++t.hitbuf %= 3;
								}
								//��e����
								if (t.recorad == 180) {
									float rad = atan2(t.hitres[k].HitPosition.x - t.pos.x(), t.hitres[k].HitPosition.z - t.pos.z());
									t.recovec = VGet(cos(rad), 0, -sin(rad));
									t.recorad = 0;
									t.recorange = 3.f;
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
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
							c.pene /= 2.0f;
						}
					}
					for (auto& a : t.usetank.module_mesh) {
						if (tt.first == a) {
							set_effect(&play.effcs[ef_reco], t.hitres[tt.first].HitPosition, t.hitres[tt.first].Normal);
							t.HP[tt.first] = std::max<int16_t>(t.HP[tt.first] - 0, 0); //
							c.pene /= 2.0f;
						}
					}
				}
			}
			//�����@�e
			else {
				//
				if (t.hitssort.begin()->second == (std::numeric_limits<float>::max)()) {
					continue;
				}
				//���߂Œe������
				hitnear = t.hitssort.begin()->first;
				if (hitnear.has_value()) {
					set_effect(&play.effcs[ef_reco2], t.hitres[hitnear.value()].HitPosition, t.hitres[hitnear.value()].Normal);

					c.vec = c.vec + VScale(t.hitres[hitnear.value()].Normal, (c.vec % t.hitres[hitnear.value()].Normal) * -2.0f);
					c.pos = c.vec.Scale(0.1f) + t.hitres[hitnear.value()].HitPosition;

					if (hitnear.value() >= 5 && hitnear.value() < t.HP.size()) {
						t.HP[hitnear.value()] = std::max<int16_t>(t.HP[hitnear.value()] - 1, 0); //
					}
				}
			}
			if (hitnear.has_value())
				break;
		}
		return (hitnear.has_value());
	}
};

