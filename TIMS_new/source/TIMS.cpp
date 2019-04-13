/* >>EXPORT
���t�@�C����
TIMS.dll: TIMS�\���탂�W���[��

���t�@�C���̐���
���̃t�@�C����TIMS�\���탂�W���[���B
DetailManager.dll�Ƃ̕��p��p�B

�h�A�p�d�킪�ʓr�K�v�B
�r���h���ɂ͕K��Panel��d�Ԏd�l����Ԏg�p���I�����邱�ƁB
*/
#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "atsplugin.h"
#include "TIMS.h"

CTims g_tims; // TIMS���u
CSpp g_spp; // ��ԉw�ʉߖh�~���u
CDate g_date; // ���t

#ifdef __cplusplus
extern "C" {
#endif

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_tims.BrakeNotch = &g_brakeNotch;
		g_tims.PowerNotch = &g_powerNotch;
		g_tims.Reverser = &g_reverser;
		g_tims.Time = &g_time;
		g_tims.TrainSpeed = &g_speed;
		g_tims.DeltaT = &g_deltaT;
		g_spp.TrainSpeed = &g_speed;
		g_spp.DeltaT = &g_deltaT;
		g_spp.BrakeNotch = &g_brakeNotch;
		/*
		g_sub.BrakeNotch = &g_brakeNotch;
		g_sub.PowerNotch = &g_powerNotch;
		g_sub.Reverser = &g_reverser;
		g_sub.TrainSpeed = &g_speed;
		g_accel.Time = &g_time;
		g_accel.TrainSpeed = &g_speed;
		g_accel.DeltaT = &g_deltaT;
		*/
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}

#ifdef __cplusplus
};
#endif

ATS_API void WINAPI Load(void)
{
	g_tims.load();
}

ATS_API void WINAPI Dispose(void)
{
}

ATS_API int WINAPI GetPluginVersion(void)
{
	return ATS_VERSION;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC vehicleSpec)
{
	g_svcBrake = vehicleSpec.BrakeNotches;
	g_emgBrake = g_svcBrake + 1;

	g_tims.EmergencyNotch = g_emgBrake;
	g_spp.ServiceNotch = vehicleSpec.AtsNotch;
	// g_sub.EmergencyNotch = g_emgBrake;
}

