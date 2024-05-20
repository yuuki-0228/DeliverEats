#pragma once
#include "..\SceneBase.h"

class CSprite;
class CUIEditor;

/************************************************
*	UI�G�f�B�^�V�[��.
*		���c�F�P.
**/
class CUIEdit final
	: public CSceneBase
{
public:
	CUIEdit();
	~CUIEdit();

	// ������.
	virtual bool Init() override;

	// �X�V.
	virtual void Update( const float& DeltaTime ) override;

	// ���f���̕`��.
	virtual void ModelRender() override {};
	// �X�v���C�g(UI)�̕`��.
	virtual void SpriteUIRender() override;
	// �X�v���C�g(3D)/Effect�̕`��.
	//	_A�F��ɕ\������� / _B�F��ɕ\�������.
	virtual void Sprite3DRender_A() override {};
	virtual void Sprite3DRender_B() override {};

private:
	std::unique_ptr<CUIEditor>	m_pUIEditor;		// UI�G�f�B�^.

};