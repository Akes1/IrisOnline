#include "StdAfx.h"
#include "UISkinLexer.h"

cUISkinLexer::cUISkinLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	/// 이미지
	BindKeyword( "images", eTOKEN_IMAGES );
	BindKeyword( "skinfiles", eTOKEN_SKINFILES );
	
	/// 속성 파일
	BindKeyword( "propertyfiles", eTOKEN_PROPERTYFILES );

	/// UI에 텍스트 적용
	BindKeyword( "text", eTOKEN_TEXT );	// 공용
		
	/// 사운드 관련
	BindKeyword( "opensound", eTOKEN_OPENSOUND );
	BindKeyword( "closesound", eTOKEN_CLOSESOUND );
	BindKeyword( "downsound", eTOKEN_DOWNSOUND );
	BindKeyword( "upsound", eTOKEN_UPSOUND );
	BindKeyword( "onsound", eTOKEN_ONSOUND );
	BindKeyword( "offsound", eTOKEN_OFFSOUND );

	/// UI 노드
	BindKeyword( "window", eTOKEN_WINDOW );
	BindKeyword( "button", eTOKEN_BUTTON );
	BindKeyword( "checkbox", eTOKEN_CHECKBOX );
	BindKeyword( "label", eTOKEN_LABEL );
	BindKeyword( "editbox", eTOKEN_EDITBOX );
	BindKeyword( "listbox", eTOKEN_LISTBOX );
	BindKeyword( "scrollbar", eTOKEN_SCROLLBAR );
	BindKeyword( "barbutton", eTOKEN_BARBUTTON );
	BindKeyword( "combobox", eTOKEN_COMBOBOX );
	BindKeyword( "gauge", eTOKEN_GAUGE );
	BindKeyword( "gaugetb", eTOKEN_GAUGETB );
	BindKeyword( "doublegauge", eTOKEN_DOUBLEGAUGE );
	BindKeyword( "spinbox", eTOKEN_SPINBOX );
	BindKeyword( "textbox", eTOKEN_TEXTBOX );
	BindKeyword( "messagebox", eTOKEN_MESSAGEBOX );
	BindKeyword( "tabwindow", eTOKEN_TABWINDOW );
	BindKeyword( "icon", eTOKEN_ICON );
	BindKeyword( "pushbutton", eTOKEN_PUSHBUTTON );
	BindKeyword( "tabbutton", eTOKEN_TABBUTTON );
	BindKeyword( "tabsheet", eTOKEN_TABSHEET );
	BindKeyword( "radiobutton", eTOKEN_RADIOBUTTON );
	BindKeyword( "multieditbox", eTOKEN_MULTIEDITBOX );
	BindKeyword( "optiongauge", eTOKEN_OPTIONGAUGE );
	BindKeyword( "folderbox", eTOKEN_LISTFOLDERBOX );
	
	/// UI Property
	BindKeyword( "skinname", eTOKEN_SKINNAME );
	BindKeyword( "parent", eTOKEN_PARENT );
	BindKeyword( "window_prop", eTOKEN_PROPERTY_WINDOW );
	BindKeyword( "label_prop", eTOKEN_PROPERTY_LABEL );
	BindKeyword( "combobox_prop", eTOKEN_PROPERTY_COMBOBOX );
	BindKeyword( "button_prop", eTOKEN_PROPERTY_BUTTON );
	BindKeyword( "editbox_prop", eTOKEN_PROPERTY_EDITBOX );
	BindKeyword( "listbox_prop", eTOKEN_PROPERTY_LISTBOX );
	BindKeyword( "scrollbar_prop", eTOKEN_PROPERTY_SCROLLBAR );
	BindKeyword( "bar_prop", eTOKEN_PROPERTY_BARBUTTON );
	BindKeyword( "spinbox_prop", eTOKEN_PROPERTY_SPINBOX );
	BindKeyword( "gauge_prop", eTOKEN_PROPERTY_GAUGE );
	BindKeyword( "gaugetb_prop", eTOKEN_PROPERTY_GAUGETB );
	BindKeyword( "doublegauge_prop", eTOKEN_PROPERTY_DOUBLEGAUGE );
	BindKeyword( "textbox_prop", eTOKEN_PROPERTY_TEXTBOX );
	BindKeyword( "checkbox_prop", eTOKEN_PROPERTY_CHECKBOX );
	BindKeyword( "tabwindow_prop", eTOKEN_PROPERTY_TABWINDOW );
	BindKeyword( "pushbutton_prop", eTOKEN_PROPERTY_PUSHBUTTON );
	BindKeyword( "numberedit_prop", eTOKEN_PROPERTY_NUMBEREDIT );
	BindKeyword( "tabbutton_prop", eTOKEN_PROPERTY_TABBUTTON );
	BindKeyword( "radiobutton_prop", eTOKEN_PROPERTY_RADIOBUTTON );
	BindKeyword( "tabsheet_prop", eTOKEN_PROPERTY_TABSHEET );
	BindKeyword( "multieditbox_prop", eTOKEN_PROPERTY_MULTIEDITBOX );
	BindKeyword( "optiongauge_prop", eTOKEN_PROPERTY_OPTIONGAUGE );
	BindKeyword( "folderbox_prop", eTOKEN_PROPERTY_LISTFOLDERBOX );


	/// UI 노드 속성
	BindKeyword( "ii", eTOKEN_IMAGEINDEX );
	BindKeyword( "pos", eTOKEN_POS );
	BindKeyword( "size", eTOKEN_SIZE);
	BindKeyword( "texpos", eTOKEN_TEXPOS );
	BindKeyword( "texsize", eTOKEN_TEXSIZE );
	BindKeyword( "hovertip", eTOKEN_HOVERTIPINDEX );
	BindKeyword( "true", eTOKEN_TRUE );
	BindKeyword( "false", eTOKEN_FALSE );
	BindKeyword( "child_ii", eTOKEN_CHILD_IMAGEINDEX );	
	BindKeyword( "child_pos", eTOKEN_CHILD_POS );
	BindKeyword( "child_texpos", eTOKEN_CHILD_TEXPOS );
	BindKeyword( "child_size", eTOKEN_CHILD_SIZE );

	/// 특정한 부모만의 속성
	BindKeyword( "movable", eTOKEN_MOVABLE ); 
	BindKeyword( "holdview", eTOKEN_HOLDVIEW ); 

	BindKeyword( "checked", eTOKEN_CHECKED );
	BindKeyword( "checkpos", eTOKEN_CHECKPOS );
	BindKeyword( "nocheckpos", eTOKEN_NOCHECKPOS );

	BindKeyword( "alpha", eTOKEN_ALPHA );
	BindKeyword( "captionXY", eTOKEN_CAPTIONXY );
	BindKeyword( "captionWH", eTOKEN_CAPTIONWH );
	
	/// 에디트 속성 
	BindKeyword( "editkind", eTOKEN_EDITKIND );
	BindKeyword( "editpos", eTOKEN_EDITPOS );
	BindKeyword( "editwidth", eTOKEN_EDITWIDTH );
	BindKeyword( "editmaxlen", eTOKEN_EDITMAXLEN );

	BindKeyword( "text_align", eTOKEN_TEXT_ALIGN );
	BindKeyword( "left", eTOKEN_LEFT );
	BindKeyword( "right", eTOKEN_RIGHT );
	BindKeyword( "center", eTOKEN_CENTER );


	BindKeyword( "labelskin", eTOKEN_LABELSKIN );
	BindKeyword( "gauge_speed", eTOKEN_GAUGE_SPEED );
	BindKeyword( "show_percent", eTOKEN_SHOW_LABEL_PERCENT );
	BindKeyword( "show_text", eTOKEN_SHOW_TEXT );

	BindKeyword( "gauge_ii", eTOKEN_GAUGEIMAGEINDEX );
	BindKeyword( "gauge_pos", eTOKEN_GAUGEPOS );
	BindKeyword( "gauge_size", eTOKEN_GAUGESIZE );
	BindKeyword( "gauge_texpos", eTOKEN_GAUGETEXPOS );
	BindKeyword( "gauge_width", eTOKEN_GAUGEWIDTH );
	BindKeyword( "gauge_height", eTOKEN_GAUGEHEIGTH );
	BindKeyword( "gauge_start", eTOKEN_GAUGESTART );

	BindKeyword( "gaugedest_ii", eTOKEN_GAUGEIMAGEINDEX_DEST );
	BindKeyword( "gaugedest_pos", eTOKEN_GAUGEPOS_DEST );
	BindKeyword( "gaugedest_size", eTOKEN_GAUGESIZE_DEST );
	BindKeyword( "gaugedest_texpos", eTOKEN_GAUGETEXPOS_DEST );
	BindKeyword( "gaugedest_width", eTOKEN_GAUGEWIDTH_DEST );
	BindKeyword( "gaugedest_height", eTOKEN_GAUGEHEIGTH_DEST );
	BindKeyword( "gaugedest_start", eTOKEN_GAUGESTART_DEST );

	/// 게이지속성
	BindKeyword( "low", eTOKEN_LOW );
	BindKeyword( "middle", eTOKEN_MIDDLE );
	BindKeyword( "high", eTOKEN_HIGH );
	

	BindKeyword( "barbuttonskin", eTOKEN_BARBUTTONSKIN );
	BindKeyword( "upbuttonskin", eTOKEN_UPBUTTONSKIN );
	BindKeyword( "downbuttonskin", eTOKEN_DOWNBUTTONSKIN );

	BindKeyword( "scrollbar_rightpos", eTOKEN_SCROLLBAR_RIGHTPOS );
	BindKeyword( "texteditskin", eTOKEN_EDITSKIN );
	BindKeyword( "scrollskin", eTOKEN_SCROLLSKIN );

	BindKeyword( "listskin", eTOKEN_LISTSKIN );
	BindKeyword( "buttonskin", eTOKEN_BUTTONSKIN );
	BindKeyword( "maxitemnum", eTOKEN_MAXITEMNUM );
	BindKeyword( "imgcolX", eTOKEN_IMGCOLX );
	BindKeyword( "columnX", eTOKEN_COLUMNX );
	BindKeyword( "selectwidth", eTOKEN_SELECTWIDTH );
	BindKeyword( "printpos", eTOKEN_PRINTPOS );
	BindKeyword( "printsize", eTOKEN_PRINTSIZE );

	/// 스핀 박스
	BindKeyword( "leftbuttonskin", eTOKEN_LEFTBUTTON );
	BindKeyword( "rightbuttonskin", eTOKEN_RIGHTBUTTON );
	BindKeyword( "rotation", eTOKEN_ROTATION );
	

	BindKeyword( "row_height", eTOKEN_ROWHEIGHT );

	BindKeyword( "okbuttonskin", eTOKEN_OKBUTTONSKIN);
	BindKeyword( "yesbuttonskin", eTOKEN_YESBUTTONSKIN );
	BindKeyword( "nobuttonskin", eTOKEN_NOBUTTONSKIN);

	/// 버튼 속성
	BindKeyword( "uppos", eTOKEN_UPPOS );
	BindKeyword( "upoverpos", eTOKEN_UPOVERPOS );
	BindKeyword( "downpos", eTOKEN_DOWNPOS );
	BindKeyword( "downoverpos", eTOKEN_DOWNOVERPOS );
	BindKeyword( "enablepos", eTOKEN_ENABLEPOS );
	BindKeyword( "useenable", eTOKEN_USEENABLE );
	BindKeyword( "upsize", eTOKEN_UPSIZE );
	BindKeyword( "downsize", eTOKEN_DOWNSIZE );
	BindKeyword( "upcolor", eTOKEN_UPCOLOR );
	BindKeyword( "downcolor", eTOKEN_DOWNCOLOR );
	
	/// 라디오 버튼의 속성
	BindKeyword( "group", eTOKEN_GROUPNUM );

	/// 아이콘 분할 영역 정보
	BindKeyword( "icon_size", eTOKEN_ICON_SIZE );

	/// buff window
	BindKeyword( "space", eTOKEN_BUFF_SPACE );
	BindKeyword( "row", eTOKEN_BUFF_ROW );
	BindKeyword( "column", eTOKEN_BUFF_COLUMN );
	BindKeyword( "scale", eTOKEN_BUFF_SCALE );
}