ATS_API void WINAPI Initialize(int brake)
{
	g_tims.event_lbInit = 1; // ����N�����͍s�x��

	g_speed = 0;
	g_tims.initialize();
	g_spp.InitSpp();
	g_date.initialize();
	// g_sub.initialize();
	// g_accel.initialize();
}

ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE vehicleState, int *panel, int *sound)
{
	g_deltaT = vehicleState.Time - g_time;
	g_time = vehicleState.Time;
	g_speed = vehicleState.Speed;

	g_tims.Location = vehicleState.Location; // ��Ԉʒu[m]
	g_tims.BcPressure = vehicleState.BcPressure; // �u���[�L�V�����_����[kPa]
	g_tims.MrPressure = vehicleState.MrPressure; // ����C�_������[kPa]
	g_tims.Current = vehicleState.Current; // �d��[A]

	g_tims.execute(); // TIMS���u
	g_spp.RunSpp(); // ��ԉw�ʉߖh�~���u

	g_date.update(); // ����
	// g_sub.execute(); // ���̑�

	// �n���h���o��
	if(g_tims.BcPressCut == 1)
	{
		g_output.Brake = 1; // EB�ɉ����Ƀu���[�L����������
	}
	else
	{
		g_output.Brake = g_brakeNotch;
	}

	g_output.Reverser = g_reverser;

	if(g_time > g_tims.AccelCutting && g_tims.VCB_ON == 1) // �͍s�x����VCB
	{
		g_output.Power = g_powerNotch;
	}
	else
	{
		g_output.Power = 0;
	}

	g_output.ConstantSpeed = ATS_CONSTANTSPEED_CONTINUE;

	// �p�l���o��
	panel[21] = g_tims.AmmeterC; // �d���v[A]
	panel[22] = g_tims.Ammeter1; // �d���v[1000��]
	panel[23] = g_tims.Ammeter2; // �d���v[100��]
	panel[24] = g_tims.Ammeter3; // �d���v[10��]
	panel[25] = g_tims.Ammeter4; // �d���v[1��]
	panel[26] = g_tims.Ammeter0; // �d���v[A]

	// panel[29] = g_sub.Key; // �}�X�R���L�[
	panel[30] = g_tims.BrakeDelay; // �u���[�L�i
	panel[31] = g_tims.BrakeDelay == g_emgBrake ? 1 : 0; // ���u���[�L
	panel[37] = (g_time / 3600000) % 24; // �f�W�^����
	panel[38] = g_time / 60000 % 60; // �f�W�^����
	panel[39] = g_time / 1000 % 60; // �f�W�^���b
	panel[41] = g_tims.UnitState[0]; // ���j�b�g�\����1
	panel[42] = g_tims.UnitState[1]; // ���j�b�g�\����2
	panel[43] = g_tims.UnitState[2]; // ���j�b�g�\����3
	panel[44] = g_tims.UnitState[3]; // ���j�b�g�\����4

	panel[45] = g_tims.Speed100; // ���x(100km/h�̌�)
	panel[46] = g_tims.Speed10; // ���x(10km/h�̌�)
	panel[47] = g_tims.Speed1; // ���x(1km/h�̌�)
	panel[50] = g_tims.Speed; // ���x�v

	// panel[46] = (g_speed != 0) && (g_accel.accel_kmhs > 2.2F) && (vehicleState.Current > 0) ? 1 + ((g_time % 1200) / 600) : 0; // ��](�_��)
	// panel[47] = (g_speed != 0) && (g_accel.accel_kmhs < -5.2F) ? 1 + ((g_time % 1200) / 600) : 0; // ����(�_��)

//�u���[�L����������
	panel[122] = g_tims.BcCaution ? ((g_time % 1000) / 500) : 0; // 200kPa�x��
	panel[123] = g_tims.BcPress0; // �u���[�L�V�����_�w�j(0-180kPa)
	panel[124] = g_tims.BcPress1; // �u���[�L�V�����_�w�j(200-380kPa)
	panel[125] = g_tims.BcPress2; // �u���[�L�V�����_�w�j(400-580kPa)
	panel[126] = g_tims.BcPress3; // �u���[�L�V�����_�w�j(600-780kPa)
	panel[127] = g_tims.MrPress0; // ����C�_���w�j(750-795kPa)
	panel[128] = g_tims.MrPress1; // ����C�_���w�j(800-845kPa)
	panel[129] = g_tims.MrPress2; // ����C�_���w�j(850-895kPa)
	/*
	//�ǉ����A�f�W�^���\���̏ꍇ�͂�����g�p
	panel[null] = g_tims.BcPress4; // �u���[�L�V�����_�w�j(100��)
	panel[null] = g_tims.BcPress5; // �u���[�L�V�����_�w�j(10��)
	panel[null] = g_tims.BcPress6; // �u���[�L�V�����_�w�j(1��)
	panel[null] = g_tims.BcPress7; // �u���[�L�V�����_�w�j(int)
	panel[null] = g_tims.MrPress4; // ����C�_���w�j(100��)
	panel[null] = g_tims.MrPress5; // ����C�_���w�j(10��)
	panel[null] = g_tims.MrPress6; // ����C�_���w�j(1��)
	panel[null] = g_tims.MrPress7; // ����C�_���w�j(int)
	*/
//�u���[�L���I��

//TIMS�S�ʕ\����������
	panel[100] = g_tims.TimsSpeed100; // TIMS���x�v(100km/h�̌�)
	panel[101] = g_tims.TimsSpeed10; // TIMS���x�v(10km/h�̌�)
	panel[102] = g_tims.TimsSpeed1; // TIMS���x�v(1km/h�̌�)

	panel[103] = g_tims.Distance1000; // ���s����(km�̌�)
	panel[104] = g_tims.Distance100; // ���s����(100m�̌�)
	//panel[] = g_tims.Distance10; // ���s����(10m�̌�)

	panel[105] = g_tims.UnitTims[0]; // TIMS���j�b�g�\��1
	panel[106] = g_tims.UnitTims[1]; // TIMS���j�b�g�\��2
	panel[107] = g_tims.UnitTims[2]; // TIMS���j�b�g�\��3
	panel[108] = g_tims.UnitTims[3]; // TIMS���j�b�g�\��4

	panel[109] = g_tims.ArrowDirection; // �i�s�������
	panel[110] = g_tims.TrainArrow; // �s�H�\���
	panel[111] = g_tims.Kind; // ��Ԏ��
	panel[112] = g_tims.Number[0]; // ��Ԕԍ�(��̌�)
	panel[113] = g_tims.Number[1]; // ��Ԕԍ�(�S�̌�)
	panel[114] = g_tims.Number[2]; // ��Ԕԍ�(�\�̌�)
	panel[115] = g_tims.Number[3]; // ��Ԕԍ�(��̌�)
	panel[116] = g_tims.Charactor; // ��Ԕԍ�(�L��)

	panel[117] = g_tims.Number[3] != 0 ? 1 : 0; // �ݒ芮��
	panel[118] = g_tims.PassMode; // �ʉߐݒ�
	panel[119] = g_tims.NextBlinkLamp; // ���w��ԕ\����
	panel[120] = g_tims.From; // �^�s�p�^�[�����w
	panel[121] = g_tims.Destination; // �^�s�p�^�[�����w
//TIMS�S�ʕ\���I��

//TIMS�d���ނ�������
	panel[217] = g_tims.AC; // ��
	panel[218] = g_tims.DC; // ����
	panel[219] = g_tims.CVacc; // ����d���ُ�
	panel[220] = g_tims.CVacc10; // ����d��(10��)
	panel[221] = g_tims.CVacc1; // ����d��(1��)
	panel[222] = g_tims.ACacc; // �𗬓d���ُ�
	panel[223] = g_tims.ACacc10000; // �𗬓d��(10000��)
	panel[224] = g_tims.ACacc1000; // �𗬓d��(1000��)
	panel[225] = g_tims.ACacc100; // �𗬓d��(100��)
	panel[226] = g_tims.DCacc; // �����d���ُ�
	panel[227] = g_tims.DCacc1000; // �����d��(1000��)
	panel[228] = g_tims.DCacc100; // �����d��(100��)
	panel[229] = g_tims.DCacc10; // �����d��(10��)
	panel[230] = g_tims.Cvmeter; // ����w�j
	panel[231] = g_tims.Acmeter; // �𗬎w�j
	panel[232] = g_tims.Dcmeter; // �����w�j
	panel[233] = g_tims.Accident; // ����
	panel[234] = g_tims.Tp; // �O��
	panel[236] = g_tims.VCB; // VCB(DigitalNumber�A0���\���`1���`2��)
	panel[235] = g_tims.alert_ACDC > 0 ? g_tims.alert_ACDC + ((g_time % 800) / 400) : 0; // �𒼐ؑ�(�_�ŁA���\��0�A�_��1-2�A3-4�Ńy�A)
//TIMS�d���ޏI��

//�d�񋤒ʕ�����������
	panel[130] = g_tims.HiddenLine[0] ? 0 : g_tims.Station[0]; // �w���\��1
	panel[131] = g_tims.HiddenLine[1] ? 0 : g_tims.Station[1]; // �w���\��2
	panel[132] = g_tims.HiddenLine[2] ? 0 : g_tims.Station[2]; // �w���\��3
	panel[133] = g_tims.HiddenLine[3] ? 0 : g_tims.Station[3]; // �w���\��4
	panel[134] = g_tims.HiddenLine[4] ? 0 : g_tims.Station[4]; // �w���\��5

	panel[135] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][0]; // ��������1H
	panel[136] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][1]; // ��������1M
	panel[137] = g_tims.HiddenLine[0] ? 0 : g_tims.Arrive[0][2]; // ��������1S
	panel[138] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][0]; // ��������2H
	panel[139] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][1]; // ��������2M
	panel[140] = g_tims.HiddenLine[1] ? 0 : g_tims.Arrive[1][2]; // ��������2S
	panel[141] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][0]; // ��������3H
	panel[142] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][1]; // ��������3M
	panel[143] = g_tims.HiddenLine[2] ? 0 : g_tims.Arrive[2][2]; // ��������3S

	panel[144] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][0]; // ��������4H
	panel[145] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][1]; // ��������4M
	panel[146] = g_tims.HiddenLine[3] ? 0 : g_tims.Arrive[3][2]; // ��������4S
	panel[147] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][0]; // ��������5H
	panel[148] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][1]; // ��������5M
	panel[149] = g_tims.HiddenLine[4] ? 0 : g_tims.Arrive[4][2]; // ��������5S

	panel[150] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][0]; // �o������1H
	panel[151] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][1]; // �o������1M
	panel[152] = g_tims.HiddenLine[0] ? 0 : g_tims.Leave[0][2]; // �o������1S
	panel[153] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][0]; // �o������2H
	panel[154] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][1]; // �o������2M
	panel[155] = g_tims.HiddenLine[1] ? 0 : g_tims.Leave[1][2]; // �o������2S
	panel[156] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][0]; // �o������3H
	panel[157] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][1]; // �o������3M
	panel[158] = g_tims.HiddenLine[2] ? 0 : g_tims.Leave[2][2]; // �o������3S

	panel[159] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][0]; // �o������4H
	panel[160] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][1]; // �o������4M
	panel[161] = g_tims.HiddenLine[3] ? 0 : g_tims.Leave[3][2]; // �o������4S
	panel[162] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][0]; // �o������5H
	panel[163] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][1]; // �o������5M
	panel[164] = g_tims.HiddenLine[4] ? 0 : g_tims.Leave[4][2]; // �o������5S

	panel[165] = g_tims.HiddenLine[0] ? 0 : g_tims.Track[0]; // ���w�����Ԑ�1
	panel[166] = g_tims.HiddenLine[1] ? 0 : g_tims.Track[1]; // ���w�����Ԑ�2
	panel[167] = g_tims.HiddenLine[2] ? 0 : g_tims.Track[2]; // ���w�����Ԑ�3
	panel[168] = g_tims.HiddenLine[3] ? 0 : g_tims.Track[3]; // ���w�����Ԑ�4
	panel[169] = g_tims.HiddenLine[4] ? 0 : g_tims.Track[4]; // ���w�����Ԑ�5
