#include "FontResource.h"
#include "..\..\Common\Font\FontCreate\FontCreate.h"
#include "..\..\Utility\FileManager\FileManager.h"

namespace {
	// フォントファイルパス.
	constexpr char FILE_PATH[] = "Data\\Sprite\\Font";
}

CFontResource::CFontResource()
	: m_FontList		()
	, m_FontTextureList	()
	, m_FontDataList	()
	, m_FontNames		()
	, m_Mutex			()
	, m_IsLoadEnd		( false )
{
}

CFontResource::~CFontResource()
{
	for ( auto& [FileName, List] : m_FontTextureList ) {
		for ( auto& [Key, Texture] : List ) {
			SAFE_RELEASE( Texture );
		}
	}
}

//---------------------------.
// インスタンスの取得.
//---------------------------.
CFontResource* CFontResource::GetInstance()
{
	static std::unique_ptr<CFontResource> pInstance = std::make_unique<CFontResource>();
	return pInstance.get();
}

//---------------------------.
// フォントの読み込み.
//---------------------------.
HRESULT CFontResource::FontLoad()
{
	CFontResource* pI = GetInstance();
	std::unique_lock<std::mutex> Lock( pI->m_Mutex );

	auto SpriteLoad = [&]( const std::filesystem::directory_entry& Entry )
	{
		const std::string Extension = Entry.path().extension().string();	// 拡張子.
		const std::string FilePath	= Entry.path().string();				// ファイルパス.
		const std::string FileName	= Entry.path().stem().string();			// ファイル名.

		// 拡張子が ".ttf" / ".otf" ではない場合読み込まない.
		if ( Extension != ".ttf" && Extension != ".TTF" &&
			 Extension != ".otf" && Extension != ".OTF" ) return;

		// フォントデータを保存.
		pI->m_FontDataList[FileName] = FilePath;

		// フォントの読み込み.
		pI->m_FontList[FileName] = std::make_unique<CFont>();
		if ( FAILED( pI->m_FontList[FileName]->Init( FilePath, FileName ) ) ) throw E_FAIL;
		pI->m_FontNames.emplace_back( FileName );

		CLog::PushLog( FilePath + " 読み込み : 成功" );
	};

	CLog::PushLog( "------ フォント読み込み開始 ------" );
	try {
		std::filesystem::recursive_directory_iterator Dir_itr( FILE_PATH ), End_itr;
		std::for_each( Dir_itr, End_itr, SpriteLoad );
	}
	catch ( const std::filesystem::filesystem_error& e ) {
		// エラーメッセージを表示.
		ErrorMessage( e.path1().string().c_str() );
		return E_FAIL;
	}
	CLog::PushLog( "------ フォント読み込み終了 ------" );

	// 読み込み終了.
	pI->m_IsLoadEnd = true;
	return S_OK;
}

//---------------------------.
// フォントの取得.
//---------------------------.
CFont* CFontResource::GetFont( const std::string& FileName, SFontRenderState* pState )
{
	// 読み込みが終わっていなかったら null を返す.
	if ( GetInstance()->m_IsLoadEnd == false )
		return nullptr;

	// 指定したエフェクトを取得.
	for ( auto& s : GetInstance()->m_FontList ) {
		if ( s.first != FileName	) continue;
		if ( pState  != nullptr		) *pState = s.second->GetRenderState();
		return s.second.get();
	}

	ErrorMessage( FileName + " sounds not found" );
	return nullptr;
}
CFont* CFontResource::GetFont( const std::string& FileName, SFontRenderStateList* pState )
{
	CFont*				Out;
	SFontRenderState	State;
	Out = GetFont( FileName, &State );
	for ( auto& s : *pState ) s = State;
	return Out;
}

//---------------------------.
// フォントテクスチャ取得関数.
//---------------------------.
ID3D11ShaderResourceView* CFontResource::GetFontTexture( const std::string& FileName, const std::string& Key )
{
	// 読み込みが終わっていなかったら null を返す.
	if ( GetInstance()->m_IsLoadEnd == false ) 
		return nullptr;

	// 指定したモデルを取得.
	for( auto& m : GetInstance()->m_FontTextureList[FileName] ){
		if( m.first == Key ) return m.second;
	}

	// まだ作成していない文字のため作成する.
	if( FAILED( CreateTexture( FileName, Key ) ) ) return nullptr;
	return GetFontTexture( FileName, Key );
}

//---------------------------.
// テクスチャの作成.
//---------------------------.
HRESULT CFontResource::CreateTexture( const std::string& FileName, const std::string& Key )
{
	CFontResource* pI = GetInstance();
	
	// 作成する文字を取得.
	std::string f = Key.substr( 0, 1 );
	if ( IsDBCSLeadByte( Key[0] ) == TRUE ) {
		f = Key.substr( 0, 2 );	// 全角文字.
	}
	else {
		f = Key.substr( 0, 1 );	// 半角文字.
	}

	// すでに作成済みか.
	const bool KeyFind = ( pI->m_FontTextureList[FileName].find( f ) != pI->m_FontTextureList[FileName].end() );
	if ( KeyFind ) return E_FAIL;

	// テクスチャの作成.
	std::unique_ptr<CFontCreate> pFontCreate = std::make_unique<CFontCreate>( pI->m_FontDataList[FileName], FileName );
	pFontCreate->CreateFontTexture2D( f.c_str(), &pI->m_FontTextureList[FileName][f] );

	CLog::PushLog( "フォント「" + FileName + "」の「" + Key + "」テクスチャ作成 : 成功");
	return S_OK;
}
