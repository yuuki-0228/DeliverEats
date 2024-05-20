#include "XInput.h"
#include "..\..\..\FileManager\FileManager.h"

namespace {
	constexpr char WINDOW_SETTING_FILE_PATH[] = "Data\\Parameter\\Config\\WindowSetting.json";	// ウィンドウの設定のファイルパス.
}

// 値を範囲内に収める関数.
template<typename T>
T Clamp( T val, T min, T max )
{
	return ( val < min ) ? min : ( max < val ) ? max : val;
}

CXInput::CXInput()
	: m_padId			( 0 )
	, m_state			()
	, m_stateOld		()
	, m_vibration		()
	, m_IsVibration		( false, u8"XInputの振動を使用するか",		"Input" )
	, m_ButStop			( false, u8"XInputのボタンを停止させるか",	"Input" )
	, m_connect			( false )
	, m_IsNotActiveStop	( false )
{
	// ウィンドウの設定の取得.
	json WndSetting		= FileManager::JsonLoad( WINDOW_SETTING_FILE_PATH );
	m_IsNotActiveStop	= WndSetting["IsInputNotActiveStop"];
}

CXInput::~CXInput()
{
	EndProc();
}

//----------------------------.
// インスタンスの取得.
//----------------------------.
CXInput* CXInput::GetInstance()
{
	static std::unique_ptr<CXInput> pInstance = std::make_unique<CXInput>();
	return pInstance.get();
}

//----------------------------.
// キー入力の更新.
//----------------------------.
bool CXInput::Update()
{
	CXInput* pI = GetInstance();

	// キー情報を更新する前に退避.
	pI->m_stateOld = pI->m_state;
	if( pI->UpdateStatus() == false ){
		return false;
	}
	return true;
}

//----------------------------.
// 終了処理.
//----------------------------.
void CXInput::EndProc()
{
	SetVibration( 0, 0 );
}

//----------------------------.
// キーが押されたことを取得.
//	※引数:XINPUT_GAMEPAD_A など.
//----------------------------.
bool CXInput::IsKeyPress( WORD GamePad )
{
	CXInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !CDirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop.get() ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiPressed( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriPressed( GamePad );
	default:
		return IsButPressed( GamePad );
	}
}

//----------------------------.
// 押した瞬間.
//	※引数:XINPUT_GAMEPAD_A など.
//----------------------------.
bool CXInput::IsKeyDown( WORD GamePad )
{
	CXInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !CDirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop.get() ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiDown( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriDown( GamePad );
	default:
		return IsButDown( GamePad );
	}
}

//----------------------------.
// 離した瞬間.
//	※引数:XINPUT_GAMEPAD_A など.
//----------------------------.
bool CXInput::IsKeyUp( WORD GamePad )
{
	CXInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !CDirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop.get() ) return false;
	switch ( GamePad ) {
	case XINPUT_LSTICK_U:
	case XINPUT_LSTICK_D:
	case XINPUT_LSTICK_L:
	case XINPUT_LSTICK_R:
	case XINPUT_RSTICK_U:
	case XINPUT_RSTICK_D:
	case XINPUT_RSTICK_L:
	case XINPUT_RSTICK_R:
		return IsStiUp( GamePad );
	case XINPUT_LTRIGGER:
	case XINPUT_RTRIGGER:
		return IsTriUp( GamePad );
	default:
		return IsButUp( GamePad );
	}
}

//----------------------------.
// 押し続けている.
//	※引数:XINPUT_GAMEPAD_A など.
//----------------------------.
bool CXInput::IsKeyRepeat( WORD GamePad )
{
	CXInput* pI = GetInstance();
	if ( pI->m_IsNotActiveStop && !CDirectX11::IsWindowActive() ) return false;

	if ( pI->m_ButStop.get() ) return false;
	switch ( GamePad ) {
		case XINPUT_LSTICK_U:
		case XINPUT_LSTICK_D:
		case XINPUT_LSTICK_L:
		case XINPUT_LSTICK_R:
		case XINPUT_RSTICK_U:
		case XINPUT_RSTICK_D:
		case XINPUT_RSTICK_L:
		case XINPUT_RSTICK_R:
			return IsStiRepeat( GamePad );
		case XINPUT_LTRIGGER:
		case XINPUT_RTRIGGER:
			return IsTriRepeat( GamePad );
		default:
			return IsButRepeat( GamePad );
	}
}