//�d�񋤒ʕ��������܂�

//��ԃX�^�t��������

	panel[170] = g_tims.HiddenLine[5] ? 0 : g_tims.Station[5]; // �w���\��6
	panel[171] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][0]; // ��������6H
	panel[172] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][1]; // ��������6M
	panel[173] = g_tims.HiddenLine[5] ? 0 : g_tims.Arrive[5][2]; // ��������6S
	panel[174] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][0]; // �o������6H
	panel[175] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][1]; // �o������6M
	panel[176] = g_tims.HiddenLine[5] ? 0 : g_tims.Leave[5][2]; // �o������6S
	panel[177] = g_tims.HiddenLine[5] ? 0 : g_tims.Track[5]; // ���w�����Ԑ�6

	panel[178] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][0]; // �w�ԑ��s����12M
	panel[179] = g_tims.HiddenLine[0] ? 0 : g_tims.Span[0][1]; // �w�ԑ��s����12S
	panel[180] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][0]; // �w�ԑ��s����23M
	panel[181] = g_tims.HiddenLine[1] ? 0 : g_tims.Span[1][1]; // �w�ԑ��s����23S
	panel[182] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][0]; // �w�ԑ��s����34M
	panel[183] = g_tims.HiddenLine[2] ? 0 : g_tims.Span[2][1]; // �w�ԑ��s����34S
	panel[184] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][0]; // �w�ԑ��s����45M
	panel[185] = g_tims.HiddenLine[3] ? 0 : g_tims.Span[3][1]; // �w�ԑ��s����45S
	panel[186] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][0]; // �w�ԑ��s����56M
	panel[187] = g_tims.HiddenLine[4] ? 0 : g_tims.Span[4][1]; // �w�ԑ��s����56S

	panel[188] = g_tims.HiddenLine[0] ? 0 : g_tims.Limit[0]; // �������x1IN
	panel[189] = g_tims.HiddenLine[0] ? 0 : g_tims.Limit2[0]; // �������x1OUT
	panel[190] = g_tims.HiddenLine[1] ? 0 : g_tims.Limit[1]; // �������x2IN
	panel[191] = g_tims.HiddenLine[1] ? 0 : g_tims.Limit2[1]; // �������x2OUT
	panel[192] = g_tims.HiddenLine[2] ? 0 : g_tims.Limit[2]; // �������x3IN
	panel[193] = g_tims.HiddenLine[2] ? 0 : g_tims.Limit2[2]; // �������x3OUT
	panel[194] = g_tims.HiddenLine[3] ? 0 : g_tims.Limit[3]; // �������x4IN
	panel[195] = g_tims.HiddenLine[3] ? 0 : g_tims.Limit2[3]; // �������x4OUT
	panel[196] = g_tims.HiddenLine[4] ? 0 : g_tims.Limit[4]; // �������x5IN
	panel[197] = g_tims.HiddenLine[4] ? 0 : g_tims.Limit2[4]; // �������x5OUT
	panel[198] = g_tims.HiddenLine[5] ? 0 : g_tims.Limit[5]; // �������x6IN
	panel[199] = g_tims.HiddenLine[5] ? 0 : g_tims.Limit2[5]; // �������x6OUT
