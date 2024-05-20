#pragma once
#include "..\..\Global.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>

class CEffect;

/************************************************
*	�G�t�F�N�g���\�[�X�N���X.
*		���c�F�P.
**/
class CEffectResource final
{
public:
	using Effect_map	= std::unordered_map<std::string, std::unique_ptr<CEffect>>;
	using Effect_List	= std::vector<std::string>;

public:
	CEffectResource();
	~CEffectResource();

	// �C���X�^���X�̎擾.
	static CEffectResource* GetInstance();

	// �G�t�F�N�g�̓ǂݍ���.
	static HRESULT EffectLoad();

	// �G�t�F�N�g�擾�֐�.
	static CEffect* GetEffect( const std::string& FileName );

	// �ۑ����Ă���G�t�F�N�g���X�g�̎擾.
	static Effect_List GetEffectList() { return GetInstance()->m_EffectNames; }

private:
	Effect_map	m_EffectList;	// �G�t�F�N�g���X�g.
	Effect_List	m_EffectNames;	// �G�t�F�N�g�̖��O���X�g.
	std::mutex	m_Mutex;
	bool		m_IsLoadEnd;	// �ǂݍ��݂��I�������.

private:
	// �R�s�[�E���[�u�R���X�g���N�^, ������Z�q�̍폜.
	CEffectResource( const CEffectResource& )				= delete;
	CEffectResource& operator = ( const CEffectResource& )	= delete;
	CEffectResource( CEffectResource&& )					= delete;
	CEffectResource& operator = ( CEffectResource&& )		= delete;
};