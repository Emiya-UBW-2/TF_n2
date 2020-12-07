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
	ef_fire = 0, //発砲炎
	ef_reco = 1, //大口径跳弾
	ef_reco2 = 2, //小口径跳弾
	ef_gndhit = 3, //大口径着弾
	ef_gndhit2 = 4, //小口径着弾
	ef_bomb = 5, //撃破爆発
	ef_smoke1 = 6, //ミサイル炎
	ef_smoke2 = 7, //銃の軌跡
	ef_gndsmoke = 8,//地面の軌跡
	ef_size = 9
};

//要改善
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
		bool flug{ false };   /*弾痕フラグ*/
		int use{ 0 };	      /*使用フレーム*/
		MV1 pic;	      /*弾痕モデル*/
		VECTOR_ref pos;	      /*座標*/
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
	//弾薬
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
	//車輛
	class Vehcs {
	public:
		//共通
		std::string name;				  /**/
		MV1 obj, col;					  /**/
		VECTOR_ref minpos, maxpos;			  /**/
		std::vector<gun_frame> gunframe;			  /**/
		std::vector<foot_frame> wheelframe;			  /**/
		std::vector<foot_frame> wheelframe_nospring;		  /*誘導輪回転*/
		uint16_t HP = 0;					  /**/
		std::vector<std::pair<size_t, float>> armer_mesh; /*装甲ID*/
		std::vector<size_t> space_mesh;			  /*装甲ID*/
		std::vector<size_t> module_mesh;		  /*装甲ID*/
		int camo_tex = 0;				  /**/
		std::vector<int> camog;				  /**/
		bool isfloat = false;			  /*浮くかどうか*/
		float down_in_water = 0.f;			  /*沈む判定箇所*/
		float max_speed_limit = 0.f;			  /*最高速度(km/h)*/
		float mid_speed_limit = 0.f;			  /*巡行速度(km/h)*/
		float min_speed_limit = 0.f;			  /*失速速度(km/h)*/
		float flont_speed_limit = 0.f;			  /*前進速度(km/h)*/
		float back_speed_limit = 0.f;			  /*後退速度(km/h)*/
		float body_rad_limit = 0.f;			  /*旋回速度(度/秒)*/
		float turret_rad_limit = 0.f;			  /*砲塔駆動速度(度/秒)*/
		frames fps_view;//コックピット
		GraphHandle ui_pic;//シルエット
		int pic_x, pic_y;//サイズ
		//専門
		std::array<int, 4> square{ 0 };//車輛の四辺
		std::array<std::vector<frames>, 2> b2upsideframe; /*履帯上*/
		std::array<std::vector<frames>, 2> b2downsideframe; /*履帯上*/
		std::vector<frames> burner;//アフターバーナー
		frames hook;//着艦フック

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
		//事前読み込み
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
		//メイン読み込み
		static void set_vehicles(std::vector<Mainclass::Vehcs>* vehcs) {
			using namespace std::literals;
			//共通
			for (auto& t : *vehcs) {
				//αテスト
				t.obj.material_AlphaTestAll(true, DX_CMP_GREATER, 128);
				//
				GetGraphSize(t.ui_pic.get(), &t.pic_x, &t.pic_y);
			}
			//固有
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
				//最大最小を取得
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
				//フレーム
				for (int i = 0; i < t.obj.frame_num(); i++) {
					std::string p = t.obj.frame_name(i);
					if (p.find("脚", 0) != std::string::npos) {
						if (p.find("ハッチ", 0) == std::string::npos) {
							t.wheelframe.resize(t.wheelframe.size() + 1);
							t.wheelframe.back().frame.first = i;
							t.wheelframe.back().frame.second = t.obj.frame(t.wheelframe.back().frame.first);

							t.wheelframe_nospring.resize(t.wheelframe_nospring.size() + 1);
							t.wheelframe_nospring.back().frame.first = t.wheelframe.back().frame.first + 1;
							t.wheelframe_nospring.back().frame.second = t.obj.frame(t.wheelframe_nospring.back().frame.first) - t.wheelframe.back().frame.second;
						}
					}
					else if (p.find("旋回", 0) != std::string::npos) {
						t.gunframe.resize(t.gunframe.size() + 1);
						t.gunframe.back().frame1.first = i;
						t.gunframe.back().frame1.second = t.obj.frame(t.gunframe.back().frame1.first);
						auto p2 = t.obj.frame_parent(t.gunframe.back().frame1.first);
						if (p2 >= 0) {
							t.gunframe.back().frame1.second -= t.obj.frame(int(p2)); //親がいる時引いとく
						}
						if (t.obj.frame_child_num(t.gunframe.back().frame1.first) >= 0) {
							if (t.obj.frame_name(t.gunframe.back().frame1.first + 1).find("仰角", 0) != std::string::npos) {
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
					else if (p.find("バーナー", 0) != std::string::npos) {
						t.burner.resize(t.burner.size() + 1);
						t.burner.back().first = i;
						t.burner.back().second = t.obj.frame(t.burner.back().first);
					}
					else if (p.find("フック", 0) != std::string::npos) {
						t.hook.first = i;
						t.hook.second = t.obj.frame(t.hook.first);
					}
					else if (p.find("視点", 0) != std::string::npos) {
						t.fps_view.first = i;
						t.fps_view.second = t.obj.frame(t.fps_view.first);
					}
				}
				//メッシュ
				for (int i = 0; i < t.col.mesh_num(); i++) {
					std::string p = t.col.material_name(i);
					if (p.find("armer", 0) != std::string::npos) { //装甲
						t.armer_mesh.resize(t.armer_mesh.size() + 1);
						t.armer_mesh.back().first = i;
						t.armer_mesh.back().second = std::stof(getparams::getright(p.c_str())); //装甲値
					}
					else if (p.find("space", 0) != std::string::npos) {		    //空間装甲
						t.space_mesh.resize(t.space_mesh.size() + 1);
						t.space_mesh.back() = i;
					}
					else { //モジュール
						t.module_mesh.resize(t.module_mesh.size() + 1);
						t.module_mesh.back() = i;
					}
				}
				//迷彩
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
				//データ取得
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
		size_t usebullet{};					      /*使用弾*/
		std::array<ammos, 64> bullet;				      /*確保する弾*/
		float loadcnt{ 0 };					      /*装てんカウンター*/
		float fired{ 0.f };					      /*駐退数*/
		int16_t rounds{ 0 };					      /*弾数*/
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
		Vehcs use_veh;							      /*固有値*/
		MV1 obj;							      /**/
		MV1 col;							      /**/
		bool hit_check = false;						      //当たり判定を取るかチェック
		size_t use_id = 0;						      //使用する車両(機材)
		uint16_t HP = 0;						      /*体力*/
		uint16_t KILL = 0;						      /*体力*/
		int KILL_ID = -1;						      /*体力*/
		uint16_t DEATH = 0;						      /*体力*/
		int DEATH_ID = -1;						      /*体力*/
		VECTOR_ref pos;							      //車体座標
		MATRIX_ref mat;							      //車体回転行列
		MATRIX_ref mat_start;					      //車体回転行列(初期配置)
		VECTOR_ref add;							      //車体加速度
		std::vector<Guns> Gun_;						      /**/
		float speed = 0.f, speed_add = 0.f, speed_sub = 0.f;		      /**/
		float xrad = 0.f, xradadd = 0.f, xradadd_left = 0.f, xradadd_right = 0.f; /**/
		float yrad = 0.f, yradadd = 0.f, yradadd_left = 0.f, yradadd_right = 0.f;	    /**/
		float zrad = 0.f, zradadd = 0.f, zradadd_left = 0.f, zradadd_right = 0.f; /**/
		std::vector<MV1_COLL_RESULT_POLY> hitres;			      /*確保*/
		std::vector<int16_t> HP_m;					      /*ライフ*/
		std::array<Hit, 24> hit_obj;					      /*弾痕*/
		size_t camo_sel = 0;						      /**/
		float wheel_Left = 0.f, wheel_Right = 0.f;			      //転輪回転
		float wheel_Leftadd = 0.f, wheel_Rightadd = 0.f;		      //転輪回転
		std::vector<pair_hit> hitssort;					      /*フレームに当たった順番*/
	public:
		void init() {
			this->obj.Dispose();
			this->col.Dispose();
			this->hit_check = false;
			this->HP = 0;
			this->KILL = 0;						      /*体力*/
			this->KILL_ID = -1;						      /*体力*/
			this->DEATH = 0;						      /*体力*/
			this->DEATH_ID = -1;						      /*体力*/
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
	//マップ
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

			//何故か要る
			sendstat(void) {
			}
			//何故か要る
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

		//操作関連//==================================================
		std::array<bool, 18> key{ false };    //キー
		float view_xrad = 0.f, view_yrad = 0.f; //砲塔操作用ベクトル
		//戦車//==================================================
		int hitbuf = 0;		       /*使用弾痕*/
		//飛行機//==================================================
		p_animes p_anime_geardown;		    //車輪アニメーション
		switchs changegear; //ギアアップスイッチ
		float p_landing_per = 0.f;		    //着艦フック
		std::array<p_animes, 6> p_animes_rudder;      //ラダーアニメーション
		std::vector<frames> p_burner;		    //バーナー
		//共通項//==================================================
		vehicles vehicle;
		VECTOR_ref winpos;
		SoundHandle se_cockpit;
		SoundHandle se_gun;
		SoundHandle se_hit;
		//セット
		void set_human(const std::vector<Mainclass::Vehcs>& vehcs, const std::vector<Ammos>& Ammo_, const MV1& hit_pic) {
			auto& c = *this;
			{
				std::fill(c.key.begin(), c.key.end(), false); //操作
				//共通
				{
					auto& veh = c.vehicle;
					{
						veh.init();
						veh.use_id = std::min<size_t>(veh.use_id, vehcs.size() - 1);
						veh.use_veh.into(vehcs[veh.use_id]);
						veh.obj = vehcs[veh.use_id].obj.Duplicate();
						veh.col = vehcs[veh.use_id].col.Duplicate();
						//コリジョン
						for (int j = 0; j < veh.col.mesh_num(); j++) {
							veh.col.SetupCollInfo(8, 8, 8, -1, j);
						}
						veh.hitres.resize(veh.col.mesh_num());   //モジュールごとの当たり判定結果を確保
						veh.hitssort.resize(veh.col.mesh_num()); //モジュールごとの当たり判定順序を確保
						//弾痕
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
						//砲
						veh.Gun_.resize(veh.use_veh.gunframe.size());
						for (auto& cg : veh.Gun_) {
							size_t index = &cg - &veh.Gun_[0];
							cg.gun_info = veh.use_veh.gunframe[index];
							cg.rounds = cg.gun_info.rounds;
							//使用砲弾
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
						//ヒットポイント
						veh.HP = veh.use_veh.HP;
						//モジュール耐久
						veh.HP_m.resize(veh.col.mesh_num());
						for (auto& h : veh.HP_m) {
							h = 100;
						}
						//迷彩
						if (veh.use_veh.camog.size() > 0) {
							veh.camo_sel %= veh.use_veh.camog.size();
							//GraphBlend(MV1GetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex), veh.use_veh.camog[veh.camo_sel], 255, DX_GRAPH_BLEND_NORMAL);
							MV1SetTextureGraphHandle(veh.obj.get(), veh.use_veh.camo_tex, veh.use_veh.camog[veh.camo_sel], FALSE);
						}
					}
				}
				//飛行機
				{
					auto& veh = c.vehicle;
					{
						c.p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
						c.p_anime_geardown.second = 1.f;
						MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_anime_geardown.first, c.p_anime_geardown.second);
						//舵
						for (int i = 0; i < c.p_animes_rudder.size(); i++) {
							c.p_animes_rudder[i].first = MV1AttachAnim(veh.obj.get(), 2 + i);
							c.p_animes_rudder[i].second = 0.f;
							MV1SetAttachAnimBlendRate(veh.obj.get(), c.p_animes_rudder[i].first, c.p_animes_rudder[i].second);
						}
					}
					//エフェクト
					for (auto& be : veh.use_veh.burner) {
						c.p_burner.emplace_back(be);
					}
					c.changegear.first = true;
					c.changegear.second = 2;
				}
			}
		}
		//弾き
		bool get_reco(std::vector<Chara>& tgts, ammos& c) {
			if (c.flug) {
				bool is_hit = false;
				std::optional<size_t> hitnear;
				for (auto& t : tgts) {
					//自分自身は省く
					if (this == &t) {
						continue;
					}
					//とりあえず当たったかどうか探す
					is_hit = false;
					{
						auto& veh_t = t.vehicle;
						//モジュール
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
						//空間装甲
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
						//装甲
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
						//当たってない
						if (!is_hit) {
							continue;
						}
						//
						t.se_hit.play(DX_PLAYTYPE_BACK,TRUE);
						//当たり判定を近い順にソート
						std::sort(veh_t.hitssort.begin(), veh_t.hitssort.end(), [](const pair_hit& x, const pair_hit& y) { return x.second < y.second; });
						//ダメージ面に届くまで判定
						for (auto& tt : veh_t.hitssort) {
							//装甲面に当たらなかったならスルー
							if (tt.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							//当たったら判定
							for (auto& a : veh_t.use_veh.armer_mesh) {
								if (tt.first != a.first) {
									continue;
								}
								hitnear = tt.first;
								//ダメージ面に当たった時に装甲値に勝てるかどうか
								{
									VECTOR_ref normal = veh_t.hitres[hitnear.value()].Normal;
									VECTOR_ref position = veh_t.hitres[hitnear.value()].HitPosition;

									VECTOR_ref vec_t = c.vec;
									//弾処理
									c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
									c.vec = c.vec.Norm();
									c.pos = c.vec * (0.1f) + position;
									//貫通
									if (c.spec.pene_a > a.second * (1.0f / std::abs(vec_t.Norm().dot(normal)))) {
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										veh_t.HP = std::max<int16_t>(veh_t.HP - c.spec.damage_a, 0); //
										//撃破時エフェクト
										if (veh_t.HP == 0) {
											this->vehicle.KILL++;
											this->vehicle.KILL_ID = (int)(&t - &tgts[0]);
											veh_t.DEATH++;
											veh_t.DEATH_ID = (int)(this - &tgts[0]);
											t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
										}
										//弾処理
										c.flug = false;
										//弾痕
										veh_t.hit_obj[t.hitbuf].use = 0;
									}
									//非貫通
									else {
										switch (c.spec.type_a) {
										case 0: //AP
											//はじく
											c.spec.pene_a /= 2.0f;
											break;
										case 1: //HE
											//爆発する
											c.flug = false;
											break;
										case 2: //ミサイル
											//爆発する
											c.flug = false;
											break;
										default:
											break;
										}
										//弾痕
										veh_t.hit_obj[t.hitbuf].use = 1;
									}
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(c.pos, normal);
									}
									else {
										this->effcs[ef_reco2].set(c.pos, normal);
									}
									//弾痕のセット
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
							//はじかれた
							if (hitnear.has_value()) {
								continue;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、貫徹力を1/2に
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
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//爆発する
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
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[tt.first] = std::max<int16_t>(veh_t.HP_m[tt.first] - 30, 0); //
										c.flug = false;//爆発する
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
	//コックピット
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
				if (p.find("座席", 0) != std::string::npos) {
					cockpit_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					compass_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					//ジャイロコンパス
				}
				else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					compass2_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					//ジャイロコンパス
				}
				else if ((p.find("予備コンパス", 0) != std::string::npos)) {
					subcompass_f = { i,cockpit.frame(i) - cockpit.frame(int(cockpit.frame_parent(i))) };
					subcompass2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
					//コンパス
				}
				else if (p.find("スティック縦", 0) != std::string::npos) {
					stickx_f = { i,cockpit.frame(i) };
					stickz_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("ペダル", 0) != std::string::npos) && (p.find("右", 0) == std::string::npos) && (p.find("左", 0) == std::string::npos)) {
					sticky_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("スロットル", 0) != std::string::npos)) {
					accel_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("速度計", 0) != std::string::npos)) {
					speed_f = { i,cockpit.frame(i) };
					speed2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("速度100", 0) != std::string::npos)) {
					spd3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("速度010", 0) != std::string::npos)) {
					spd2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("速度001", 0) != std::string::npos)) {
					spd1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("高度1000", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt4_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0100", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0010", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0001", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("高度1000", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt4_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0100", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt3_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0010", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt2_f = { i,cockpit.frame(i) };
				}
				else if ((p.find("高度0001", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt1_f = { i,cockpit.frame(i) };
				}

				else if ((p.find("高度計", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt_1000_f = { i,cockpit.frame(i) };
					alt_1000_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("高度長針", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt_100_f = { i,cockpit.frame(i) };
					alt_100_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("高度計", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos) && (p.find("長針", 0) == std::string::npos)) {
					salt_1000_f = { i,cockpit.frame(i) };
					salt_1000_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("高度計長針", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt_100_f = { i,cockpit.frame(i) };
					salt_100_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}

				else if ((p.find("燃料計", 0) != std::string::npos)) {
					fuel_f = { i,cockpit.frame(i) };
					fuel_2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}

				else if ((p.find("時計", 0) != std::string::npos)) {
					clock_h_f = { i,cockpit.frame(i) };
					clock_h2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("分針", 0) != std::string::npos)) {
					clock_m_f = { i,cockpit.frame(i) };
					clock_m2_f = { i + 1,cockpit.frame(i + 1) - cockpit.frame(i) };
				}
				else if ((p.find("秒針", 0) != std::string::npos)) {
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
			//速度計
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
			//時計
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
			//コンパス
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