//��ԃX�^�t�����܂�


//�d�ԃX�^�t��������
/*
	panel[170] = g_tims.After; // ���̎��w�\��
	panel[171] = g_tims.AfterStationTimeA[0]; // ���̎��w��������H
	panel[172] = g_tims.AfterStationTimeA[1]; // ���̎��w��������M
	panel[173] = g_tims.AfterStationTimeA[2]; // ���̎��w��������S
	panel[174] = g_tims.AfterStationTimeL[0]; // ���̎��w�o������H
	panel[175] = g_tims.AfterStationTimeL[1]; // ���̎��w�o������M
	panel[176] = g_tims.AfterStationTimeL[2]; // ���̎��w�o������S
	panel[177] = g_tims.AfterTrack; // ���w�����Ԑ�

	panel[178] = g_tims.Before; // ���O�̎��w�w���\��
	panel[179] = g_tims.BeforeStationTime[0]; // ���O�̎��w��������H
	panel[180] = g_tims.BeforeStationTime[1]; // ���O�̎��w��������M
	panel[181] = g_tims.BeforeStationTime[2]; // ���O�̎��w��������S
	panel[182] = g_tims.BeforeTrack; // ���O�̎��w�w���\��

	panel[183] = g_tims.RelayName; // �~�ԉw�w���\��
	panel[184] = g_tims.LastStopTime[0]; // �~�ԉw��������H
	panel[185] = g_tims.LastStopTime[1]; // �~�ԉw��������M
	panel[186] = g_tims.LastStopTime[2]; // �~�ԉw��������S
	panel[187] = g_tims.LastStopTimeL[0]; // �~�ԉw���Ԏ���H
	panel[188] = g_tims.LastStopTimeL[1]; // �~�ԉw���Ԏ���M
	panel[189] = g_tims.LastStopTimeL[2]; // �~�ԉw���Ԏ���S
	panel[190] = g_tims.LastStopTrack; // �~�ԉw�����Ԑ�

	panel[191] = g_tims.AfterKind; // ���s�H���
	panel[192] = g_tims.AfterNumber[0]; // ���s�H���
	panel[193] = g_tims.AfterNumber[1]; // ���s�H���
	panel[194] = g_tims.AfterNumber[2]; // ���s�H���
	panel[195] = g_tims.AfterNumber[3]; // ���s�H���
	panel[196] = g_tims.AfterChara; // ���s�H���

	panel[197] = g_tims.AfterTime[0] ? 0 : g_tims.AfterTime[0][0]; // ���s�H��������H
	panel[198] = g_tims.AfterTime[0] ? 0 : g_tims.AfterTime[0][1]; // ���s�H��������M
	panel[199] = g_tims.AfterTime[0] ? 0 : g_tims.AfterTime[0][2]; // ���s�H��������S
	panel[200] = g_tims.AfterTime[1] ? 0 : g_tims.AfterTime[1][0]; // ���s�H�o������H
	panel[201] = g_tims.AfterTime[1] ? 0 : g_tims.AfterTime[1][1]; // ���s�H�o������M
	panel[202] = g_tims.AfterTime[1] ? 0 : g_tims.AfterTime[1][2]; // ���s�H�o������S
*/
//�d�ԃX�^�t�I��


//�d�񋤒ʕ�����������
	panel[203] = g_tims.HiddenLine[3] ? 0 : (g_tims.Next * g_tims.NextBlink); // �w���\��(���w�A�_�ł���)
	panel[204] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[0]; // ��������(���w�A��)
	panel[205] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[1]; // ��������(���w�A��)
	panel[206] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTime[2]; // ��������(���w�A�b)
	panel[207] = g_tims.HiddenLine[3] ? 0 : g_tims.NextTrack; // ���w�����Ԑ�
	panel[208] = g_tims.For; // ��ԍs��
//�d�񋤒ʕ��������܂�

