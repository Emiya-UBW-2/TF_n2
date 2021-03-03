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
	ef_smoke3 = 9, //飛行機の軌跡
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
	class Chara;
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
	//車輛
	class Vehcs {
		class foot_frame {
		public:
			frames frame;
			EffectS gndsmkeffcs;
			void init() {
				this->gndsmkeffcs.scale = 0.1f;
			}
			template<class Y, class D>
			void math(std::unique_ptr<Y, D>& mapparts, Chara* c, bool *hit_f) {
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
	public:
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
		float canlook_dist = 1000.f;
		std::vector<std::pair<size_t, float>> armer_mesh; //装甲ID
		std::vector<size_t> space_mesh;			  //装甲ID
		std::vector<std::pair<size_t, size_t>> module_mesh;		  //装甲ID
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
			this->canlook_dist = t.canlook_dist;
			this->armer_mesh = t.armer_mesh;
			this->space_mesh = t.space_mesh;
			this->module_mesh = t.module_mesh;
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
				if (Async) {
					SetUseASyncLoadFlag(TRUE);
				}
				t.ui_pic = GraphHandle::Load(std::string(name) + t.name + "/pic.png");
				if (Async) {
					SetUseASyncLoadFlag(FALSE);
				}
				//todo
				if (Async) {
					SetUseASyncLoadFlag(TRUE);
				}
				t.graph_HP_m.resize(7);
				for (int i = 0; i < t.graph_HP_m.size(); i++) {
					t.graph_HP_m[i] = GraphHandle::Load(std::string(name) + t.name + "/parts" + std::to_string(i + 1) + ".png");
				}
				t.graph_HP_m_all = GraphHandle::Load(std::string(name) + t.name + "/parts_all.png");
				if (Async) {
					SetUseASyncLoadFlag(FALSE);
				}
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
						t.burner.back().second = t.obj.GetFrameLocalMatrix(t.burner.back().first).pos();
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
					t.canlook_dist = getparams::_float(mdata);
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
	class vehicles;
	class Guns {
		std::vector<Ammos> Spec;		//
		std::array<ammos, 64> bullet;	//確保する弾
		size_t usebullet{};				//使用弾
		int16_t rounds_{ 0 };			//弾数
		gun_frame gun_info;				//
		float loadcnt{ 0 };				//装てんカウンター
	public:
		const auto& get_rounds_() {
			return rounds_;
		}
		const auto& get_gun_info() {
			return gun_info;
		}
		const auto& get_loadcnt() {
			return loadcnt;
		}
		const auto& getbullet_use() {
			return this->bullet[this->usebullet];
		}
		void clear() {
			this->Spec.clear();
			this->usebullet = 0;
			this->rounds_ = 0;
			this->loadcnt = 0.f;
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
			this->rounds_ = this->gun_info.rounds;
		}
		void draw() {
			for (auto& a : this->bullet) {
				if (a.flug) {
					if (!CheckCameraViewClip(a.pos.get())) {
						DXDraw::Capsule3D(a.pos, a.repos, (((a.spec.caliber_a - 0.00762f) * 0.1f + 0.00762f) * ((a.pos - GetCameraPosition()).size() / 24.f))*4.5f, a.color, GetColor(255, 255, 255));
					}
				}
			}
		}
		void math(const bool& key, Chara* c) {
			if (this->rounds_ > 0) {
				if (key && this->loadcnt == 0) {
					auto& u = this->bullet[this->usebullet];
					++this->usebullet %= this->bullet.size();
					//ココだけ変化
					u.spec = this->Spec[0];
					u.spec.speed_a += c->vehicle.speed;
					u.spec.speed_a *= float(75 + GetRand(50)) / 100.f;
					u.pos = c->vehicle.obj.frame(this->gun_info.frame2.first);
					u.vec = (c->vehicle.obj.frame(this->gun_info.frame3.first) - c->vehicle.obj.frame(this->gun_info.frame2.first)).Norm();
					//
					this->loadcnt = this->gun_info.load_time;
					this->rounds_ = std::max<uint16_t>(this->rounds_ - 1, 0);
					u.hit = false;
					u.flug = true;
					u.count = 0.f;
					u.yadd = 0.f;
					u.repos = u.pos;
					if (u.spec.type_a != 2) {
						c->effcs[ef_fire].set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec, u.spec.caliber_a / 0.1f);
						if (u.spec.caliber_a >= 0.017f) {
							//c->effcs_gun[c->gun_effcnt].first.set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec);
							//c->effcs_gun[c->gun_effcnt].second = &u;
							//c->effcs_gun[c->gun_effcnt].count = 0.f;
							//++c->gun_effcnt %= c->effcs_gun.size();
						}
						c->se.gun.play(DX_PLAYTYPE_BACK, TRUE);
					}
					else {
						c->effcs_missile[c->missile_effcnt].first.set(c->vehicle.obj.frame(this->gun_info.frame3.first), u.vec);
						c->effcs_missile[c->missile_effcnt].second = &u;
						c->effcs_missile[c->missile_effcnt].count = 0.f;
						++c->missile_effcnt %= c->effcs_missile.size();

						c->se.missile.play(DX_PLAYTYPE_BACK, TRUE);
					}
				}
				this->loadcnt = std::max(this->loadcnt - 1.f / GetFPS(), 0.f);
			}
		}
		//要改善
		template<class Y, class D>
		void math_reco(std::unique_ptr<Y, D>& mapparts, Chara* c, std::vector<Chara>* chara) {
			auto fps = GetFPS();
			for (auto& a : this->bullet) {
				float size = 2.f;
				for (int z = 0; z < int(size); z++) {
					if (a.flug) {
						a.repos = a.pos;
						a.pos += a.vec * (a.spec.speed_a / fps / size);
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
									a.spec.pene_a -= 1.0f / fps / size;
									a.spec.speed_a -= 5.f / fps / size;
									a.pos += VGet(0.f, a.yadd / size, 0.f);
								}
								break;
								case 1: //HE
								{
									a.spec.speed_a -= 5.f / fps / size;
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
										if (c == &t || c->type == t.type) {
											continue;
										}
										auto p = (t.vehicle.pos - a.pos).size();
										if (dist > p) {
											dist = p;
											id = &t - &(*chara)[0];
											pos = t.vehicle.pos + (t.vehicle.mat.zvec() * (-t.vehicle.speed / fps))*((a.pos - pos).size() / (a.spec.speed_a));
										}
									}
									if (id != chara->size()) {
										(*chara)[id].missile_cnt++;
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
												float limit = deg2rad(22.5f) / fps;
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
						if (a.count >= 4.f || a.spec.speed_a < 100.f || a.spec.pene_a <= 0.f) {
							a.flug = false;
						}
						//
					}
				}
				a.yadd += M_GR / powf(fps, 2.f);
				a.count += 1.f / fps;
			}
		}
	};								//
	class hit_data {							//
		class pair_hit {							//
			size_t first = 0;
			float second = 0.f;
		public:
			const auto& get_first() {
				return first;
			}
			const auto& get_second() {
				return second;
			}
			void set(const size_t& one, const float& two) {
				this->first = one;
				this->second = two;
			}
			void clear() {
				this->first = 0;
				this->second = 0.f;
			}
		};
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
		float HP_r = 0.f;
		bool dmgf = false;						//体力
		bool deathf = false;
		bool hitf = false;						//体力
		bool killf = false;						//体力
		bool kill_f = false;
		float kill_time = 0.f;
		uint16_t KILL_COUNT = 0;				//体力
		int KILL_ID = -1;						//体力
		uint16_t DEATH_COUNT = 0;				//体力
		int DEATH_ID = -1;						//体力
		VECTOR_ref pos;							//車体座標
		MATRIX_ref mat;							//車体回転行列
		float accel = 0.f;
		float speed = 0.f;						//
		VECTOR_ref pos_spawn;					//車体座標
		MATRIX_ref mat_spawn;					//車体回転行列
		float accel_spawn = 0.f;
		float speed_spawn = 0.f;				//
		VECTOR_ref add;							//車体加速度
		std::vector<Guns> Gun_;						//
		size_t sel_weapon = 0;
		float accel_add = 0.f;
		float WIP_timer_limit = 0.f;
		float WIP_timer = 0.f;
		bool over_heat = false;
		float speed_add = 0.f;		//
		float xradadd_left = 0.f, xradadd_right = 0.f; //
		float yradadd_left = 0.f, yradadd_right = 0.f;	    //
		float zradadd_left = 0.f, zradadd_right = 0.f; //
		std::vector<int16_t> HP_m;					//ライフ
		std::vector<GraphHandle> graph_HP_m;		//ライフ
		GraphHandle graph_HP_m_all;
		struct breaks {
			VECTOR_ref pos;							//車体座標
			MATRIX_ref mat;							//車体回転行列
			VECTOR_ref add;							//車体加速度
			float per = 1.f;
			float speed = 0.f;

			void set(vehicles& veh) {
				add = veh.add;
				speed = veh.speed;
				pos = veh.pos;
				mat = veh.mat;
				per = 1.f;
			}

			void set_break() {
				add.yadd(M_GR / powf(GetFPS(), 2.f) / 2.f);
				pos += add + (mat.zvec() * (-speed / GetFPS()));
				per = std::max(per - (1.f / 2.f) / GetFPS(), 0.f);
			}
		};
		std::vector<breaks> info_break;					//ライフ

		float wheel_Left = 0.f, wheel_Right = 0.f;			//転輪回転
		float wheel_Leftadd = 0.f, wheel_Rightadd = 0.f;		//転輪回転

		std::vector<hit_data> hits;

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
			this->sel_weapon = 0;

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
				this->col.SetupCollInfo(1, 1, 1, -1, j);
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

			this->graph_HP_m.clear();
			for (auto& p : vehcs.graph_HP_m) { this->graph_HP_m.emplace_back(p.Duplicate()); }
			graph_HP_m_all = vehcs.graph_HP_m_all.Duplicate();
			this->info_break.resize(this->col.mesh_num());
			//砲
			this->Gun_.resize(this->use_veh.gunframe.size());
			for (auto& cg : this->Gun_) {
				cg.init(this->use_veh.gunframe[&cg - &this->Gun_[0]], Ammo_);
			}
			respawn();
		}
		void Dispose() {
			this->obj.Dispose();
			this->obj_break.Dispose();
			this->col.Dispose();

			this->hit_check = false;
			this->HP = 0;
			this->dmgf = false;
			this->deathf = false;
			this->hitf = false;
			this->killf = false;
			this->KILL_COUNT = 0;						//体力
			this->KILL_ID = -1;						//体力
			this->DEATH_COUNT = 0;						//体力
			this->DEATH_ID = -1;						//体力

			for (auto& hz : this->hits) {
				hz.sort.clear();
			}
			this->hits.clear();
			for (auto& p : this->graph_HP_m) {
				p.Dispose();
			}
			this->graph_HP_m.clear();
			this->graph_HP_m_all.Dispose();
			this->HP_m.clear();
			this->wheel_Left = 0.f;
			this->wheel_Right = 0.f;
			this->wheel_Leftadd = 0.f;
			this->wheel_Rightadd = 0.f;
			std::for_each(this->Gun_.begin(), this->Gun_.end(), [](Guns& g) {g.clear(); });					//砲
			this->Gun_.clear();

			this->reset();
		}
		void respawn() {
			spawn(this->pos_spawn, this->mat_spawn, this->accel_spawn, this->speed_spawn*3.6f);
		}
		void spawn(const VECTOR_ref& pos_, const MATRIX_ref& mat_,const float& acc_, const float& spd_) {
			this->reset();

			this->pos_spawn = pos_;
			this->mat_spawn = mat_;
			this->accel_spawn = acc_;// 25.f;
			this->speed_spawn = spd_ / 3.6f;// this->use_veh.min_speed_limit;
			
			this->pos = this->pos_spawn;
			this->mat = this->mat_spawn;
			this->accel = this->accel_spawn;
			this->speed = this->speed_spawn;

			std::for_each(this->Gun_.begin(), this->Gun_.end(), [](Guns& g) {g.set(); });					//砲
			this->HP = this->use_veh.HP;																	//ヒットポイント
			std::for_each(this->HP_m.begin(), this->HP_m.end(), [&](int16_t& m) {m = this->use_veh.HP; });	//モジュール耐久
		}

		void draw() {
			if (!CheckCameraViewClip_Box((this->pos + VGet(-7.f, -7.f, -7.f)).get(), (this->pos + VGet(7.f, 7.f, 7.f)).get())) {
				for (auto& h : this->HP_m) {
					size_t i = &h - &this->HP_m[0];
					if (i >= 3) {
						if (h > 0) {
							this->obj.DrawMesh(int(this->use_veh.module_mesh[int(i - 3)].second));
						}
						else {
							if (this->info_break[i].per > 0.1f) {
								this->obj_break.SetOpacityRate(this->info_break[i].per);
								this->obj_break.SetMatrix(this->info_break[i].mat * MATRIX_ref::Mtrans(this->info_break[i].pos));
								this->obj_break.DrawMesh(int(this->use_veh.module_mesh[int(i - 3)].second));
							}
						}
					}
				}
				for (int i = 0; i < this->use_veh.module_mesh[0].second; i++) {
					this->obj.DrawMesh(i);
				}
			}
		}
	};
public:
	//カメラ
	struct CAMS {
		cam_info cam;
		int Rot = 0;//
	};
	//サウンド
	class sounds_3D {
	public:
		SoundHandle cockpit;
		SoundHandle engine;
		SoundHandle gun;
		SoundHandle missile;
		SoundHandle hit;

		void Load() {
			SetCreate3DSoundFlag(TRUE);
			this->cockpit = SoundHandle::Load("data/audio/fighter-cockpit1.wav");
			this->engine = SoundHandle::Load("data/audio/engine.wav");
			this->gun = SoundHandle::Load("data/audio/hit.wav");
			this->missile = SoundHandle::Load("data/audio/rolling_rocket.wav");
			this->hit = SoundHandle::Load("data/audio/fall.wav");
			SetCreate3DSoundFlag(FALSE);
		}

		void Duplicate(sounds_3D& handle) {
			this->cockpit = handle.cockpit.Duplicate();
			this->engine = handle.engine.Duplicate();
			this->gun = handle.gun.Duplicate();
			this->missile = handle.missile.Duplicate();
			this->hit = handle.hit.Duplicate();
			this->cockpit.Radius(600.f);
			this->engine.Radius(600.f);
			this->gun.Radius(300.f);
			this->missile.Radius(300.f);
			this->hit.Radius(900.f);
		}

		void setpos(const VECTOR_ref& pos) {
			this->cockpit.SetPosition(pos);
			this->engine.SetPosition(pos);
			this->hit.SetPosition(pos);
			this->gun.SetPosition(pos);
			this->missile.SetPosition(pos);
		}
		void stop() {
			this->engine.stop();
			this->cockpit.stop(); //gun
			this->gun.stop(); //gun
			this->missile.stop();
			this->hit.stop(); //gun
		}
	};
	//キーバインド
	class key_bind {
		struct keys {
			int mac = 0, px = 0, py = 0;
			char onhandle[256], offhandle[256];
		};
		struct keyhandle {
			keys key;
			GraphHandle onhandle, offhandle;
		};
		FontHandle font18;
		std::vector<keyhandle> keyg;
	public:
		class key_pair {
		public:
			int first;
			std::string second;
			switchs on_off;
			bool get_key(int id) {
				switch (id) {
				case 0:
					return CheckHitKey(this->first) != 0;
				case 1:
					on_off.get_in(CheckHitKey(this->first) != 0);
					return on_off.on();
				case 2:
					on_off.get_in(CheckHitKey(this->first) != 0);
					return on_off.push();
				default:
					return CheckHitKey(this->first) != 0;
				}
			}
		};
		std::vector < key_pair > key_use_ID;
		//
		void load_keyg() {
			font18 = FontHandle::Create(18, DX_FONTTYPE_EDGE);
			//
			key_pair tmp_k;
			tmp_k.first = KEY_INPUT_W;
			tmp_k.second = "下降";
			this->key_use_ID.emplace_back(tmp_k);//0
			tmp_k.first = KEY_INPUT_S;
			tmp_k.second = "上昇";
			this->key_use_ID.emplace_back(tmp_k);//1
			tmp_k.first = KEY_INPUT_D;
			tmp_k.second = "右ロール";
			this->key_use_ID.emplace_back(tmp_k);//2
			tmp_k.first = KEY_INPUT_A;
			tmp_k.second = "左ロール";
			this->key_use_ID.emplace_back(tmp_k);//3
			tmp_k.first = KEY_INPUT_Q;
			tmp_k.second = "左ヨー";
			this->key_use_ID.emplace_back(tmp_k);//4
			tmp_k.first = KEY_INPUT_E;
			tmp_k.second = "右ヨー";
			this->key_use_ID.emplace_back(tmp_k);//5
			tmp_k.first = KEY_INPUT_R;
			tmp_k.second = "スロットル開く";
			this->key_use_ID.emplace_back(tmp_k);//6
			tmp_k.first = KEY_INPUT_F;
			tmp_k.second = "スロットル絞る";
			this->key_use_ID.emplace_back(tmp_k);//7
			tmp_k.first = KEY_INPUT_G;
			tmp_k.second = "ランディングブレーキ";
			this->key_use_ID.emplace_back(tmp_k);//8
			tmp_k.first = KEY_INPUT_LSHIFT;
			tmp_k.second = "精密動作";
			this->key_use_ID.emplace_back(tmp_k);//9
			tmp_k.first = KEY_INPUT_O;
			tmp_k.second = "タイトル画面に戻る";
			this->key_use_ID.emplace_back(tmp_k);//10
			tmp_k.first = KEY_INPUT_ESCAPE;
			tmp_k.second = "強制終了";
			this->key_use_ID.emplace_back(tmp_k);//11
			tmp_k.first = KEY_INPUT_C;
			tmp_k.second = "ロックオンリセット";
			this->key_use_ID.emplace_back(tmp_k);//12
			tmp_k.first = KEY_INPUT_V;
			tmp_k.second = "オートスラスト";
			this->key_use_ID.emplace_back(tmp_k);//13
			tmp_k.first = KEY_INPUT_SPACE;
			tmp_k.second = "武装切替";
			this->key_use_ID.emplace_back(tmp_k);//14
			tmp_k.first = KEY_INPUT_LCONTROL;
			tmp_k.second = "視点切替";
			this->key_use_ID.emplace_back(tmp_k);//15
			{
				std::fstream file;
				/*
				std::vector<keys> key;
				file.open("data/1.dat", std::ios::binary | std::ios::out);
				for (auto& m : key)
					file.write((char*)&m, sizeof(m));
				file.close();
				//*/
				//*
				file.open("data/1.dat", std::ios::binary | std::ios::in);
				keys keytmp;
				while (true) {
					file.read((char*)&keytmp, sizeof(keytmp));
					if (file.eof()) {
						break;
					}
					for (auto& k : this->key_use_ID) {
						if (keytmp.mac == k.first) {
							this->keyg.resize(this->keyg.size() + 1);
							this->keyg.back().key = keytmp;
							this->keyg.back().onhandle = GraphHandle::Load(this->keyg.back().key.onhandle);
							this->keyg.back().offhandle = GraphHandle::Load(this->keyg.back().key.offhandle);
							break;
						}
					}
				}
				file.close();
				//*/
			}
		}
		//
		void draw() {
			int xp_s = 100, yp_s = 300, y_size = 25;
			for (auto& m : this->keyg) {
				for (auto& i : this->key_use_ID) {
					if (m.key.mac == i.first) {
						if (CheckHitKey(m.key.mac) != 0) {
							m.onhandle.DrawRotaGraph(xp_s + y_size / 2, yp_s + y_size / 2, float(y_size - 2) / 30.f, 0.f, false);
						}
						else {
							m.offhandle.DrawRotaGraph(xp_s + y_size / 2, yp_s + y_size / 2, float(y_size - 2) / 30.f, 0.f, false);
						}
						font18.DrawString(xp_s + y_size * 2, yp_s + y_size - 18, i.second, GetColor(255, 255, 255)); yp_s += y_size;
					}
				}
			}
		}
		//
		void set(Chara& chara) {
			//
			chara.key[0] = ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0);   //射撃
			chara.key[1] = ((GetMouseInput() & MOUSE_INPUT_MIDDLE) != 0); //マシンガン
			//操縦
			chara.key[2] = this->key_use_ID[0].get_key(0) && !this->key_use_ID[9].get_key(0);
			chara.key[3] = this->key_use_ID[1].get_key(0) && !this->key_use_ID[9].get_key(0);
			chara.key[4] = this->key_use_ID[2].get_key(0) && !this->key_use_ID[9].get_key(0);
			chara.key[5] = this->key_use_ID[3].get_key(0) && !this->key_use_ID[9].get_key(0);
			chara.key[6] = this->key_use_ID[4].get_key(0) && !this->key_use_ID[9].get_key(0);
			chara.key[7] = this->key_use_ID[5].get_key(0) && !this->key_use_ID[9].get_key(0);
			//スロットル
			{
				if (this->key_use_ID[13].get_key(2)) {
					chara.use_auto_thrust ^= 1;
				}
				if (chara.use_auto_thrust) {
					chara.auto_thrust(chara.speed_auto_thrust);
					if (this->key_use_ID[6].get_key(2)) {
						if ((chara.speed_auto_thrust + 50.f) / 3.6f < chara.vehicle.use_veh.max_speed_limit) {
							chara.speed_auto_thrust += 50.f;
						}
					}
					if (this->key_use_ID[7].get_key(2)) {
						if ((chara.speed_auto_thrust - 50.f) / 3.6f > chara.vehicle.use_veh.min_speed_limit) {
							chara.speed_auto_thrust -= 50.f;
						}
					}
				}
				else {
					chara.speed_auto_thrust = 400.f;
					chara.key[8] = this->key_use_ID[6].get_key(0);
					chara.key[9] = this->key_use_ID[7].get_key(0);
				}
			}
			//脚
			chara.key[10] = false;
			//ブレーキ
			chara.key[11] = this->key_use_ID[8].get_key(0);
			if (chara.chock) {
				if (chara.key[11]) {
					chara.chock = false;
				}
				chara.key[11] = true;
			}
			//精密操作
			chara.key[12] = this->key_use_ID[0].get_key(0) && this->key_use_ID[9].get_key(0);
			chara.key[13] = this->key_use_ID[1].get_key(0) && this->key_use_ID[9].get_key(0);
			chara.key[14] = this->key_use_ID[2].get_key(0) && this->key_use_ID[9].get_key(0);
			chara.key[15] = this->key_use_ID[3].get_key(0) && this->key_use_ID[9].get_key(0);
			chara.key[16] = this->key_use_ID[4].get_key(0) && this->key_use_ID[9].get_key(0);
			chara.key[17] = this->key_use_ID[5].get_key(0) && this->key_use_ID[9].get_key(0);
		}
		//
	};
	//player
	class Chara {
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
		//
		struct ef_missiles {
			EffectS first;
			ammos* second = nullptr;
			bool n_l;
			bool flug;
			float count = -1.f;
		};
		//
		typedef std::pair<int, float> p_animes;
		//
	public:
		//====================================================
		std::array<EffectS, ef_size> effcs; //effect
		std::array<ef_missiles, 8> effcs_missile; //effect
		size_t missile_effcnt = 0;
		size_t type;
		size_t aim_cnt = 0;//狙われている相手の数
		size_t missile_cnt = 0;//狙われている相手の数
		float death_timer = 0.f;
		bool death = false;
		//操作関連//==================================================
		float view_xrad = 0.f, view_yrad = 0.f; //砲塔操作用ベクトル
		std::array<bool, 18> key{ false };    //キー
		bool use_auto_thrust = true;
		float speed_auto_thrust = 400.f;
		switchs changegear; //ギアアップスイッチ
		bool chock = false;	//チョーク
		//飛行機//==================================================
		p_animes p_anime_geardown;		    //車輪アニメーション
		std::array<p_animes, 6> p_animes_rudder;//ラダーアニメーション
		std::vector<frames> p_burner;		    //バーナー
		//共通項//==================================================
		vehicles vehicle;
		VECTOR_ref winpos;
		VECTOR_ref winpos_if;

		sounds_3D se;
		//コックピット
		cockpits cocks;
		//セット
		void set_human(const std::vector<Vehcs>& vehcs, const std::vector<Ammos>& Ammo_) {
			//操作
			{
				std::fill(this->key.begin(), this->key.end(), false);
				//オートスラスト
				this->use_auto_thrust = true;
				this->speed_auto_thrust = 400.f;
				//脚スイッチ
				this->changegear.get_in(true);
				//チョーク
				this->chock = true;
			}
			{
				auto& veh = this->vehicle;
				veh.use_id = std::min<size_t>(veh.use_id, vehcs.size() - 1);
				veh.init(vehcs[veh.use_id], Ammo_);
				//追加アニメーション
				{
					//脚
					this->p_anime_geardown.first = MV1AttachAnim(veh.obj.get(), 1);
					this->p_anime_geardown.second = 1.f;
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
			{
				this->death = false;
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
								veh_t.hits[m.first].sort.set(m.first, (c.repos - veh_t.hits[m.first].res.HitPosition).size());
								is_hit = true;
							}
							else {
								veh_t.hits[m.first].sort.set(m.first, (std::numeric_limits<float>::max)());
							}
						}
						//空間装甲
						for (auto& m : veh_t.use_veh.space_mesh) {
							veh_t.hits[m].res = veh_t.col.CollCheck_Line(c.repos, (c.pos + (c.pos - c.repos) * (0.1f)), -1, int(m));
							if (veh_t.hits[m].res.HitFlag) {
								veh_t.hits[m].sort.set(m, (c.repos - veh_t.hits[m].res.HitPosition).size());
								is_hit = true;
							}
							else {
								veh_t.hits[m].sort.set(m, (std::numeric_limits<float>::max)());
							}
						}
						//装甲
						for (auto& m : veh_t.use_veh.armer_mesh) {
							veh_t.hits[m.first].res = veh_t.col.CollCheck_Line(c.repos, c.pos, -1, int(m.first));
							if (veh_t.hits[m.first].res.HitFlag) {
								veh_t.hits[m.first].sort.set(m.first, (c.repos - veh_t.hits[m.first].res.HitPosition).size());
								is_hit = true;
							}
							else {
								veh_t.hits[m.first].sort.set(m.first, (std::numeric_limits<float>::max)());
							}
						}
						//当たってない
						if (!is_hit) {
							continue;
						}
						//
						t.se.hit.play(DX_PLAYTYPE_BACK, TRUE);
						//当たり判定を近い順にソート
						std::sort(veh_t.hits.begin(), veh_t.hits.end(), [](hit_data& x, hit_data& y) { return x.sort.get_second() < y.sort.get_second(); });
						//ダメージ面に届くまで判定
						for (auto& tt : veh_t.hits) {
							auto sort_ = tt.sort.get_first();
							//装甲面に当たらなかったならスルー
							if (tt.sort.get_second() == (std::numeric_limits<float>::max)()) {
								break;
							}
							//当たったら判定
							for (auto& a : veh_t.use_veh.armer_mesh) {
								if (sort_ != a.first) {
									continue;
								}
								hitnear = sort_;
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
										auto ttn = veh_t.HP;

										if (t.p_anime_geardown.second <= 0.5f) {
											//veh_t.HP_m[tt.sort.first] = std::max<int16_t>(veh_t.HP_m[tt.sort.first] - c.spec.damage_a, 0); //
											veh_t.HP = std::max<int16_t>(veh_t.HP - c.spec.damage_a, 0); //
										}
										//撃破時エフェクト
										veh_t.dmgf = true;
										this->vehicle.hitf = true;
										if (veh_t.HP == 0 && ttn != veh_t.HP) {
											this->vehicle.KILL_COUNT++;
											this->vehicle.KILL_ID = (int)(&t - &tgts[0]);
											veh_t.deathf = true;

											this->vehicle.killf = true;
											this->vehicle.kill_f = true;
											this->vehicle.kill_time = 7.f;

											veh_t.DEATH_COUNT++;
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
								if (sort_ == a) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									default:
										break;
									}
									//破壊時時エフェクト
									if (veh_t.HP_m[sort_] == 0) {
										t.effcs[ef_bomb].set(veh_t.obj.frame(veh_t.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0));
									}
								}
							}
							for (auto& a : veh_t.use_veh.module_mesh) {
								if (sort_ == a.first) {
									if (c.spec.caliber_a >= 0.020f) {
										this->effcs[ef_reco].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									else {
										this->effcs[ef_reco2].set(VECTOR_ref(tt.res.HitPosition) + VECTOR_ref(tt.res.Normal) * (0.1f), tt.res.Normal);
									}
									switch (c.spec.type_a) {
									case 0: //AP
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.spec.pene_a /= 2.0f;
										break;
									case 1: //HE
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									case 2: //ミサイル
										veh_t.HP_m[sort_] = std::max<int16_t>(veh_t.HP_m[sort_] - c.spec.damage_a, 0); //
										c.flug = false;//爆発する
										break;
									default:
										break;
									}
									//破壊時時エフェクト
									if (veh_t.HP_m[sort_] == 0) {
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
		//操作反映
		float rand_(const float& p) {
			return float((-int(p*1000.f) + GetRand(int(p*1000.f) * 2))) / 1000.f;
		}
		float updn_(const float& p, const bool& u, const bool& d) {
			return u ? p : (d ? p / 3.f : 0.f);
		}
		template<class Y, class D>
		void update(std::unique_ptr<Y, D>& mapparts, std::unique_ptr<DXDraw, std::default_delete<DXDraw>>& Drawparts, std::vector<Chara>* chara, bool& start_c2) {
			auto fps = GetFPS();
			fps = (fps <= 60) ? 60.f : fps;
			auto& veh = this->vehicle;

			float rad_spec = 0.f;
			const float temp_t = 1000.f;

			float temp_0 = 0.f;
			float temp_1 = 0.f;
			float temp_2 = 0.f;
			//飛行機演算
			{
				//角速度指定
				rad_spec = deg2rad(
					veh.use_veh.body_rad_limit *
					((veh.speed < veh.use_veh.min_speed_limit) ? (std::clamp(veh.speed, 0.f, veh.use_veh.min_speed_limit) / veh.use_veh.min_speed_limit) : (veh.use_veh.mid_speed_limit / veh.speed))
				);
				//ピッチ
				{
					if (veh.HP_m[this->vehicle.use_veh.module_mesh[2].first] > 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[3].first] > 0) {
						temp_0 = rad_spec / 3.f;
						temp_1 = 0;
						temp_2 = 0;
					}
					else if (!(veh.HP_m[this->vehicle.use_veh.module_mesh[2].first] == 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[3].first] == 0)) {
						temp_0 = rad_spec / 6.f;
						temp_1 = rand_(rad_spec / 12.f);
						temp_2 = rand_(rad_spec / 12.f);
					}
					else {
						temp_0 = rad_spec / 12.f;
						temp_1 = rand_(rad_spec / 3.f);
						temp_2 = rand_(rad_spec / 3.f);
					}
					easing_set(&veh.xradadd_right, temp_1 + updn_(-temp_0, this->key[2], this->key[12]), 0.95f);
					easing_set(&veh.xradadd_left, temp_2 + updn_(temp_0, this->key[3], this->key[13]), 0.95f);
				}
				//ロール
				{
					if (veh.HP_m[this->vehicle.use_veh.module_mesh[0].first] > 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[1].first] > 0) {
						temp_0 = rad_spec / 1.f;
						temp_1 = 0;
						temp_2 = 0;
					}
					else if (!(veh.HP_m[this->vehicle.use_veh.module_mesh[0].first] == 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[1].first] == 0)) {
						temp_0 = rad_spec / 2.f;
						temp_1 = rand_(rad_spec / 12.f);
						temp_2 = rand_(rad_spec / 12.f);
					}
					else {
						temp_0 = rad_spec / 4.f;
						temp_1 = rand_(rad_spec / 6.f);
						temp_2 = rand_(rad_spec / 6.f);
					}
					easing_set(&veh.zradadd_right, temp_1 + updn_(-temp_0, this->key[5], this->key[15]), 0.95f);
					easing_set(&veh.zradadd_left, temp_2 + updn_(temp_0, this->key[4], this->key[14]), 0.95f);
				}
				//ヨー
				{
					if (veh.HP_m[this->vehicle.use_veh.module_mesh[4].first] > 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[5].first] > 0) {
						temp_0 = rad_spec / 24.f;
						temp_1 = 0;
						temp_2 = 0;
					}
					else if (!(veh.HP_m[this->vehicle.use_veh.module_mesh[4].first] == 0 && veh.HP_m[this->vehicle.use_veh.module_mesh[5].first] == 0)) {
						temp_0 = rad_spec / 48.f;
						temp_1 = rand_(rad_spec / 24.f);
						temp_2 = rand_(rad_spec / 24.f);
					}
					else {
						temp_0 = rad_spec / 96.f;
						temp_1 = rand_(rad_spec / 9.f);
						temp_2 = rand_(rad_spec / 9.f);
					}
					easing_set(&veh.yradadd_right, temp_1 + updn_(-temp_0, this->key[6], this->key[16]), 0.95f);
					easing_set(&veh.yradadd_left, temp_2 + updn_(temp_0, this->key[7], this->key[17]), 0.95f);
				}
				//スロットル
				{
					easing_set(&veh.accel_add, (veh.over_heat & (veh.accel >= 80.f)) ? -200.f : (this->key[8] ? 25.0f : (this->key[9] ? -25.0f : 0.f)), 0.95f);
					veh.accel = std::clamp(veh.accel + veh.accel_add / fps, 0.f, 110.f);
					if (veh.accel >= 100.f) {
						//WIP
						veh.WIP_timer += 1.0f / fps;
						veh.WIP_timer_limit = 15.f*std::clamp(2.f - veh.speed / veh.use_veh.max_speed_limit, 0.f, 1.f);
						if (veh.WIP_timer >= veh.WIP_timer_limit) {
							veh.over_heat = true;//オーバーヒート
						}
						//
						easing_set(&veh.speed_add, (0.6f / 3.6f), 0.95f);//0.1km/h
						//
					}
					else {
						//WIP
						veh.WIP_timer = std::max(veh.WIP_timer - 1.0f / fps, 0.f);
						if (veh.over_heat && veh.WIP_timer <= 0.f) {
							veh.over_heat = false;
						}
						//
						auto ac = veh.accel / 10.f;
						if (veh.accel <= 50.f) {
							if (veh.speed <= ((veh.use_veh.mid_speed_limit*ac + veh.use_veh.min_speed_limit*0.5f *(5.f - ac)) / 5.f)) {
								easing_set(&veh.speed_add, (((0.05f*ac + 0.2f *(5.f - ac)) / 5.f) / 3.6f), 0.95f);//0.1km/h
							}
							else {
								easing_set(&veh.speed_add, (-((0.5f*ac + 0.85f *(5.f - ac)) / 5.f) / 3.6f), 0.975f);//-0.05km/h
							}
						}
						else if (veh.speed >= ((veh.use_veh.mid_speed_limit*4.f + veh.use_veh.min_speed_limit*0.5f) / 5.f)) {
							if (veh.speed <= ((veh.use_veh.max_speed_limit*(ac - 5.f) + veh.use_veh.mid_speed_limit *(5.f - (ac - 5.f))) / 5.f)) {
								easing_set(&veh.speed_add, (((0.2f*(ac - 5.f) + 0.15f *(5.f - (ac - 5.f))) / 5.f) / 3.6f), 0.95f);//0.1km/h
							}
							else {
								easing_set(&veh.speed_add, (-((0.75f*(ac - 5.f) + 1.35f *(5.f - (ac - 5.f))) / 5.f) / 3.6f), 0.975f);//-0.05km/h
							}
						}
						else {//離昇出力
							easing_set(&veh.speed_add, (0.5f / 3.6f), 0.95f);//0.1km/h
						}
					}
				}
				//スピード
				{
					veh.speed += veh.speed_add * 60.f / fps;
					{
						auto tmp = veh.mat.zvec();
						auto tmp2 = std::sin(atan2f(tmp.y(), std::hypotf(tmp.x(), tmp.z())));
						veh.speed += (((std::abs(tmp2) > std::sin(deg2rad(1.0f))) ? tmp2 * 0.5f : 0.f) / 3.6f) * 60.f / fps; //落下
					}
				}
				//座標系反映
				{
					auto t_mat = veh.mat;
					veh.mat *= MATRIX_ref::RotAxis(t_mat.xvec(), (veh.xradadd_right + veh.xradadd_left) / fps);
					veh.mat *= MATRIX_ref::RotAxis(t_mat.zvec(), (veh.zradadd_right + veh.zradadd_left) / fps);
					veh.mat *= MATRIX_ref::RotAxis(t_mat.yvec(), (veh.yradadd_left + veh.yradadd_right) / fps);
				}
				//脚演算
				{
					//this->changegear.get_in(this->key[10]);
					this->changegear.first = (veh.speed <= veh.use_veh.min_speed_limit*1.5f) && (veh.pos.y() <= 100.f);
					easing_set(&this->p_anime_geardown.second, float(this->changegear.first), 0.95f);
				}
				//舵演算
				for (auto& r : this->p_animes_rudder) {
					auto i = (size_t)(&r - &this->p_animes_rudder[0]);
					easing_set(&r.second, float(this->key[i + 2] + this->key[i + 12])*0.5f, 0.95f);
				}
				//車輪その他
				{
					//
					if (veh.speed >= veh.use_veh.min_speed_limit) {
						easing_set(&veh.add, VGet(0.f, 0.f, 0.f), 0.9f);
					}
					else {
						veh.add.yadd(M_GR / powf(fps, 2.f));
					}
					//
					if (this->p_anime_geardown.second >= 0.5f) {
						bool hit_f = false;
						for (auto& t : veh.use_veh.wheelframe) {
							t.math(mapparts, this, &hit_f);
						}

						if (hit_f) {
							easing_set(&veh.wheel_Leftadd, -veh.speed / 20.f, 0.95f);
							easing_set(&veh.wheel_Rightadd, -veh.speed / 20.f, 0.95f);
						}
						else {
							easing_set(&veh.wheel_Leftadd, 0.f, 0.9f);
							easing_set(&veh.wheel_Rightadd, 0.f, 0.9f);
						}
						veh.wheel_Left += veh.wheel_Leftadd;
						veh.wheel_Right += veh.wheel_Rightadd;
						for (auto& f : veh.use_veh.wheelframe_nospring) {
							veh.obj.SetFrameLocalMatrix(f.frame.first, MATRIX_ref::RotAxis(MATRIX_ref::Vtrans(VGet(0.f, 0.f, 0.f), MV1GetFrameLocalMatrix(veh.obj.get(), f.frame.first + 1)), (f.frame.second.x() >= 0) ? veh.wheel_Left : veh.wheel_Right) *MATRIX_ref::Mtrans(f.frame.second));
						}
					}
					else {
						for (auto& t : veh.use_veh.wheelframe) {
							easing_set(&t.gndsmkeffcs.scale, 0.01f, 0.9f);
						}
					}
					veh.pos += veh.add + (veh.mat.zvec() * (-veh.speed / fps));
				}
				//死亡関連
				if (this->death) {
					this->death_timer -= 1.f / fps;
					if (this->death_timer <= 0.f) {
						this->death = false;
						this->death_timer = 0.f;
						veh.respawn();
						this->p_anime_geardown.second = 1.f;
						this->changegear.first = true;
					}
				}
				//壁の当たり判定
				{
					bool hitb = false;
					{
						VECTOR_ref p_0 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
						VECTOR_ref p_1 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.maxpos.z()), veh.mat);
						VECTOR_ref p_2 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.maxpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
						VECTOR_ref p_3 = veh.pos + MATRIX_ref::Vtrans(VGet(veh.use_veh.minpos.x(), 0.f, veh.use_veh.minpos.z()), veh.mat);
						if (p_0.y() <= 0.f || p_1.y() <= 0.f || p_2.y() <= 0.f || p_3.y() <= 0.f) {
							hitb = true;
						}
						if (!hitb) {
							for (int i = 0; i < mapparts->map_col_get().mesh_num(); i++) {
								if (mapparts->map_col_line(p_0, p_1, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_1, p_2, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_2, p_3, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
								if (mapparts->map_col_line(p_3, p_0, i).HitFlag == TRUE) {
									hitb = true;
									break;
								}
							}
						}
						if (veh.HP == 0) {
							hitb = true;
						}
					}
					if (hitb && !this->death) {
						veh.HP = 0;
						this->death_timer = 3.f;
						this->death = true;
					}
				}
				//
			}
			//射撃
			{
				bool ms_key = true;
				for (auto& cg : veh.Gun_) {
					size_t i = (&cg - &veh.Gun_[0]);
					ms_key = this->key[(i == 0) ? 0 : 1];
					if (ms_key && i != (veh.sel_weapon + 1) && i != 0) {
						ms_key = false;
					}
					cg.math(ms_key, this);
				}
			}
			//弾関連
			{
				//弾判定
				for (auto& cg : veh.Gun_) {
					cg.math_reco(mapparts, this, chara);
				}
				//ミサイル
				for (auto& a : this->effcs_missile) {
					if (a.second != nullptr) {
						a.n_l = (a.second != nullptr);
						a.flug = a.second->flug;

						if (a.flug) {
							a.first.pos = a.second->pos;
							a.first.handle.SetPos(a.first.pos);
						}
						if (a.count >= 0.f) {
							a.count += 1.f / fps;
							if (a.count >= 4.5f) {
								a.first.handle.Stop();
								a.count = -1.f;
							}
						}
					}
				}
				//
			}
			//飛行機演算共通
			{
				//舵
				for (auto& r : this->p_animes_rudder) {
					MV1SetAttachAnimBlendRate(veh.obj.get(), r.first, r.second);
				}
				//脚
				MV1SetAttachAnimBlendRate(veh.obj.get(), this->p_anime_geardown.first, this->p_anime_geardown.second);
				//バーナー
				for (auto& be : this->p_burner) {
					veh.obj.SetFrameLocalMatrix(be.first, MATRIX_ref::Scale(VGet(1.f, 1.f, std::clamp(veh.speed / veh.use_veh.mid_speed_limit, 0.1f, 1.f))) * MATRIX_ref::Mtrans(be.second));
				}
			}
			//effect
			for (auto& t : this->effcs) {
				const size_t index = &t - &this->effcs[0];
				if (index != ef_smoke1 && index != ef_smoke2 && index != ef_smoke3) {
					t.put(Drawparts->get_effHandle(int(index)));
				}
			}
			//脚フレーム
			{
				for (auto& t : veh.use_veh.wheelframe) {
					t.gndsmkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), t.gndsmkeffcs.scale);
				}
				if (start_c2) {
					for (auto& t : veh.use_veh.wheelframe) {
						t.gndsmkeffcs.set_loop(Drawparts->get_effHandle(ef_gndsmoke));
					}
				}
			}
			//撃破エフェクト
			{
				for (auto& t : veh.use_veh.wingframe) {
					t.smkeffcs.put_loop(veh.obj.frame(int(t.frame.first + 1)), VGet(0, 1, 0), 10.f);
				}
				if (this->death) {
					if (this->death_timer == 3.f) {
						for (auto& t : veh.use_veh.wingframe) {
							t.smkeffcs.set_loop(Drawparts->get_effHandle(ef_smoke3));
						}
					}
				}
				else {
					for (auto& t : veh.use_veh.wingframe) {
						t.smkeffcs.handle.Stop();
					}
				}
			}
			//ミサイル
			for (auto& t : this->effcs_missile) {
				t.first.put(Drawparts->get_effHandle(ef_smoke1));
			}
			//モデルに反映
			{
				veh.obj.SetMatrix(veh.mat * MATRIX_ref::Mtrans(veh.pos));
				//破損表現
				for (auto& h : veh.HP_m) {
					auto& br = veh.info_break[&h - &veh.HP_m[0]];
					if (h > 0) {
						br.set(veh);
					}
					else {
						br.set_break();
					}
				}
			}
			//
		}
		//alive
		void set_alive(float& se_vol) {
			auto& veh = this->vehicle;
			//当たり判定クリア
			if (veh.hit_check) {
				veh.col.SetMatrix(MATRIX_ref::Mtrans(VGet(0.f, -100.f, 0.f)));
				for (int i = 0; i < veh.col.mesh_num(); i++) {
					veh.col.RefreshCollInfo(-1, i);
				}
				veh.hit_check = false;
			}
			this->aim_cnt = 0;
			this->missile_cnt = 0;
			easing_set(&veh.HP_r, float(veh.HP), 0.95f);

			this->se.engine.vol(int(float(128 + int(127.f * this->vehicle.accel / 100.f))*se_vol));

			if (veh.kill_f) {
				veh.kill_time -= 1.f / GetFPS();
				if (veh.kill_time <= 0.f) {
					veh.kill_time = 0.f;
					veh.kill_f = false;
				}
			}
			if (veh.HP == 0) {
				if (GetRand(100) < 12) {
					auto tmp = GetRand(int(veh.HP_m.size() - 1 - 1));
					if (veh.HP_m[tmp] != 0) {
						this->effcs[ef_bomb].set(veh.obj.frame(veh.use_veh.gunframe[0].frame1.first), VGet(0, 0, 0), 0.5f);
					}
					veh.HP_m[tmp] = 0;
				}
			}
			/*
			if (veh.Gun_.size() > veh.sel_weapon + 1) {
				if (veh.Gun_[veh.sel_weapon + 1].get_rounds_() == 0) {
					for (auto&g : veh.Gun_) {
						auto i = &g - &veh.Gun_[0];
						if (g.get_rounds_() != 0 && i != 0) {
							veh.sel_weapon = i - 1;
							break;
						}
					}
				}
			}
			//*/
		}
		//オートスラスト
		void auto_thrust(const float& spd) {
			auto& veh = this->vehicle;

			auto min = veh.use_veh.min_speed_limit*0.5f;
			auto mid = veh.use_veh.mid_speed_limit;
			auto max = veh.use_veh.max_speed_limit;
			auto ac = veh.accel / 10.f;
			auto aim_s = spd / 3.6f;

			aim_s = aim_s - (veh.speed - aim_s)*10.f;


			if (ac <= 5.f) {
				if (aim_s <= ((mid*ac + min * (5.f - ac)) / 5.f)) {
					this->key[8] = false;
					this->key[9] = true;
				}
				else {
					this->key[8] = true;
					this->key[9] = false;
				}
			}
			else if (aim_s >= ((mid*4.f + min) / 5.f)) {
				if (aim_s <= ((max*(ac - 5.f) + mid * ac) / 5.f)) {
					this->key[8] = false;
					this->key[9] = true;
				}
				else {
					this->key[8] = true;
					this->key[9] = false;
				}
			}
			else {//離昇出力
				this->key[8] = false;
				this->key[9] = true;
			}
		}
		//cpu
		void cpu_doing(std::vector<Chara>* chara) {
			auto& veh = this->vehicle;

			std::fill(this->key.begin(), this->key.end(), false); //操作

			bool ret = false;
			bool up = false;
			size_t tmp_id = chara->size();
			VECTOR_ref tgt_pos;

			float dist = (std::numeric_limits<float>::max)();
			for (auto& t : *chara) {
				//弾関連
				if (this == &t ||
					this->type == t.type ||
					(this->vehicle.pos - t.vehicle.pos).size() >= this->vehicle.use_veh.canlook_dist ||
					t.aim_cnt > 2
					) {
					continue;
				}
				auto p = (t.vehicle.pos - veh.pos).size();
				if (dist > p) {
					dist = p;
					tmp_id = &t - &(*chara)[0];
					tgt_pos = t.vehicle.pos;
				}
			}

			if (tmp_id == chara->size()) {
				tgt_pos = veh.pos - veh.mat.zvec();
			}
			else {
				(*chara)[tmp_id].aim_cnt++;
			}
			{
				auto tmpv = veh.pos;
				tmpv.y(0);
				if (tmpv.size() >= 5000) {
					tgt_pos = VGet(0, 0, 0);
					ret = true;
				}
				if (veh.pos.y() <= 400) {
					tmpv.y(500);
					tgt_pos = tmpv;
					up = true;
				}
			}

			VECTOR_ref tgt_zvec = (tgt_pos - veh.pos).Norm();
			VECTOR_ref my_xvec = veh.mat.xvec();
			VECTOR_ref my_yvec = veh.mat.yvec();
			VECTOR_ref my_zvec = veh.mat.zvec();
			//ピッチ
			{
				if (tmp_id == chara->size() && !ret && !up) {
					tgt_zvec.y(0.f);
					tgt_zvec = tgt_zvec.Norm();
				}
				auto tgt_yvec = tgt_zvec.cross(my_xvec);
				auto cross_yvec = tgt_zvec.cross(tgt_yvec);
				auto cross_vec = tgt_zvec.cross(my_zvec);
				auto dot = cross_vec.dot(cross_yvec);
				if (dot >= 0.1f) {
					this->key[2] = GetRand(10) <= 5;
					this->key[12] = GetRand(10) <= 5;
				}
				else if (dot <= -0.1f) {
					this->key[3] = GetRand(10) <= 5;
					this->key[13] = GetRand(10) <= 5;
				}
				else {
					if (dot >= 0.f) {
						this->key[12] = true;
					}
					else {
						this->key[13] = true;
					}
					if (tmp_id != chara->size() && !ret && !up) {
						if ((this->vehicle.mat.zvec()).dot(tgt_pos - this->vehicle.pos) < 0) {
							if ((tgt_pos - this->vehicle.pos).size() <= 300) {
								this->key[0] = GetRand(100) <= 20;   //射撃
							}
							if ((tgt_pos - this->vehicle.pos).size() <= 1500) {
								this->key[1] = GetRand(200) <= 1; //マシンガン
							}
						}
					}
				}
			}
			//ロール
			{
				if (tmp_id == chara->size() && !ret && !up) {
					tgt_zvec = VGet(0, 1, 0);
				}
				auto tgt_xvec = tgt_zvec.cross(my_zvec).Norm();
				auto tgt_yvec = tgt_xvec.cross(my_zvec);
				auto cross_vec = tgt_xvec.cross(my_xvec);
				auto dot = cross_vec.dot(tgt_xvec.cross(tgt_yvec));
				if (dot >= 0.1f) {
					this->key[4] = GetRand(10) <= 5;
					this->key[14] = GetRand(10) <= 5;
				}
				else if (dot <= -0.1f) {
					this->key[5] = GetRand(10) <= 5;
					this->key[15] = GetRand(10) <= 5;
				}
				else {
					if (dot >= 0.f) {
						this->key[14] = true;
					}
					else {
						this->key[15] = true;
					}
				}

			}
			//オートスラスト
			auto_thrust(450.f);
			//
		}
		//
	};
	//
};
