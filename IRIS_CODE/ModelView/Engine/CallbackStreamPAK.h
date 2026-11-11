/* ====================================================================
 * 파    일 :	CallbackStreamPAK.h
 * 목    적 :	리소스를 쓰레드로 로딩하기 위한 I/O인터페이스 (Gamebryo기반 Wrap)
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.12
 * 주의사항 :	쓰레드에서 조심해야 하는 모든 사항!!
 *				만약, 공유데이터를 늘릴시 크리티컬 섹션을 이용해서 
 *				철저하게 막아주세요.
 * =================================================================== */

#pragma once

class cCallbackStreamPACK : public NiStream
{
public:
	cCallbackStreamPACK();
	virtual ~cCallbackStreamPACK();

	/// 게임브리오 연결을 위한 기본 쓰레드 함수 Wrapping
	class cBackgroundLoadProcedureEx : public BackgroundLoadProcedure
	{
	public:
		cBackgroundLoadProcedureEx( NiStream* pStream )
			: BackgroundLoadProcedure( pStream )
		{
		}

		/// 쓰레드 메인 처리
		virtual unsigned int	ThreadProcedure( void* pArg );
	};

	/// 큐에 파일명을 채운다.
	void			AddLoadFile( const char* fileName );

	/// 큐를 완전히 비운다.
	void			ClearQueue();

	/// 쓰레드 처리부
	void			Process();

	/// 임의로 호출 금지
	/// 실제 쓰레드 처리부 (큐를 적용한 확장함수)
	void			BackgroundLoadEx();

	/// 임의로 호출 금지
	/// 단일 객체 로딩 완료시 처리부
	virtual void	BackgroundLoadOnExit();

protected:
	/// 쓰레드 시작 체크 (큐를 적용한 확장함수)
	void			BackgroundLoadBeginEx();

protected:
	/// 공유큐에 대한 크리티컬 섹션
	static NiCriticalSection	mQueueCriticalSection;

	/// 쓰레드 로딩 큐 (로딩될 데이터의 목록을 담는다.
	/// 이 큐가 비워 질때까지 쓰레드를 계속 유지한다.
	NiTPrimitiveQueue<char*>*			mpFileQueue;
};