//���̂ق��@�\

	// panel[212] = g_date.Cooler; // ��[���


/* >>EXPORT
��Panel�C���f�b�N�X
ats22 ���x�v
ats23 ���x(100km/h�̌�)
ats24 ���x(10km/h�̌�)
ats25 ���x(1km/h�̌�)
ats26 �d���v[A]
ats30 �u���[�L�i
ats31 ���u���[�L
ats37 �f�W�^����
ats38 �f�W�^����
ats39 �f�W�^���b
ats41 ���j�b�g�\����1
ats42 ���j�b�g�\����2
ats43 ���j�b�g�\����3
ats44 ���j�b�g�\����4
ats110 �u���[�L�V�����_�w�j(0-180kPa)
ats111 �u���[�L�V�����_�w�j(200-380kPa)
ats112 �u���[�L�V�����_�w�j(400-580kPa)
ats113 �u���[�L�V�����_�w�j(600-780kPa)
ats114 ����C�_���w�j(750-795kPa)
ats115 ����C�_���w�j(800-845kPa)
ats116 ����C�_���w�j(850-895kPa)
ats145 TIMS���x�v(100km/h�̌�)
ats146 TIMS���x�v(10km/h�̌�)
ats147 TIMS���x�v(1km/h�̌�)
ats148 ���s����(km�̌�)
ats149 ���s����(100m�̌�)
ats150 TIMS���j�b�g�\��1
ats151 TIMS���j�b�g�\��2
ats152 TIMS���j�b�g�\��3
ats153 TIMS���j�b�g�\��4
ats154 �i�s�������

ats155 ��Ԏ��
���C���f�b�N�X�ύX

ats157 ��Ԕԍ�(��̌�)
ats158 ��Ԕԍ�(�S�̌�)
ats159 ��Ԕԍ�(�\�̌�)
ats160 ��Ԕԍ�(��̌�)
ats161 ��Ԕԍ�(�L��)
ats162 �ݒ芮��
ats163 �ʉߐݒ�
ats165 �w���\��1
ats166 �w���\��2
ats167 �w���\��3
ats168 ��������1H
ats169 ��������1M
ats170 ��������1S
ats171 ��������2H
ats172 ��������2M
ats173 ��������2S
ats174 ��������3H
ats175 ��������3M
ats176 ��������3S
ats177 �o������1H
ats178 �o������1M
ats179 �o������1S
ats180 �o������2H
ats181 �o������2M
ats182 �o������2S
ats183 �o������3H
ats184 �o������3M
ats185 �o������3S
ats186 �w�ԑ��s����12M
ats187 �w�ԑ��s����12S
ats188 �w�ԑ��s����23M
ats189 �w�ԑ��s����23S
ats190 ���w�����Ԑ�1
ats191 ���w�����Ԑ�2
ats192 ���w�����Ԑ�3
ats193 �������x1IN
ats194 �������x1OUT
ats195 �������x2IN
ats196 �������x2OUT
ats197 �������x3IN
ats198 �������x3OUT
ats199 �w���\��(���w�A�_�ł���)
ats200 ��������(���w�A��)
ats201 ��������(���w�A��)
ats202 ��������(���w�A�b)
ats203 ���w�����Ԑ�
ats206 200kPa�x��
ats208 ���w��ԕ\����
ats217 ��
ats218 ����
ats219 ����d���ُ�
ats220 ����d��(10��)
ats221 ����d��(1��)
ats222 �𗬓d���ُ�
ats223 �𗬓d��(10000��)
ats224 �𗬓d��(1000��)
ats225 �𗬓d��(100��)
ats226 �����d���ُ�
ats227 �����d��(1000��)
ats228 �����d��(100��)
ats229 �����d��(10��)
ats230 ����w�j
ats231 �𗬎w�j
ats232 �����w�j
ats233 ����
ats234 �O��
ats236 VCB(DigitalNumber�A0���\���`1���`2��)
ats235 �𒼐ؑ�(�_��)

���ǉ��@�\
ats155 TIMS��Ԏ��(�Čf)
��156��155�ɕύX���܂���
ats156 TIMS�s�H�\���
��155��156�ɕύX���܂���

ats164 TIMS�s��

ats213 TIMS�^�s�p�^�[�����w
ats214 TIMS�^�s�p�^�[�����w
*/

	// �T�E���h�o��
	// sound[90] = g_sub.KeyOn;
	// sound[91] = g_sub.KeyOff;
	// sound[25] = g_tims.RelayD;
	sound[225] = g_tims.LbInit;
	sound[100] = g_tims.AirApply;
	sound[101] = g_tims.AirApplyEmg;
	sound[103] = g_tims.AirHigh;
	sound[105] = g_tims.EmgAnnounce;
	sound[106] = g_tims.UpdateInfo;
	// sound[106] = (sound[106] == ATS_SOUND_PLAYLOOPING) ? ATS_SOUND_CONTINUE : (g_date.Cooler == 1 ? ATS_SOUND_PLAYLOOPING : ATS_SOUND_STOP);
	sound[14] = g_spp.HaltChime3;
	sound[15] = g_spp.HaltChime;
	sound[16] = g_spp.PassAlarm;
	sound[17] = g_spp.HaltChime2;

