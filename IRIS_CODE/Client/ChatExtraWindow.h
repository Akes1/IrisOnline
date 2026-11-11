/* ==========================================================================
* 파    일 : ChatExtraWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-06-13
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ChatWindow.h"

const int CHATEXTRA_GAP = 15;
const int CHATEXTRA_MAXROWNUM = 12;
const int CHATEXTRA_DEFAULT_ROWNUM = 7;

class cChatExtraWindow : public cChatWindow
{
public:
	cChatExtraWindow();
	virtual ~cChatExtraWindow();

	virtual void Open() {}

	void InsertText( eChatMsgKind msgkind, LPCTSTR text );
	 void InsertItemText( sInventory inventory, LPCTSTR name, LPCTSTR msg,  ePLAYER_JOB job, eChatMsgKind kind, void* exdata = NULL );
	void SetPos( int i );

	///
	virtual void SetPress( bool normal, bool party, bool guild, bool shout, bool trade, bool whisper, bool system, bool megaphone );
	virtual bool IsPressNormal();
	virtual bool IsPressParty();
	virtual bool IsPressGuild();
	virtual bool IsPressShout();
	virtual bool IsPressTrade();
	virtual bool IsPressWhisper();
	virtual bool IsPressSystem();
	virtual bool IsPressMegaPhone();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnMouseMove( const cUIPos& pos );
	void UpdateRect();
	void UpdateSkin();

	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnButtonDowned(cUINode* pcaller, unsigned int id);
	
private:
	///
	void UpdateSize( int height );

private:
	unsigned int mButtonH;
};
