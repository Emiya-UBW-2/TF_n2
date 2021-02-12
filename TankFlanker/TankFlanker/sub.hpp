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
	//ef_smoke3 = 9, //飛行機の軌跡
	ef_size
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

		void set(int& mdata) {
			this->name_a = getparams::_str(mdata);
			this->type_a = uint16_t(getparams::_ulong(mdata)); //ap=0,he=1
			this->caliber_a = getparams::_float(mdata);
			this->pene_a = getparams::_float(mdata);
			this->speed_a = getparams::_float(mdata);
			this->damage_a = uint16_t(getparams::_ulong(mdata));
		}
		static void set_ammos(std::vector<Ammos>* Ammo_) {
			int mdata = FileRead_open("data/ammo/ammo.txt", FALSE);
			while (true) {
				Ammo_->resize(Ammo_->size() + 1);
				Ammo_->back().set(mdata);
				if (getparams::get_str(mdata).find("end") != std::string::npos) {
					break;
				}
			}
			FileRead_close(mdata);
		}
	};
	//車輛
	class Chara;

	class Vehcs {
	public:

		class foot_frame {
		public:
			frames frame;
			EffectS gndsmkeffcs;
			void init() {
				this->gndsmkeffcs.scale = 0.1f;
			}
			template<class Y, class D>
			void math(std::unique_ptr<Y, D>& mapparts , Chara* c,bool *hit_f) {
				auto& veh = c->vehicle;
				easing_set(&this->gndsmkeffcs.scale, 0.01f, 0.9f);
				auto tmp = veh.obj.frame(int(this->frame.first + 1)) - VGet(0.f, 0.2f, 0.f);
				//地面
				{
					auto hp = mapparts->map_col_line(tmp + (veh.mat.yvec() * (0.5f)), tmp, 0);
					if (hp.HitFlag == TRUE) {
						veh.add = (VECTOR_ref(hp.HitPosition) - tmp);
						{
							auto normal = veh.mat.yvec();
							easing_set(&normal, hp.Normal, 0.95f);
							veh.mat *= MATRIX_ref::RotVec2(veh.mat.yvec(), normal);
						}
						this->gndsmkeffcs.scale = std::clamp(veh.speed * 3.6f / 50.f, 0.1f, 1.f);
						if (!*hit_f) {
							if (veh.speed >= 0.f && (c->key[11])) {
								veh.speed += -1.f / 3.6f * 60.f / GetFPS();
							}
							if (veh.speed <= 0.f) {
								easing_set(&veh.speed, 0.f, 0.9f);
							}
							*hit_f = true;
						}
					}
				}
			}
		};
		struct wing_frame {
			frames frame;
			EffectS smkeffcs;
		};
		//共通
		std::string name;				  //
		MV1 obj, col;					  //
		std::vector<GraphHandle> graph_HP_m;		//ライフ
		GraphHandle graph_HP_m_all;
		VECTOR_ref minpos, maxpos;			  //
		std::vector<gun_frame> gunframe;			  //
		std::vector<foot_frame> wheelframe;			  //
		std::vector<foot_frame> wheelframe_nospring;		  //誘導輪回転
		std::vector< wing_frame> wingframe;
		uint16_t HP = 0;					  //
		std::vector<std::pair<size_t, float>> armer_mesh; //装甲ID
		std::vector<size_t> space_mesh;			  //装甲ID
		std::vector<std::pair<size_t, size_t>> module_mesh;		  //装甲ID
		int camo_tex = 0;				  //
		std::vector<int> camog;				  //
		bool isfloat = false;			  //浮くかどうか
		float down_in_water = 0.f;			  //沈む判定箇所
		float max_speed_limit = 0.f;			  //最高速度(km/h)
		float mid_speed_limit = 0.f;			  //巡行速度(km/h)
		float min_speed_limit = 0.f;			  //失速速度(km/h)
		float flont_speed_limit = 0.f;			  //前進速度(km/h)
		float back_speed_limit = 0.f;			  //後退速度(km/h)
		float body_rad_limit = 0.f;			  //旋回速度(度/秒)
		float turret_rad_limit = 0.f;			  //砲塔駆動速度(度/秒)
		frames fps_view;//コックピット
		GraphHandle ui_pic;//シルエット
		int pic_x, pic_y;//サイズ
		//専門
		std::array<int, 4> square{ 0 };//車輛の四辺
		std::array<std::vector<frames>, 2> b2upsideframe; //履帯上
		std::array<std::vector<frames>, 2> b2downsideframe; //履帯上
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
			this->wingframe.clear();
			for (auto& p : t.wingframe) {
				this->wingframe.resize(this->wingframe.size() + 1);
				this->wingframe.back().frame = p.frame;
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
		static void set_vehicles_pre(const char* name, std::vector<Vehcs>* veh_, const bool& Async) {
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
				//todo
				t.graph_HP_m.resize(7);
				for (int i = 0; i < 7; i++) {
					t.graph_HP_m[i] = GraphHandle::Load(std::string(name) + t.name + "/parts" + std::to_string(i + 1) + ".png");
				}
				t.graph_HP_m_all = GraphHandle::Load(std::string(name) + t.name + "/parts_all.png");
			}
		}
		//メイン読み込み
		static void set_vehicles(std::vector<Vehcs>* vehcs) {
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
					else if (p.find("センター", 0) != std::string::npos) {
						t.wingframe.resize(t.wingframe.size() + 1);
						t.wingframe.back().frame.first = i;
						t.wingframe.back().frame.second = t.obj.frame(t.wingframe.back().frame.first);
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
					else if (p.find("space", 0) != std::string::npos) {	//空間装甲
						t.space_mesh.resize(t.space_mesh.size() + 1);
						t.space_mesh.back() = i;
					}
					else if (p.find("parts", 0) != std::string::npos) {	//モジュール
						t.module_mesh.resize(t.module_mesh.size() + 1);
						t.module_mesh.back().first = i;
					}
				}
				{
					size_t z = 0;
					for (int i = 0; i < t.obj.mesh_num(); i++) {
						std::string p = t.obj.material_name(i);
						if (p.find("parts", 0) != std::string::npos) {	//モジュール
							t.module_mesh[z].second = i;
							z++;
						}
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
					char mstr[64]; //tank
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
	//
	struct ammos {
		bool hit{ false };
		bool flug{ false };
		float count = 0.f;
		unsigned int color = 0;
		Ammos spec;
		float yadd = 0.f;
		VECTOR_ref pos, repos, vec;
	};
	//
public:
	class Chara;

public:
	class Chara;
private:
	class vehicles;

	class Guns {							//
		std::vector<Ammos> Spec;				//
	public:
		size_t usebullet{};					//使用弾
		int16_t rounds{ 0 };					//弾数
		gun_frame gun_info;						//
		float loadcnt{ 0 };					//装てんカウンター
		std::array<ammos, 64> bullet;				//確保する弾

		void clear() {
			this->usebullet = 0;
			this->loadcnt = 0.f;
			this->rounds = 0;
			this->Spec.clear();
		}
		void init(const gun_frame& gunf, const std::vector<Ammos>& Ammo_) {
			this->gun_info = gunf;
			//使用砲弾
			this->Spec.clear();
			for (auto& pa : Ammo_) {
				if (pa.name_a.find(this->gun_info.useammo[0]) != std::string::npos) {
					this->Spec.emplace_back(pa);
					break;
				}
			}
			for (auto& p : this->bullet) {
				p.color = GetColor(255, 255, 172);
				p.spec = this->Spec[0];
			}
		}
		void set() {
			this->rounds = this->gun_info.rounds;
		}
		void draw(const VECTOR_ref& position) {
			for (auto& a : this->bullet) {
				if (a.flug) {
					DXDraw::Capsule3D(a.pos, a.repos, (((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - position).size() / 24.f))*4.5f, a.color, GetColor(255, 255, 255));
				}
			}
		}
		void math(const bool& key,Chara* c) {
			if (key && this->loadcnt == 0 && this->rounds > 0) {
				auto& u = this->bullet[this->usebullet];
				++this->usebullet %= this->bullet.size();
				//ココだけ変化
				u.spec = this->Spec[0];
				u.spec.speed_a *= float(75 + GetRand(50)) / 100.f;
				u.pos = c->vehicle.obj.frame(this->gun_info.frame2.first);
				u.vec = (c->vehicle.obj.frame(this->gun_info.frame3.first) - c->vehicle.obj.frame(this->gun_info.frame2.first)).Norm();
				//
				this->loadcnt = this->gun_info.load_time;
				this->rounds = std::max<uint16_t>(this->rounds - 1, 0);
				u.hit = false;
				u.flug = true;
				u.count = 0.f;
				u.yadd = 0.f;
				u.repos = u.pos;
				if (u.spec.type_a != 2) {
					c->effcs[ef_fire].set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
					if (u.spec.caliber_a >= 0.017f) {
						c->effcs_gun[c->gun_effcnt].first.set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec);
						c->effcs_gun[c->gun_effcnt].second = &u;
						c->effcs_gun[c->gun_effcnt].count = 0.f;
						++c->gun_effcnt %= c->effcs_gun.size();
					}
					c->se_gun.play(DX_PLAYTYPE_BACK, TRUE);
				}
				else {
					c->effcs_missile[c->missile_effcnt].first.set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec);
					c->effcs_missile[c->missile_effcnt].second = &u;
					c->effcs_missile[c->missile_effcnt].count = 0.f;
					++c->missile_effcnt %= c->effcs_missile.size();

					c->se_missile.play(DX_PLAYTYPE_BACK, TRUE);
				}
			}
			this->loadcnt = std::max(this->loadcnt - 1.f / GetFPS(), 0.f);
		}
		//要改善
		template<class Y, class D>
		void math_reco(std::unique_ptr<Y, D>& mapparts,Chara* c, std::vector<Chara>* chara) {
			for (auto& a : this->bullet) {
				float size = 3.f;
				for (int z = 0; z < int(size); z++) {
					if (a.flug) {
						a.repos = a.pos;
						a.pos += a.vec * (a.spec.speed_a / GetFPS() / size);
						//判定
						{
							bool ground_hit = false;
							VECTOR_ref normal;
							//機体以外に当たる
							for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
								auto hps = mapparts->map_col_line(a.repos, a.pos, i);
								if (hps.HitFlag) {
									a.pos = hps.HitPosition;
									normal = hps.Normal;
									ground_hit = true;
								}
							}
							//必要な時に当たり判定をリフレッシュする
							for (auto& t : *chara) {
								auto& veh_t = t.vehicle;
								if (c == &t || veh_t.hit_check) {
									continue;
								}
								if ((Segment_Point_MinLen(a.pos, a.repos, veh_t.pos) > 10.f)) {
									continue;
								}
								veh_t.col.SetMatrix((MATRIX_ref)(veh_t.mat) * MATRIX_ref::Mtrans(veh_t.pos));
								for (int i = 0; i < veh_t.col.mesh_num(); i++) {
									if (veh_t.HP_m[i] > 0) {
										veh_t.col.RefreshCollInfo(-1, i);
									}
								}
								veh_t.hit_check = true;
							}
							//飛行機にあたる
							auto hitplane = c->get_reco(*chara, a);
							//その後処理
							if (!hitplane) {
								if (ground_hit) {
									if (a.spec.caliber_a >= 0.020f) {
										c->effcs[ef_gndhit].set(a.pos + normal * (0.1f), normal);
									}
									else {
										c->effcs[ef_gndhit2].set(a.pos + normal * (0.1f), normal);
									}
									switch (a.spec.type_a) {
									case 0: //AP
										if ((a.vec.Norm().dot(normal)) <= cos(deg2rad(60))) {
											a.flug = false;
										}
										else {
											a.vec += normal * ((a.vec.dot(normal)) * -2.f);
											a.vec = a.vec.Norm();
											a.pos += a.vec * (0.01f);
											a.spec.pene_a /= 2.f;
										}
										break;
									case 1: //HE
										a.flug = false;
										break;
									case 2: //ミサイル
										a.flug = false;
										break;
									default:
										break;
									}
								}
							}
							if (a.flug) {
								switch (a.spec.type_a) {
								case 0: //AP
								{
									a.spec.pene_a -= 1.0f / GetFPS() / size;
									a.spec.speed_a -= 5.f / GetFPS() / size;
									a.pos += VGet(0.f, a.yadd / size, 0.f);
								}
								break;
								case 1: //HE
								{
									a.spec.speed_a -= 5.f / GetFPS() / size;
									a.pos += VGet(0.f, a.yadd / size, 0.f);
								}
								break;
								case 2: //ミサイル
								{
									size_t id = chara->size();
									VECTOR_ref pos;
									float dist = (std::numeric_limits<float>::max)();
									for (auto& t : *chara) {
										//弾関連
										if (c == &t || c->id == t.id) {
											continue;
										}
										auto p = (t.vehicle.pos - a.pos).size();
										if (dist > p) {
											dist = p;
											id = &t - &(*chara)[0];
											pos = t.vehicle.pos+ (t.vehicle.mat.zvec() * (-t.vehicle.speed / GetFPS()))*((a.pos - pos).size()/ (a.spec.speed_a));
										}
									}
									if (id != chara->size()) {
										//反映
										auto vec_a = (a.pos - pos).Norm();
										auto vec_z = a.vec;
										if (vec_a.dot(vec_z) < 0 && (a.pos - pos).size() <= 2000.f) {
											float z_hyp = std::hypotf(vec_z.x(), vec_z.z());
											float a_hyp = std::hypotf(vec_a.x(), vec_a.z());
											float cost = (vec_a.z() * vec_z.x() - vec_a.x() * vec_z.z()) / (a_hyp * z_hyp);
											float view_yrad = (atan2f(cost, sqrtf(std::abs(1.f - cost * cost)))) / 5.f; //cos取得2D
											float view_xrad = (atan2f(-vec_z.y(), z_hyp) - atan2f(vec_a.y(), a_hyp)) / 5.f;
											{
												float limit = deg2rad(22.5f) / GetFPS();
												float y = atan2f(a.vec.x(), a.vec.z()) + std::clamp(view_yrad, -limit, limit);
												float x = atan2f(a.vec.y(), std::hypotf(a.vec.x(), a.vec.z())) + std::clamp(view_xrad, -limit, limit);
												a.vec = VGet(cos(x) * sin(y), sin(x), cos(x) * cos(y));
											}
										}
									}
								}
								break;
								default:
									break;
								}
							}
						}

						//消す(2秒たった、スピードが100以下、貫通が0以下)
						if (a.count >= 2.f || a.spec.speed_a < 100.f || a.spec.pene_a <= 0.f) {
							a.flug = false;
						}
						if (!a.flug) {
							for (auto& b : c->effcs_gun) {
								if (b.second == &a) {
									b.first.handle.SetPos(b.second->pos);
									break;
								}
							}
						}
					}
				}
				a.yadd += M_GR / powf(GetFPS(), 2.f);
				a.count += 1.f / GetFPS();
			}
		}
	};								//
	class pair_hit {							//
	public:
		size_t first = 0;
		float second = 0.f;
		void clear() {
			this->first = 0;
			this->second = 0.f;
		}
	};
	class hit_data {							//
	public:
		MV1_COLL_RESULT_POLY res;
		pair_hit sort;
	};
	class vehicles {
	public:
		Vehcs use_veh;						//固有値
		MV1 obj, obj_break;					//
		MV1 col;							//
		bool hit_check = false;						//当たり判定を取るかチェック
		size_t use_id = 0;						//使用する車両(機材)
		uint16_t HP = 0;						//体力
		uint16_t KILL = 0;						//体力
		int KILL_ID = -1;						//体力
		uint16_t DEATH = 0;						//体力
		int DEATH_ID = -1;						//体力
		VECTOR_ref pos;							//車体座標
		MATRIX_ref mat;							//車体回転行列
		MATRIX_ref mat_start;					//車体回転行列(初期配置)
		VECTOR_ref add;							//車体加速度
		std::vector<Guns> Gun_;						//
		float accel = 0.f, accel_add = 0.f;
		float WIP_timer_limit = 0.f;
		float WIP_timer = 0.f;
		bool over_heat = false;
		float speed = 0.f, speed_add = 0.f;		//
		float xradadd_left = 0.f, xradadd_right = 0.f; //
		float yradadd_left = 0.f, yradadd_right = 0.f;	    //
		float zradadd_left = 0.f, zradadd_right = 0.f; //
		std::vector<int16_t> HP_m;					//ライフ
		std::vector<GraphHandle> graph_HP_m;		//ライフ
		GraphHandle graph_HP_m_all;
		struct breaks{
			VECTOR_ref pos;							//車体座標
			MATRIX_ref mat;							//車体回転行列
			VECTOR_ref add;							//車体加速度
			float per = 1.f;
			float speed = 0.f;
		};
		std::vector<breaks> info_break;					//ライフ

		size_t camo_sel = 0;						//
		float wheel_Left = 0.f, wheel_Right = 0.f;			//転輪回転
		float wheel_Leftadd = 0.f, wheel_Rightadd = 0.f;		//転輪回転

		std::vector<hit_data> hits;
		//std::vector<MV1_COLL_RESULT_POLY> hitres;			//確保
		//std::vector<pair_hit> hitssort;					//フレームに当たった順番

		void reset() {
			this->xradadd_right = 0.f;
			this->xradadd_left = 0.f;
			this->yradadd_left = 0.f;
			this->yradadd_right = 0.f;
			this->zradadd_right = 0.f;
			this->zradadd_left = 0.f;

			this->WIP_timer_limit = 0.f;
			this->WIP_timer = 0.f;

			this->over_heat = false;

			this->accel_add = 0.f;
			this->accel = 0.f;
			this->speed_add = 0.f;
			this->speed = 0.f;

			this->add.clear();
		}
	public:
		void init(const Vehcs& vehcs, const std::vector<Ammos>& Ammo_) {
			//
			this->Dispose();
			//
			this->use_veh.into(vehcs);
			//
			this->obj = vehcs.obj.Duplicate();
			this->obj_break = vehcs.obj.Duplicate();
			this->col = vehcs.col.Duplicate();
			//コリジョン
			for (int j = 0; j < this->col.mesh_num(); j++) {
				this->col.SetupCollInfo(8, 8, 8, -1, j);
			}
			this->hits.resize(this->col.mesh_num());
			//弾痕
			for (int j = 0; j < this->obj.material_num(); ++j) {
				MV1SetMaterialSpcColor(this->obj.get(), j, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
				MV1SetMaterialSpcPower(this->obj.get(), j, 50.0f);
			}
			for (int j = 0; j < this->obj_break.material_num(); ++j) {
				MV1SetMaterialSpcColor(this->obj_break.get(), j, GetColorF(0.85f, 0.82f, 0.78f, 0.1f));
				MV1SetMaterialSpcPower(this->obj_break.get(), j, 50.0f);
			}
			//モジュール耐久
			this->HP_m.resize(this->col.mesh_num());
			for (auto& p : vehcs.graph_HP_m) {
				this->graph_HP_m.resize(this->graph_HP_m.size() + 1);
				this->graph_HP_m.back() = p.Duplicate();
			}
			graph_HP_m_all = vehcs.graph_HP_m_all.Duplicate();
			this->info_break.resize(this->col.mesh_num());
			//迷彩
			if (this->use_veh.camog.size() > 0) {
				this->camo_sel %= this->use_veh.camog.size();
				//GraphBlend(MV1GetTextureGraphHandle(this->obj.get(), this->use_veh.camo_tex), this->use_veh.camog[this->camo_sel], 255, DX_GRAPH_BLEND_NORMAL);
				MV1SetTextureGraphHandle(this->obj.get(), this->use_veh.camo_tex, this->use_veh.camog[this->camo_sel], FALSE);
				//GraphBlend(MV1GetTextureGraphHandle(this->obj_break.get(), this->use_veh.camo_tex), this->use_veh.camog[this->camo_sel], 255, DX_GRAPH_BLEND_NORMAL);
				MV1SetTextureGraphHandle(this->obj_break.get(), this->use_veh.camo_tex, this->use_veh.camog[this->camo_sel], FALSE);
			}
			//砲
			this->Gun_.resize(this->use_veh.gunframe.size());
			for (auto& cg : this->Gun_) {
				cg.init(this->use_veh.gunframe[&cg - &this->Gun_[0]], Ammo_);
			}
			spawn(VGet(0, 0, 0), MGetIdent());
		}
		void Dispose() {
			this->obj.Dispose();
			this->obj_break.Dispose();
			this->col.Dispose();
			this->hit_check = false;
			this->HP = 0;
			this->KILL = 0;						//体力
			this->KILL_ID = -1;						//体力
			this->DEATH = 0;						//体力
			this->DEATH_ID = -1;						//体力

			for (auto& hz : this->hits) {
				hz.sort.clear();
			}
			this->hits.clear();
			for (auto& p : this->graph_HP_m) {
				p.Dispose();
			}
			this->graph_HP_m_all.Dispose();
			this->HP_m.clear();
			this->graph_HP_m.clear();
			this->wheel_Left = 0.f;
			this->wheel_Right = 0.f;
			this->wheel_Leftadd = 0.f;
			this->wheel_Rightadd = 0.f;
			for (auto& cg : this->Gun_) { cg.clear(); }
			this->Gun_.clear();

			this->reset();
		}
		void spawn(const VECTOR_ref& pos_, const MATRIX_ref& mat_) {
			this->reset();

			this->accel = 30.f;
			this->speed = 300.f / 3.6f;
			//リセット
			this->pos = pos_;
			this->mat = mat_;
			this->mat_start = this->mat;
			//砲
			for (auto& cg : this->Gun_) {
				cg.set();
			}
			//ヒットポイント
			this->HP = this->use_veh.HP;
			//モジュール耐久
			for (auto& h : this->HP_m) {
				h = this->use_veh.HP;
			}
			if (this->HP_m.size() > 2) {
				this->HP_m[0] = 1;
				this->HP_m[1] = 1;
				this->HP_m[2] = 1;
			}
		}
	};
public:
	//
	struct ef_guns {
		EffectS first;
		ammos* second = nullptr;
		bool n_l;
		bool flug;
		float count = -1.f;
	};
	//カメラ
	struct CAMS {
		cam_info cam;
		int Rot = 0;//
	};
	typedef std::pair<int, float> p_animes;
	class Chara;
	//コックピット
	class cockpits {
	public:
		frames	stickx_f, sticky_f, stickz_f, compass_f, compass2_f, speed_f, speed2_f, cockpit_f, clock_h_f, clock_h2_f, clock_m_f, clock_m2_f, clock_s_f, clock_s2_f, subcompass_f, subcompass2_f
			, spd3_f, spd2_f, spd1_f
			, alt4_f, alt3_f, alt2_f, alt1_f
			, salt4_f, salt3_f, salt2_f, salt1_f
			, alt_1000_f, alt_1000_2_f, alt_100_f, alt_100_2_f
			, salt_1000_f, salt_1000_2_f, salt_100_f, salt_100_2_f
			, fuel_f, fuel_2_f
			, accel_f;

		float spd3_fp = 0.f, spd2_fp = 0.f, spd1_fp = 0.f
			, alt4_fp = 0.f, alt3_fp = 0.f, alt2_fp = 0.f, alt1_fp = 0.f
			, salt4_fp = 0.f, salt3_fp = 0.f, salt2_fp = 0.f, salt1_fp = 0.f;

		MV1 obj;

		void set_(const MV1& cocks) {
			obj = cocks.Duplicate();
			//
			for (int i = 0; i < obj.frame_num(); i++) {
				std::string p = obj.frame_name(i);
				if (p.find("座席", 0) != std::string::npos) {
					cockpit_f = { i,obj.frame(i) };
				}
				else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					compass_f = { i,obj.frame(i) - obj.frame(int(obj.frame_parent(i))) };
					//ジャイロコンパス
				}
				else if ((p.find("姿勢指示器", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					compass2_f = { i,obj.frame(i) - obj.frame(int(obj.frame_parent(i))) };
					//ジャイロコンパス
				}
				else if ((p.find("予備コンパス", 0) != std::string::npos)) {
					subcompass_f = { i,obj.frame(i) - obj.frame(int(obj.frame_parent(i))) };
					subcompass2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
					//コンパス
				}
				else if (p.find("スティック縦", 0) != std::string::npos) {
					stickx_f = { i,obj.frame(i) };
					stickz_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("ペダル", 0) != std::string::npos) && (p.find("右", 0) == std::string::npos) && (p.find("左", 0) == std::string::npos)) {
					sticky_f = { i,obj.frame(i) };
				}
				else if ((p.find("スロットル", 0) != std::string::npos)) {
					accel_f = { i,obj.frame(i) };
				}
				else if ((p.find("速度計", 0) != std::string::npos)) {
					speed_f = { i,obj.frame(i) };
					speed2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("速度100", 0) != std::string::npos)) {
					spd3_f = { i,obj.frame(i) };
				}
				else if ((p.find("速度010", 0) != std::string::npos)) {
					spd2_f = { i,obj.frame(i) };
				}
				else if ((p.find("速度001", 0) != std::string::npos)) {
					spd1_f = { i,obj.frame(i) };
				}

				else if ((p.find("高度1000", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt4_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0100", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt3_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0010", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt2_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0001", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt1_f = { i,obj.frame(i) };
				}

				else if ((p.find("高度1000", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt4_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0100", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt3_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0010", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt2_f = { i,obj.frame(i) };
				}
				else if ((p.find("高度0001", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt1_f = { i,obj.frame(i) };
				}

				else if ((p.find("高度計", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt_1000_f = { i,obj.frame(i) };
					alt_1000_2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("高度長針", 0) != std::string::npos) && (p.find("予備", 0) == std::string::npos)) {
					alt_100_f = { i,obj.frame(i) };
					alt_100_2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("高度計", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos) && (p.find("長針", 0) == std::string::npos)) {
					salt_1000_f = { i,obj.frame(i) };
					salt_1000_2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("高度計長針", 0) != std::string::npos) && (p.find("予備", 0) != std::string::npos)) {
					salt_100_f = { i,obj.frame(i) };
					salt_100_2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}

				else if ((p.find("燃料計", 0) != std::string::npos)) {
					fuel_f = { i,obj.frame(i) };
					fuel_2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}

				else if ((p.find("時計", 0) != std::string::npos)) {
					clock_h_f = { i,obj.frame(i) };
					clock_h2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("分針", 0) != std::string::npos)) {
					clock_m_f = { i,obj.frame(i) };
					clock_m2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
				else if ((p.find("秒針", 0) != std::string::npos)) {
					clock_s_f = { i,obj.frame(i) };
					clock_s2_f = { i + 1,obj.frame(i + 1) - obj.frame(i) };
				}
			}
		}
		void ready_(Chara& c) {
			float px = (c.p_animes_rudder[1].second - c.p_animes_rudder[0].second)*deg2rad(30);
			float pz = (c.p_animes_rudder[2].second - c.p_animes_rudder[3].second)*deg2rad(30);
			float py = (c.p_animes_rudder[5].second - c.p_animes_rudder[4].second)*deg2rad(20);
			//操縦桿
			obj.SetFrameLocalMatrix(sticky_f.first, MATRIX_ref::RotY(py) * MATRIX_ref::Mtrans(sticky_f.second));
			obj.SetFrameLocalMatrix(stickz_f.first, MATRIX_ref::RotZ(pz) * MATRIX_ref::Mtrans(stickz_f.second));
			obj.SetFrameLocalMatrix(stickx_f.first, MATRIX_ref::RotX(px) * MATRIX_ref::Mtrans(stickx_f.second));
			//ジャイロコンパス
			obj.SetFrameLocalMatrix(compass_f.first, c.vehicle.mat.Inverse() * MATRIX_ref::Mtrans(compass_f.second));
			obj.SetFrameLocalMatrix(compass2_f.first, c.vehicle.mat.Inverse() * MATRIX_ref::Mtrans(compass2_f.second));
			//スロットル
			{
				float accel_ = c.vehicle.accel;
				obj.SetFrameLocalMatrix(accel_f.first, MATRIX_ref::RotX(deg2rad(30.f - (accel_ / 100.f)*60.f)) * MATRIX_ref::Mtrans(accel_f.second));
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
					obj.frame_reset(speed_f.first);
					obj.SetFrameLocalMatrix(speed_f.first, MATRIX_ref::RotAxis(speed2_f.second, -deg2rad(spd)) * MATRIX_ref::Mtrans(speed_f.second));
				}
				{
					float spd_buf = c.vehicle.speed*3.6f / 1224.f*100.f;
					easing_set(&spd3_fp, deg2rad(360.f / 10.f*(int)(spd_buf / 100.f)), 0.9f);
					easing_set(&spd2_fp, deg2rad(360.f / 10.f*(int)(spd_buf / 10.f)), 0.9f);
					easing_set(&spd1_fp, deg2rad(360.f / 10.f*(int)(spd_buf / 1.f)), 0.9f);
					obj.SetFrameLocalMatrix(spd3_f.first, MATRIX_ref::RotX(-spd3_fp) * MATRIX_ref::Mtrans(spd3_f.second));
					obj.SetFrameLocalMatrix(spd2_f.first, MATRIX_ref::RotX(-spd2_fp) * MATRIX_ref::Mtrans(spd2_f.second));
					obj.SetFrameLocalMatrix(spd1_f.first, MATRIX_ref::RotX(-spd1_fp) * MATRIX_ref::Mtrans(spd1_f.second));
				}
				//高度計
				{
					float alt_buf = c.vehicle.pos.y();
					{
						easing_set(&alt4_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 1000.f)), 0.9f);
						easing_set(&alt3_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 100.f)), 0.9f);
						easing_set(&alt2_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 10.f)), 0.9f);
						easing_set(&alt1_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 1.f)), 0.9f);

						obj.SetFrameLocalMatrix(alt4_f.first, MATRIX_ref::RotX(-alt4_fp) * MATRIX_ref::Mtrans(alt4_f.second));
						obj.SetFrameLocalMatrix(alt3_f.first, MATRIX_ref::RotX(-alt3_fp) * MATRIX_ref::Mtrans(alt3_f.second));
						obj.SetFrameLocalMatrix(alt2_f.first, MATRIX_ref::RotX(-alt2_fp) * MATRIX_ref::Mtrans(alt2_f.second));
						obj.SetFrameLocalMatrix(alt1_f.first, MATRIX_ref::RotX(-alt1_fp) * MATRIX_ref::Mtrans(alt1_f.second));
					}
					{
						obj.frame_reset(alt_1000_f.first);
						obj.SetFrameLocalMatrix(alt_1000_f.first, MATRIX_ref::RotAxis(alt_1000_2_f.second, -deg2rad(360.f *alt_buf / 1000)) * MATRIX_ref::Mtrans(alt_1000_f.second));
						obj.frame_reset(alt_100_f.first);
						obj.SetFrameLocalMatrix(alt_100_f.first, MATRIX_ref::RotAxis(alt_100_2_f.second, -deg2rad(360.f *alt_buf / 100)) * MATRIX_ref::Mtrans(alt_100_f.second));
					}
					{
						easing_set(&salt4_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 1000.f)), 0.9f);
						easing_set(&salt3_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 100.f)), 0.9f);
						easing_set(&salt2_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 10.f)), 0.9f);
						easing_set(&salt1_fp, deg2rad(360.f / 10.f*(int)(alt_buf / 1.f)), 0.9f);

						obj.SetFrameLocalMatrix(salt4_f.first, MATRIX_ref::RotX(-salt4_fp) * MATRIX_ref::Mtrans(salt4_f.second));
						obj.SetFrameLocalMatrix(salt3_f.first, MATRIX_ref::RotX(-salt3_fp) * MATRIX_ref::Mtrans(salt3_f.second));
						obj.SetFrameLocalMatrix(salt2_f.first, MATRIX_ref::RotX(-salt2_fp) * MATRIX_ref::Mtrans(salt2_f.second));
						obj.SetFrameLocalMatrix(salt1_f.first, MATRIX_ref::RotX(-salt1_fp) * MATRIX_ref::Mtrans(salt1_f.second));
					}
					{
						obj.frame_reset(salt_1000_f.first);
						obj.SetFrameLocalMatrix(salt_1000_f.first, MATRIX_ref::RotAxis(salt_1000_2_f.second, -deg2rad(360.f *alt_buf / 1000)) * MATRIX_ref::Mtrans(salt_1000_f.second));
						obj.frame_reset(salt_100_f.first);
						obj.SetFrameLocalMatrix(salt_100_f.first, MATRIX_ref::RotAxis(salt_100_2_f.second, -deg2rad(360.f *alt_buf / 100)) * MATRIX_ref::Mtrans(salt_100_f.second));
					}
				}
				{
					float fuel_buf = 1.f;

					obj.frame_reset(fuel_f.first);
					obj.SetFrameLocalMatrix(fuel_f.first, MATRIX_ref::RotAxis(fuel_2_f.second, -deg2rad(300.f * fuel_buf)) * MATRIX_ref::Mtrans(fuel_f.second));
				}

			}
			//時計
			{
				DATEDATA DateBuf;
				GetDateTime(&DateBuf);
				obj.frame_reset(clock_h_f.first);
				obj.SetFrameLocalMatrix(clock_h_f.first, MATRIX_ref::RotAxis(clock_h2_f.second, -deg2rad(360.f *DateBuf.Hour / 12 + 360.f / 12.f*DateBuf.Min / 60)) * MATRIX_ref::Mtrans(clock_h_f.second));
				obj.frame_reset(clock_m_f.first);
				obj.SetFrameLocalMatrix(clock_m_f.first, MATRIX_ref::RotAxis(clock_m2_f.second, -deg2rad(360.f *DateBuf.Min / 60 + 360.f / 60.f*DateBuf.Sec / 60)) * MATRIX_ref::Mtrans(clock_m_f.second));
				obj.frame_reset(clock_s_f.first);
				obj.SetFrameLocalMatrix(clock_s_f.first, MATRIX_ref::RotAxis(clock_s2_f.second, -deg2rad(360.f *DateBuf.Sec / 60)) * MATRIX_ref::Mtrans(clock_s_f.second));
			}
			//コンパス
			{
				VECTOR_ref tmp = c.vehicle.mat.zvec();
				tmp = VGet(tmp.x(), 0.f, tmp.z());
				tmp = tmp.Norm();

				obj.frame_reset(subcompass_f.first);
				obj.SetFrameLocalMatrix(subcompass_f.first, MATRIX_ref::RotAxis(subcompass2_f.second, std::atan2f(tmp.z(), -tmp.x())) * MATRIX_ref::Mtrans(subcompass_f.second));
			}
			obj.SetMatrix(c.vehicle.mat*MATRIX_ref::Mtrans(c.vehicle.obj.frame(c.vehicle.use_veh.fps_view.first) - MATRIX_ref::Vtrans(cockpit_f.second, c.vehicle.mat)));
		}
	};
	//player
	class Chara {
	public:
		class sendstat {

		public:
			MATRIX_ref v_mat;
			VECTOR_ref pos;
			MATRIX_ref mat;
			std::vector<Guns> Gun_;						//
			float speed;
			struct eff_buf {
				bool flug{ false };				 //
				VECTOR_ref pos;					 //
				VECTOR_ref nor;					 //
				float scale = 1.f;				 //
			};
			std::array<eff_buf, ef_size> effcs_; //effect
			std::array<ef_guns, 8> effcs_missile_; //effect
			std::array<ef_guns, 12> effcs_gun_;    //effect
			std::array<float, 3> gndsmkeffcs_; //effect
			p_animes p_anime_geardown;		    //車輪アニメーション
			std::array<p_animes, 6> p_animes_rudder;//ラダーアニメーション
			float wheel_Left = 0.f, wheel_Right = 0.f;			//転輪回転
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

				this->p_anime_geardown = data.p_anime_geardown;		    //車輪アニメーション
				this->p_animes_rudder = data.p_animes_rudder;//ラダーアニメーション

				this->wheel_Left = veh.wheel_Left;
				this->wheel_Right = veh.wheel_Right;


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

				data.p_anime_geardown = this->p_anime_geardown;		    //車輪アニメーション
				data.p_animes_rudder = this->p_animes_rudder;//ラダーアニメーション
				veh.wheel_Left = this->wheel_Left;
				veh.wheel_Right = this->wheel_Right;

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
				fout.write((char *)&this->p_anime_geardown, sizeof(this->p_anime_geardown));
				fout.write((char *)&this->p_animes_rudder, sizeof(this->p_animes_rudder));

				fout.write((char *)&this->wheel_Left, sizeof(this->wheel_Left));
				fout.write((char *)&this->wheel_Right, sizeof(this->wheel_Right));

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
			void read(std::ifstream& fout) {
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
				fout.read((char *)&this->p_anime_geardown, sizeof(this->p_anime_geardown));
				fout.read((char *)&this->p_animes_rudder, sizeof(this->p_animes_rudder));

				fout.read((char *)&this->wheel_Left, sizeof(this->wheel_Left));
				fout.read((char *)&this->wheel_Right, sizeof(this->wheel_Right));

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
		};
		std::list<sendstat> rep;
		//====================================================
		std::array<EffectS, ef_size> effcs; //effect
		std::array<ef_guns, 8> effcs_missile; //effect
		std::array<ef_guns, 12> effcs_gun;    //effect
		size_t missile_effcnt = 0;
		size_t gun_effcnt = 0;
		size_t id;
		//操作関連//==================================================
		std::array<bool, 18> key{ false };    //キー
		float view_xrad = 0.f, view_yrad = 0.f; //砲塔操作用ベクトル
		bool ms_on = true;
		bool ms_key = true;
		float ms_cnt = 0.f;
		//飛行機//==================================================
		p_animes p_anime_geardown;		    //車輪アニメーション
		switchs changegear; //ギアアップスイッチ
		std::array<p_animes, 6> p_animes_rudder;//ラダーアニメーション
		std::vector<frames> p_burner;		    //バーナー
		//共通項//==================================================
		vehicles vehicle;
		VECTOR_ref winpos;
		SoundHandle se_cockpit;
		SoundHandle se_gun;
		SoundHandle se_missile;
		SoundHandle se_hit;

		cockpits cocks;	//コックピット
		//セット
		void set_human(const std::vector<Vehcs>& vehcs, const std::vector<Ammos>& Ammo_) {
			std::fill(this->key.begin(), this->key.end(), false); //操作
			auto& veh = this->vehicle;
			//共通
			{
				veh.use_id = std::min<size_t>(veh.use_id, vehcs.size() - 1);
				veh.init(vehcs[veh.use_id], Ammo_);
			}
			//飛行機
			{
				//追加アニメーション
				{
					this->p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
					this->p_anime_geardown.second = 1.f;
					this->changegear.first = true;
					this->changegear.second = 2;
					//舵
					for (auto& r : this->p_animes_rudder) {
						r.first = MV1AttachAnim(veh.obj.get(), 2 + (int)(&r - &this->p_animes_rudder[0]));
						r.second = 0.f;
					}
				}
				//エフェクト
				for (auto& be : veh.use_veh.burner) {
					this->p_burner.emplace_back(be);
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
							veh_t.hits[m.first].res = veh_t.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m.first));
							if (veh_t.hits[m.first].res.HitFlag) {
								veh_t.hits[m.first].sort = { m.first, (c.repos - veh_t.hits[m.first].res.HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hits[m.first].sort = { m.first, (std::numeric_limits<float>::max)() };
							}
						}
						//空間装甲
						for (auto& m : veh_t.use_veh.space_mesh) {
							veh_t.hits[m].res = veh_t.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh_t.hits[m].res.HitFlag) {
								veh_t.hits[m].sort = { m, (c.repos - veh_t.hits[m].res.HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hits[m].sort = { m, (std::numeric_limits<float>::max)() };
							}
						}
						//装甲
						for (auto& m : veh_t.use_veh.armer_mesh) {
							veh_t.hits[m.first].res = veh_t.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
							if (veh_t.hits[m.first].res.HitFlag) {
								veh_t.hits[m.first].sort = { m.first, (c.repos - veh_t.hits[m.first].res.HitPosition).size() };
								is_hit = true;
							}
							else {
								veh_t.hits[m.first].sort = { m.first, (std::numeric_limits<float>::max)() };
							}
						}
						//当たってない
						if (!is_hit) {
							continue;
						}
						//
						t.se_hit.play(DX_PLAYTYPE_BACK, TRUE);
						//当たり判定を近い順にソート
						std::sort(veh_t.hits.begin(), veh_t.hits.end(), [](const hit_data& x, const hit_data& y) { return x.sort.second < y.sort.second; });
						//ダメージ面に届くまで判定
						for (auto& tt : veh_t.hits) {
							//装甲面に当たらなかったならスルー
							if (tt.sort.second == (std::numeric_limits<float>::max)()) {
								break;
							}
							//当たったら判定
							for (auto& a : veh_t.use_veh.armer_mesh) {
								if (tt.sort.first != a.first) {
									continue;
								}
								hitnear = tt.sort.first;
								//ダメージ面に当たった時に装甲値に勝てるかどうか
								{
									VECTOR_ref normal = tt.res.Normal;
									VECTOR_ref position = tt.res.HitPosition;

									VECTOR_ref vec_t = c.vec;
									//弾処理
									c.vec += normal * ((c.vec.dot(normal)) * -2.0f);
									c.vec = c.vec.Norm();
									c.pos = c.vec * (0.1f) + position;
									//貫通
									if (c.spec.pene_a > a.second * (1.0f / std::abs(vec_t.Norm().dot(normal)))) {
										auto tt = veh_t.HP;

										if (t.p_anime_geardown.second <= 0.5f) {
											//veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
											veh_t.HP = std::max<int16_t>(veh_t.HP - c.spec.damage_a, 0); //
										}
										//撃破時エフェクト
										if (veh_t.HP == 0 && tt!= veh_t.HP) {
											this->vehicle.KILL++;
											this->vehicle.KILL_ID = (int)(&t - &tgts[0]);
											veh_t.DEATH++;
											veh_t.DEATH_ID = (int)(this - &tgts[0]);
											t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
										}
										//弾処理
										c.flug = false;
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
									}
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(c.pos, normal);
									}
									else {
										this->effcs[ef_reco2].set(c.pos, normal);
									}
								}
							}
							//はじかれた
							if (hitnear.has_value()) {
								continue;
							}
							//空間装甲、モジュールに当たったのでモジュールに30ダメ、貫徹力を1/2に
							for (auto& a : veh_t.use_veh.space_mesh) {
								if (tt.sort.first == a) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									default:
										break;
									}
									//破壊時時エフェクト
									if (veh_t.HP_m[tt.sort.first] == 0) {
										t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
									}
								}
							}
							for (auto& a : veh_t.use_veh.module_mesh) {
								if (tt.sort.first == a.first) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									default:
										break;
									}
									//破壊時時エフェクト
									if (veh_t.HP_m[tt.sort.first] == 0) {
										t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
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
	//
};