/* >>EXPORT
��Sound�C���f�b�N�X
ats15 ��ʉߖh�~ (��ԁA1��)
ats16 ��ʉߖh�~ (�ʉ�)
ats17 ��ʉߖh�~ (��ԁA���[�v)
ats100 �u���[�L������
ats101 �u���[�L������(���)
ats103 ���u���[�L�ɉ���
ats105 ���u���[�L����
ats106 �^�s���X�V�`���C��
ats225 HB(LbInit)
*/

	return g_output;
}

ATS_API void WINAPI SetPower(int notch)
{
	g_powerNotch = notch;
	g_tims.RandomizeSeed();
}

ATS_API void WINAPI SetBrake(int notch)
{
	g_tims.PlaySoundAirHigh(g_brakeNotch, notch);
	g_tims.PlaySoundEmgAnnounce(g_brakeNotch, notch);
	g_tims.PlaySoundAirApplyEmg(g_brakeNotch, notch);
	g_spp.NotchChanged();

	g_brakeNotch = notch;
	g_tims.RandomizeSeed();
}

ATS_API void WINAPI SetReverser(int pos)
{
	g_reverser = pos;
	g_tims.SetLbInit(pos, g_tims.event_lbInit);
}

ATS_API void WINAPI KeyDown(int atsKeyCode)
{
	/*
	switch(atsKeyCode)
	{
	case ATS_KEY_I: // �}�X�R���L�[����
		g_sub.KeyEvent(0);
		break;
	case ATS_KEY_J: // �}�X�R���L�[����
		g_sub.KeyEvent(1);
		break;
	}
	*/
}

ATS_API void WINAPI KeyUp(int atsKeyCode)
{
}

/* >>EXPORT
���L�[�A�T�C��
�Ȃ�
*/

ATS_API void WINAPI HornBlow(int hornType)
{
}

ATS_API void WINAPI DoorOpen(void)
{
	g_pilotlamp = false;
	g_tims.DoorOpening();
	g_spp.StopChime(); // ��ʂ̃`���C�����~�߂�
}

ATS_API void WINAPI DoorClose(void)
{
	g_pilotlamp = true;
	g_tims.DoorClosing();
}

ATS_API void WINAPI SetSignal(int signal)
{
	g_tims.CheckPath(signal);
}

