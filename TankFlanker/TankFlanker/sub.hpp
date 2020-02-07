#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_

#define NOMINMAX
#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Box2D/Box2D.h"
#include "useful.hpp"
#include "DXLib_vec.hpp"
#include "DXLib_mat.hpp"
#include "MV1ModelHandle.hpp"
#include "EffekseerEffectHandle.hpp"
#include "SoundHandle.hpp"
#include "GraphHandle.hpp"
#include "FontHandle.hpp"
#include <windows.h>
#include <fstream>
#include <string_view>

#include <array>
#include <vector>

struct switches {
	bool flug{ false };
	uint8_t cnt{ 0 };
};

class MainClass {
private:
	/*setting*/
	bool USE_GRAV;	       /*�l�̕������Z�̃I�t�A�I��*/
	unsigned char ANTI;    /*�A���`�G�C���A�X�{��*/
	bool USE_YSync;	       /*��������*/
	float frate;	       /*fps*/
	bool USE_windowmode;   /*�E�B���h�Eor�S���*/
	float drawdist;	       /*�؂̕`�拗��*/
	unsigned char gnd_x;   /*�n�ʂ̃N�I���e�B*/
	unsigned char shade_x; /*�e�̃N�I���e�B*/
	bool USE_HOST;	       /**/
	bool USE_PIXEL;	       /*�s�N�Z�����C�e�B���O�̗��p*/
	float se_vol;	       /**/
	float bgm_vol;	       /**/
	/**/		       /**/


public:
	/*setting*/
	inline const auto get_GRAV(void) { return USE_GRAV; }
	inline const auto get_gnd(void) { return gnd_x; }
	inline const auto get_shade(void) { return shade_x; }
	inline const auto get_drawdist(void) { return drawdist; }
	inline const auto get_frate(void) { return frate; }
	inline const auto get_host(void) { return USE_HOST; }
	inline const auto get_se_vol(void) { return se_vol; }
	inline const auto get_bgm_vol(void) { return bgm_vol; }
	void write_setting(void);
	MainClass(void);
	~MainClass(void);
	/*draw*/
	void set_light(const VECTOR_ref vec);
	void set_cam(const float neard, const float fard, const VECTOR_ref cam, const VECTOR_ref view, const VECTOR_ref up, const float fov); //�J�������w��
	void Screen_Flip(void);
	void Screen_Flip(LONGLONG waits);


};
class VEHICLE : public MainClass
{
private:
	struct vehicle {
		std::string name;		  /*���O*/
		int countryc;			  /*��*/
		MV1ModelHandle model;		  /*���f��*/
		MV1ModelHandle colmodel;	  /*�R���W����*/
		MV1ModelHandle inmodel;		  /*����*/
		std::array<float, 4> speed_flont; /*�O�i*/
		std::array<float, 4> speed_back;  /*���*/
		float vehicle_RD = 0.0f;	  /*���񑬓x*/
		std::array<float, 4> armer;       /*���b*/
		bool gun_lim_LR = 0;		  /*�C���������̗L��*/
		std::array<float, 4> gun_lim_;    /*�C�����񐧌�*/
		float gun_RD = 0.0f;		  /*�C�����񑬓x*/
		std::array<float, 3> gun_speed;   /*�e��*/
		std::array<float, 3> pene;	/*�ђ�*/
		std::array<int, 3> ammotype;      /*�e��*/
		std::vector<VECTOR_ref> loc;      /*�t���[���̌����W*/
		VECTOR_ref min;			  /*box2D�p�t���[��*/
		VECTOR_ref max;			  /*box2D�p�t���[��*/
		int turretframe;		  /*�C���t���[��*/
		struct guninfo {
			int gunframe;   /*�e�t���[��*/
			int reloadtime; /*�����[�h�^�C��*/
			float ammosize; /*�C���a*/
			int accuracy;   /*�C���x*/
		};
		std::array<guninfo, 2> gun_; /*�e�t���[��*/
		std::vector<int> youdoframe;    /*�U����*/
		std::vector<int> wheelframe;    /*�]��*/
		std::array<int, 2> kidoframe;   /*�N����*/
		std::array<int, 2> smokeframe;  /*�r��*/
		std::vector<int> upsizeframe;   /*���я�*/
		int engineframe;		/*�G���W��*/
	};
	std::vector<vehicle> vecs; /*���p���*/
	std::array<SoundHandle, 13> se_; /*���ʉ�*/
	std::array<SoundHandle, 6> bgm_; /*���ʉ�*/
public:
	VEHICLE();
	~VEHICLE();
};

#endif
