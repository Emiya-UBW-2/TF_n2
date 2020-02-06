#pragma once

#ifndef INCLUDED_define_h_
#define INCLUDED_define_h_

#include "DxLib.h"
#include "EffekseerForDXLib.h"
#include "Box2D/Box2D.h"
#include "useful.hpp"
#include <windows.h>
#include <thread>
#include <string_view>

class MainClass {
private:
	/*setting*/
	bool USE_GRAV;	       /*�l�̕������Z�̃I�t�A�I��*/
	unsigned char ANTI;    /*�A���`�G�C���A�X�{��*/
	bool USE_YSync;	       /*��������*/
	float f_rate;	       /*fps*/
	bool USE_windowmode;   /*�E�B���h�Eor�S���*/
	float drawdist;	       /*�؂̕`�拗��*/
	unsigned char gnd_x;   /*�n�ʂ̃N�I���e�B*/
	unsigned char shade_x; /*�e�̃N�I���e�B*/
	bool USE_HOST;	       /**/
	bool USE_PIXEL;	       /*�s�N�Z�����C�e�B���O�̗��p*/
	float se_vol;	       /**/
	float bgm_vol;	       /**/
public:
	MainClass(void);
	~MainClass(void);
};


#endif