/* >>EXPORT
��Signal�C���f�b�N�X
�Ȃ�
*/

ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA beaconData)
{
	switch(beaconData.Type)
	{
	case ATS_BEACON_SPP: // ��ԉw�ʉߖh�~���u
		if(g_speed != 0){g_spp.Receive(beaconData.Optional % 10000);} // �w�W�����v�����O����
		break;
	/*
	case ATS_BEACON_UPD: // TIMS�X�V�錾
		g_tims.UpdateView();
		break;
	*/
		/*20180824�b��P��
	case ATS_BEACON_APP: // TIMS���w�ڋ�
		if(g_speed != 0){g_spp.Receive(beaconData.Optional % 10000);} // �w�W�����v�����O����
		g_tims.Receive(beaconData.Optional); // �w�W�����v�����O���Ȃ�
		break;
		*/
	//20180824�b���
	case ATS_BEACON_APP: // TIMS���w�ڋ�
		if(g_speed != 0){g_spp.Receive(beaconData.Optional % 10000);} // �w�W�����v�����O����
		g_tims.Receive(beaconData.Optional % 10000000, beaconData.Optional / 10000000); // �w�W�����v�����O���Ȃ�
		break;

	case ATS_BEACON_NEXT: // TIMS���w�ݒ�
		g_tims.SetNext(beaconData.Optional);
		break;
	case ATS_BEACON_NEXTTIME: // TIMS���w���������ݒ�
		g_tims.SetNextTime(beaconData.Optional);
		break;
	case ATS_BEACON_NEXTTRL: // TIMS���w�����Ԑ��ݒ�
		g_tims.SetNextTrack(beaconData.Optional);
		break;
	case ATS_BEACON_KIND: // TIMS��Ԏ��
		g_tims.SetKind(beaconData.Optional);
		break;
	case ATS_BEACON_NUM: // TIMS��Ԕԍ�
		g_tims.SetNumber(beaconData.Optional / 100, beaconData.Optional % 100);
		break;
	case ATS_BEACON_STA: // TIMS�w���\��
		g_tims.InputLine(1, (beaconData.Optional / 1000) - 1, beaconData.Optional % 1000); //1000�Ŋ����Ă���-1�����ԍ��ɁA�]��͉w�ԍ�
		break;
	case ATS_BEACON_ARV: // TIMS��������
		g_tims.InputLine(2, (beaconData.Optional / 1000000) - 1, beaconData.Optional % 1000000); //1000000�Ŋ����Ă���-1�����ԍ��ɁA�]��͎���
		break;
	case ATS_BEACON_LEV: // TIMS�o������
		g_tims.InputLine(3, (beaconData.Optional / 1000000) - 1, beaconData.Optional % 1000000); //1000000�Ŋ����Ă���-1�����ԍ��ɁA�]��͎���
		break;
	case ATS_BEACON_TRL: // TIMS�����Ԑ�
		g_tims.InputLine(4, (beaconData.Optional / 100) - 1, (beaconData.Optional % 100)); // �����Ԑ��A100�Ŋ����Ă���-1�����ԍ��ɁA�]��͔Ԑ�
		break;
	case ATS_BEACON_LIM1: // TIMS�������xIN
		g_tims.InputLine(5, (beaconData.Optional / 100) - 1, (beaconData.Optional % 100)); // �������xIN�A100�Ŋ����Ă���-1�����ԍ��ɁA�]��͐���
		break;
	case ATS_BEACON_LIM2: // TIMS�������xOUT
		g_tims.InputLine(6, (beaconData.Optional / 100) - 1, (beaconData.Optional % 100)); // �������xOUT�A100�Ŋ����Ă���-1�����ԍ��ɁA�]��͐���
		break;
	case ATS_BEACON_SPN: // TIMS�w�Ԏ���
		g_tims.InputLine(0, (beaconData.Optional / 10000) - 1, beaconData.Optional % 10000); //10000�Ŋ����Ă���-1�����ԍ��ɁA�]��͉w�Ԏ���
		break;
	/*
	case ATS_BEACON_PATH: // TIMS�J�ʏ��
		g_tims.CheckPath(beaconData.Signal, beaconData.Distance, beaconData.Optional);
		break;
	case ATS_BEACON_CRWL: // TIMS���s���
		g_tims.SetCrawl(beaconData.Optional);
		break;
	*/
	case ATS_BEACON_LEG: // TIMS�^�s���
		g_tims.SetLeg(beaconData.Optional);
		break;
	
	case ATS_BEACON_DIST: // TIMS�������Z�ݒ�
		g_tims.SetDistance(beaconData.Distance, beaconData.Optional);
		break;
	/*
	case ATS_BEACON_DISDEF: // TIMS��Ԉʒu�␳
		g_tims.SetPositionDef(beaconData.Optional);
		break;
	*/

	case ATS_BEACON_SUBK: // TIMS���^�p���
		g_tims.SetAfteruent(3, beaconData.Optional, 0);
		break;
	
	case ATS_BEACON_SUBN: // TIMS���^�p���
		g_tims.SetAfteruent(0, beaconData.Optional / 100, beaconData.Optional % 100);
		break;
	
	case ATS_BEACON_SUBA: // TIMS���^�p����
		g_tims.SetAfteruent(1, beaconData.Optional, 0);
		break;
	case ATS_BEACON_SUBL: // TIMS���^�p����
		g_tims.SetAfteruent(2, beaconData.Optional, 0);
		break;
	
	case ATS_BEACON_DIR: // TIMS�i�s�����ݒ�
		g_tims.SetDirection(beaconData.Optional);
		break;
	
	case ATS_BEACON_LSA: // TIMS�I�_��������
		g_tims.SetLastStop(0, beaconData.Optional);
		break;
	case ATS_BEACON_LSL: // TIMS�I�_�o������
		g_tims.SetLastStop(1, beaconData.Optional);
		break;
	case ATS_BEACON_LST: // TIMS�I�_�����Ԑ�
		g_tims.SetLastStop(2, beaconData.Optional);
		break;
	case ATS_BEACON_LSN: // TIMS�s�H�I�_�w��
		g_tims.SetRelayStation(beaconData.Optional);
		break;
	
	case ATS_BEACON_ARW: // TIMS�s�H�\���
		g_tims.SetArrowState(beaconData.Optional);
		break;

	case ATS_BEACON_FOR: // TIMS�s�H�\�s��
		g_tims.SetFor(beaconData.Optional);
		break;



	case ATS_BEACON_BTSL: // TIMS���O�̎��w����
		g_tims.SetTimeStationTime(0, beaconData.Optional);
		break;
	case ATS_BEACON_ATSA: // TIMS���̎��w������
		g_tims.SetTimeStationTime(1, beaconData.Optional);
		break;
	case ATS_BEACON_ATSL: // TIMS���̎��w������
		g_tims.SetTimeStationTime(2, beaconData.Optional);
		break;
	case ATS_BEACON_BTST: // TIMS���O�̎��w�Ԑ�
		g_tims.SetTimeStationTime(3, beaconData.Optional);
		break;
	case ATS_BEACON_ATST: // TIMS���̎��w�Ԑ�
		g_tims.SetTimeStationTime(4, beaconData.Optional);
		break;

	case ATS_BEACON_BTSN: // TIMS���O�̎��w�w��
		g_tims.SetTimeStationName(0, beaconData.Optional);
		break;
	case ATS_BEACON_ATSN: // TIMS���̎��w�w��
		g_tims.SetTimeStationName(1, beaconData.Optional);
		break;

	
	case 120: // �ؑւ��m�点
		g_tims.AlartACDC(beaconData.Optional);
		break;
	case 121: // �𒼐ؑ�
		g_tims.SetACDC(beaconData.Optional);
		break;
	}
}