//----------------------------.
// 振動設定.
//----------------------------.
bool CXInput::SetVibration( const float LMotorSpd, const float RMotorSpd)
{
	CXInput* pI = GetInstance();

	if ( pI->m_IsVibration.get() ) {
		pI->m_vibration.wLeftMotorSpeed  =
			static_cast<WORD>( INPUT_VIBRATION_MAX * Clamp( LMotorSpd, 0.0f, 1.0f ) );;
		pI->m_vibration.wRightMotorSpeed =
			static_cast<WORD>( INPUT_VIBRATION_MAX * Clamp( RMotorSpd, 0.0f, 1.0f ) );;

		if ( ERROR_SUCCESS == XInputSetState(
			pI->m_padId, &pI->m_vibration ) ) {
			return true;
		}
	}
	return false;
}

//----------------------------.
// キー入力の更新.
//----------------------------.
bool CXInput::UpdateStatus()
{
	m_connect = false;
	if( ERROR_SUCCESS == XInputGetState(
		m_padId,
		&m_state ) )
	{
		m_connect = true;
		return true;
	}
	return false;
}

//----------------------------.
// ボタンが押されたことを取得する.
//----------------------------.
bool CXInput::IsButPressed( WORD GamePad )
{
	return	GetInstance()->m_state.Gamepad.wButtons & GamePad;
}

//----------------------------.
// ボタンを押した瞬間.
//----------------------------.
bool CXInput::IsButDown( WORD GamePad )
{
	CXInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) == 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) != 0;
}

//----------------------------.
// ボタンを離した瞬間.
//----------------------------.
bool CXInput::IsButUp( WORD GamePad )
{
	CXInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) != 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) == 0;
}

//----------------------------.
// ボタンを押し続けている.
//----------------------------.
bool CXInput::IsButRepeat( WORD GamePad )
{
	CXInput* pI = GetInstance();

	return	( pI->m_stateOld.Gamepad.wButtons & GamePad ) != 0 &&
			( pI->m_state.Gamepad.wButtons & GamePad ) != 0;
}

//----------------------------.
// スティックが倒したことを取得する.
//----------------------------.
bool CXInput::IsStiPressed( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ) return pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_LSTICK_D ) return pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_LSTICK_L ) return pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_LSTICK_R ) return pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_RSTICK_U ) return pI->m_state.Gamepad.sThumbRY >= INPUT_THUMB_MAX;
	if ( GamePad == XINPUT_RSTICK_D ) return pI->m_state.Gamepad.sThumbRY <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_RSTICK_L ) return pI->m_state.Gamepad.sThumbRX <= INPUT_THUMB_MIN;
	if ( GamePad == XINPUT_RSTICK_R ) return pI->m_state.Gamepad.sThumbRX >= INPUT_THUMB_MAX;
	return false;
}

//----------------------------.
// スティックを倒した瞬間.
//----------------------------.
bool CXInput::IsStiDown( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbRY < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbRY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbRY > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbRY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbRX > INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbRX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbRX < INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbRX >= INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// スティックを戻した瞬間.
//----------------------------.
bool CXInput::IsStiUp( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY < INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX > INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX < INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// スティックを倒し続けている.
//----------------------------.
bool CXInput::IsStiRepeat( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_LSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_LSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_U ){
		return	pI->m_stateOld.Gamepad.sThumbLY >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLY >= INPUT_THUMB_MAX;
	}
	if ( GamePad == XINPUT_RSTICK_D ){
		return	pI->m_stateOld.Gamepad.sThumbLY <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLY <= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_L ){
		return	pI->m_stateOld.Gamepad.sThumbLX <= INPUT_THUMB_MIN &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MIN;
	}
	if ( GamePad == XINPUT_RSTICK_R ){
		return	pI->m_stateOld.Gamepad.sThumbLX >= INPUT_THUMB_MAX &&
				pI->m_state.Gamepad.sThumbLX >= INPUT_THUMB_MAX;
	}
	return false;
}

//----------------------------.
// トリガーが押されたことを取得する.
//----------------------------.
bool CXInput::IsTriPressed( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ) return pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	if ( GamePad == XINPUT_RTRIGGER ) return pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	return false;
}

//----------------------------.
// トリガーを押した瞬間.
//----------------------------.
bool CXInput::IsTriDown( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger < INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger < INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	}
	return false;
}

//----------------------------.
// トリガーを離した瞬間.
//----------------------------.
bool CXInput::IsTriUp( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger < INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger < INPUT_TRIGGER_MAX;
	}
	return false;
}

//----------------------------.
// トリガーを押し続けている.
//----------------------------.
bool CXInput::IsTriRepeat( WORD GamePad )
{
	CXInput* pI = GetInstance();

	if ( GamePad == XINPUT_LTRIGGER ){
		return	pI->m_stateOld.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bLeftTrigger >= INPUT_TRIGGER_MAX;
	}
	if ( GamePad == XINPUT_RTRIGGER ){
		return	pI->m_stateOld.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX &&
				pI->m_state.Gamepad.bRightTrigger >= INPUT_TRIGGER_MAX;
	}
	return false;
}
