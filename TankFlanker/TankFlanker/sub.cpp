#include "sub.hpp"
#define NOMINMAX

MainClass::MainClass(void) {
	using namespace std::literals;
	//WIN32_FIND_DATA win32fdt;
	//HANDLE hFind;
	SetOutApplicationLogValidFlag(FALSE); /*log*/
	const auto mdata = FileRead_open("data/setting.txt", FALSE);
	{
		USE_GRAV = bool(getparam_u(mdata));
		ANTI = unsigned char(getparam_u(mdata));
		USE_YSync = bool(getparam_u(mdata));
		f_rate = (USE_YSync) ? 60.f : getparam_f(mdata);
		USE_windowmode = bool(getparam_u(mdata));
		drawdist = getparam_f(mdata);
		gnd_x = getparam_i(mdata);
		shade_x = getparam_i(mdata);
		USE_HOST = bool(getparam_u(mdata));
		USE_PIXEL = bool(getparam_u(mdata));
		se_vol = getparam_f(mdata) / 100.f;
	}
	FileRead_close(mdata);

	//SetWindowStyleMode(4);			    /**/
	//SetWindowUserCloseEnableFlag(FALSE);		    /*alt+F4�Ώ�*/
	SetMainWindowText("Tank Flanker");		    /*name*/
	SetAeroDisableFlag(TRUE);			    /**/
	SetUseDirect3DVersion(DX_DIRECT3D_11);		    /*directX ver*/
	SetEnableXAudioFlag(FALSE);			    /**/
	Set3DSoundOneMetre(1.0f);			    /*3Dsound*/
	SetGraphMode(dispx, dispy, 32);			    /*�𑜓x*/
	if (ANTI >= 2)					    /**/
		SetFullSceneAntiAliasingMode(ANTI, 3);	    /*�A���`�G�C���A�X*/
	SetUsePixelLighting(USE_PIXEL);			    /*�s�N�Z�����C�e�B���O*/
	SetWaitVSyncFlag(USE_YSync);			    /*��������*/
	ChangeWindowMode(USE_windowmode);		    /*���\��*/
	DxLib_Init();					    /*init*/
	Effekseer_Init(8000);				    /*Effekseer*/
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);  /*Effekseer*/
	Effekseer_SetGraphicsDeviceLostCallbackFunctions(); /*Effekseer*/
	SetAlwaysRunFlag(TRUE);				    /*background*/
	SetUseZBuffer3D(TRUE);				    /*zbufuse*/
	SetWriteZBuffer3D(TRUE);			    /*zbufwrite*/
	MV1SetLoadModelReMakeNormal(TRUE);		    /*�@��*/
	MV1SetLoadModelPhysicsWorldGravity(M_GR);	    /*�d��*/
							    //SetSysCommandOffFlag(TRUE)//�����|�[�Y�΍�()
}

MainClass::~MainClass(void) {
	Effkseer_End();
	DxLib_End();
}