/* >>EXPORT
��Beacon�C���f�b�N�X
30 ��ԉw�ʉߖh�~���u
sendData[m] �ɒ�~�ʒu�܂ł̋�����ݒ�A�ʉ߉w�̏ꍇ�͕��̒l

100 TIMS��Ԕԍ�
sendData �ɗ�ԕ\����̃V�t�g�P�� (�L��2��+���4��) ��ݒ�

101 TIMS��Ԏ��
sendData �Ɏ�ʕ\����̃V�t�g�P�ʂ�ݒ�
sendData+100 �Œʉߐݒ�

102 TIMS�i�s�����ݒ�
sendData �ɗ�Ԃ̐i�s���� (TIMS��ʂ̍�����1�A�E����2) ��ݒ�

103 TIMS�������Z�ݒ�
sendData �ɐݒ苗��/10m�̒l��ݒ�
�O�i���Č���^�C�v��+10000����
(��) 23k05�̏ꍇ�A2305��ݒ�

105 TIMS���w�ڋ�
sendData[m] �ɒ�~�ʒu�܂ł̋�����ݒ�ASPP (#30) �������Ɏ��s�����
�ʉ߉w�̏ꍇ�� sendData �𕉂̒l�ɂ��Đݒ肷��
sendData �̃I�v�V�����Ƃ��āA �ȉ����g�p�ł��܂� (���p�s��)
�@�ꖜ�̌�: �X�e�b�v�X�V����񐔂̐ݒ�A�ݒ�Ȃ���f�t�H���g��1
�@�\���̌�: ��čX�V����񐔂̐ݒ�A�X�V����񐔂�ݒ�
�@�S���̌�: �w�����X�V���Ȃ��ݒ�A1��ݒ肷��

106 TIMS���w�ݒ�
���w���� sendData �ɉw�ԍ� (���w2��+���w2��) ��ݒ�

107 TIMS���w���������ݒ�
���w������ HHmmss �̌`����6����ݒ� (e.g. 21:36:15 �� 213615)

108 TIMS���w�����Ԑ��ݒ�
���w�����Ԑ��� sendData �ɓ����Ԑ��摜�V�t�g�ԍ���ݒ�A0�𖳕\���Ƃ���

110 TIMS�w���\��
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �w�ԍ�
�@�\�̌�: �V
�@�S�̌��ȏ�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�1�s�ځA�w�ԍ���12�̏ꍇ�A112��ݒ�

111 TIMS��������
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: ��������(�b)�̈�̈�
�@�\�̌�: ��������(�b)�̏\�̈�
�@�S�̌�: ��������(��)�̈�̈�
�@��̌�: ��������(��)�̏\�̈�
�@�ꖜ�̌�: ��������(��)�̈�̈�
�@�\���̌�: ��������(��)�̏\�̈�
�@�S���̌��ȏ�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�6�s�ځA����������12:34:50�̏ꍇ�A6123450��ݒ�
(��2) �ݒ�3�s�ځA�������������\���̏ꍇ�A3246060��ݒ�

112 TIMS�o������
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �o������(�b)�̈�̈�
�@�\�̌�: �o������(�b)�̏\�̈�
�@�S�̌�: �o������(��)�̈�̈�
�@��̌�: �o������(��)�̏\�̈�
�@�ꖜ�̌�: �o������(��)�̈�̈�
�@�\���̌�: �o������(��)�̏\�̈�
�@�S���̌��ȏ�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�2�s�ځA�o��������22:15:05�̏ꍇ�A2221505��ݒ�
(��2) �ݒ�1�s�ځA�o�����������\���̏ꍇ�A1246060��ݒ�

113 TIMS�����Ԑ�
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �����Ԑ��̈�̈�
�@�\�̌�: �����Ԑ��̏\�̈�
�@�S�̌�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�4�s�ځA�����Ԑ���12�Ԑ��̏ꍇ�A412��ݒ�

114 TIMS�������xIN
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �������x�̈�̈� (5�̔{��)
�@�\�̌�: �������x�̏\�̈�
�@�S�̌�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�4�s�ځA�������x��35km/h�̏ꍇ�A435��ݒ�

115 TIMS�������xOUT
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �������x�̈�̈� (5�̔{��)
�@�\�̌�: �������x�̏\�̈�
�@�S�̌�: �ݒ�s�̔ԍ� (1�`10)

116 TIMS�w�Ԏ���
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �w�ԑ��s����(�b)�̈�̈�
�@�\�̌�: �w�ԑ��s����(�b)�̏\�̈�
�@�S�̌�: �w�ԑ��s����(��)�̈�̈�
�@��̌�: �w�ԑ��s����(��)�̏\�̈�
�@�ꖜ�̌��ȏ�: �ݒ�s�̔ԍ� (1�`10)
(��j �ݒ�1�s�ځA�w�ԑ��s���Ԃ�3��45�b�̏ꍇ�A10345��ݒ�

120 �𒼐ؑւ��m�点(��)
0�Ŗ����A1�ŗL��

121 �𒼐ؑ�(��)
0VCB�؁A1���d��ԁA2AC�L�d��ԁA3DC�L�d���

���ǉ��@�\
104 TIMS�^�s�p�^�[��
�w���� sendData �ɉw�ԍ� (���w2��+���w2��) ��ݒ�

109 TIMS�s��ݒ�
�w���� sendData �ɉw�ԍ� (�s��2��) ��ݒ�

117 TIMS�s�H�\���
sendData �ɍs�H�\���̃V�t�g�P�ʂ�ݒ�

��v0.04����̒ǉ��@�\

118 TIMS���s�H������
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: ��������(�b)�̈�̈�
�@�\�̌�: ��������(�b)�̏\�̈�
�@�S�̌�: ��������(��)�̈�̈�
�@��̌�: ��������(��)�̏\�̈�
�@�ꖜ�̌�: ��������(��)�̈�̈�
�@�\���̌�: ��������(��)�̏\�̈�

119 TIMS���s�H������
sendData �͈ȉ��̂悤�ɐݒ肷��
�@��̌�: �o������(�b)�̈�̈�
�@�\�̌�: �o������(�b)�̏\�̈�
�@�S�̌�: �o������(��)�̈�̈�
�@��̌�: �o������(��)�̏\�̈�
�@�ꖜ�̌�: �o������(��)�̈�̈�
�@�\���̌�: �o������(��)�̏\�̈�

*/
