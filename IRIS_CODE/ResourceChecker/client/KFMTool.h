/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.04.28
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <NiKFMTool.h>

class NiFixedString;
class cEngineFile;

/// KFM 툴
class cKFMTool : public NiKFMTool
{
	class cTransition : public Transition
	{
		friend class cKFMTool;

	public:
		class cBlendPair : public Transition::BlendPair
		{
			friend class cKFMTool;

		public:
			cBlendPair(){}
			cBlendPair( const NiFixedString& startKey, const NiFixedString& targetKey )
				: BlendPair( startKey, targetKey ){}
		};

		class cChainInfo : public Transition::ChainInfo
		{
			friend class cKFMTool;

		public:
			cChainInfo(){}
			cChainInfo( unsigned int sequenceID, float duration )
				: ChainInfo( sequenceID, duration ){}
		};

		cTransition(){}
		cTransition( TransitionType type, float duration )
			: Transition( type, duration ){}
	};

	class cSequence : public Sequence
	{
		friend class cKFMTool;

	public:
		cSequence(){}
//		cSequence( unsigned int sequenceID, const NiFixedString& fileName, int animIndex )
//			: Sequence( sequenceID, fileName, animIndex ){}
	};

	class cSequenceGroup : public SequenceGroup
	{
		friend class cKFMTool;

	public:
		class cSequenceInfo : public SequenceGroup::SequenceInfo
		{
			friend class cKFMTool;

		public:
			cSequenceInfo(){}
			cSequenceInfo( unsigned int sequenceID, int priority, float weight, float easeInTime, float easeOutTime, unsigned int synchronizeSequenceID = SYNC_SEQUENCE_ID_NONE )
				: SequenceInfo( sequenceID, priority, weight, easeInTime, easeOutTime, synchronizeSequenceID ){}
		};

		cSequenceGroup(){}
		cSequenceGroup( unsigned int groupID, const NiFixedString& name )
			: SequenceGroup( groupID, name ){}
	};

public:
	KFM_RC LoadFile( const cString& pathName );
	KFM_RC ReadBinary( cEngineFile& file, unsigned int version );

protected:
	void LoadCString( cEngineFile& file, char*& string );
	void LoadFixedString( cEngineFile& file, NiFixedString& string );
	void LoadCStringAsFixedString( cEngineFile& file, NiFixedString& string );
};
NiSmartPointer(cKFMTool);



//////////////////////////////////////////////////////////////////////////
class NiNodeEx : public NiNode 
{
	NiDeclareRTTI;
	NiDeclareClone(NiNodeEx);
public:
	NiNodeEx();
	virtual ~NiNodeEx();

	virtual void UpdateWorldData();

	NiNode* mNode;
};
NiSmartPointer(NiNodeEx);
