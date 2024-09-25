#pragma once
#include <queue>
#include <memory>
#include <afxmt.h>
#include <cassert>
#include <chrono>
#include <ctime>
#include <windows.h>
#include <afxwin.h>
#include <atomic>
#include <shlwapi.h>
#include <unordered_set>
#include "StandardizedLogger.h"

#pragma comment(lib, "Shlwapi.lib")

class CStandardizedLoggerImpl : public CStandardizedLoggerPrivate
{
public:
	CStandardizedLoggerImpl();
	~CStandardizedLoggerImpl() override;

private:

	struct SLogItem
	{
		CString strFilePath;
		CString strLogContent;

		virtual bool Save();
	};

	struct SListFileLogItem : public SLogItem
	{
		bool Save() override;
	};

	struct SListFileBasicLogItem : public SLogItem
	{
		bool Save() override;
	};

	enum class EPreTag
	{
		InterfaceIn = 0,
		InterfaceOut,
		ThreadStart,
		ThreadEnd,
		SyncIn,
		SyncOut,
		UserIO,
		HeartBeatIn,
		HeartBeatOut,

	};

	enum class EPostTag
	{
		None = 0,
		MainLoopStart = 1,
		MainLoopEnd,
		FunctionStart,
		FucntionEnd,
	};

	enum class EInterfaceTarget
	{
		PLC = 0,
		SPC,
		Camera,
		Umac,
		LightController,
		Encoder,
		AutoCorrector,
		Argos, // Deep Learning Library
		VisionPC,
		Broker,
		Mavin,
		Classifier,
		TriggerBoard,
		Database,
		CellTracker,
	};

	struct SPreTagInfo
	{
		enum EPreTag eTagFlag;
		enum EInterfaceTarget eTarget;
	};

	enum class ESystemName
	{
		Major = 0,
		Minor
	};

	enum class ELogFileType
	{
		ProcessLog,
		SystemLog,
		AlarmLog,
		ResultLog,
		ThreadList
	};

	enum class EImgType
	{
		Normal, 
		Spc
	};

	enum class EDataType
	{
		Normal,
		Spc
	};

	enum class ELogThreadType
	{
		Main,
		Camera,
		ImgProcess,
		SaveData,
		SaveImg,
		Result,
		System,
		Alarm
	};

	static CString GetPostTagString(const EPostTag tag);

	static CString GetPreTagString(const EPreTag tag);

	static CString GetTriggerOnString();

	static CString GetTriggerOffString();

	static CString GetLogTypeString(const ELogFileType logType);

	static CString GetMainThreadString(const int nMainThreadIdx);

	static CString GetCameraThreadString(const int nCamThreadIdx);

	static CString Get3DCameraThreadString(const int n3DCameraThreadIdx);

	static CString Get3DCameraThreadStringThreadName(const int n3DCameraThreadIdx);

	static CString GetMainThreadStringThreadName(const int nMainThreadIdx);

	static CString GetCameraThreadStringThreadName(const int nCamThreadIdx);

	static CString GetImgProcessThreadStringThreadName(const int nImgProcThreadIdx);

	static CString GetSaveImgThreadStringThreadName(const int nSaveImgThreadIdx);

	static CString GetSaveImgLogContents(const int nImgThreadIdx, EImgType eImgTpye);

	static CString GetSaveDataLogContents(EDataType eDataType);

	static CString GetCellTrackingCallBackEndLogContents();

	static CString GetSaveDataThreadStringThreadName(const int nSaveDataThreadIdx);

	static CString GetAlaramLogAlarmString();

	static CString GetInterfaceTargetString(const EInterfaceTarget target);

	static CString GetGrabShortenedSting(const int nCamIdx);

	static CString GetImgProcessThreadString(const int nImgProcThreadIdx);

	static CString GetSaveImgThreadString(const int nSaveImgThreadIdx);

	static CString GetVisionFinishSignalOnString();

	static CString GetVisionFinishSignalOffString();

	static CString GetResultSignalOnString();

	static CString GetResultSignalOffString();

	static CString GetResultAckOnString();

	static CString GetResultAckOffString();

	static CString GetResetAckOnString();
		
	static CString GetWriteStepNumberString(const int nStepNumber);

	static CString GetCheckStepNumber(const int nStepNumber);

	static CString GetSaveDataThreadString(const int nSaveDataThreadidx);

	static CString GetSaveProcLogThreadSting();

	static CString GetSaveProcLogThreadStringName();

	static CString GetSaveEtcThreadStringName();

	static CString GetSaveEtcThread();

	static CString GetResetSignalOnString();

	static CString GetResetSignalOffString();


	static CString GetVisionReadyString();

	CString GetLogFilePath(const ESystemName eName, const ELogFileType eLogType) const;

	CString GetSystemName(const ESystemName eSystem) const;

	void Clear() override;

	void SetVisionSystemMajorName(const CString& strMajorName) override;

	void SetVisionSystemMinorName(const CString& strMinorName) override;

	void PushProcLogMainThreadMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1) override;

	void PushProcLogMainThreadMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) override;

	void PushProcLogCameraThreadInterfaceInGrab(const int nProductCount,const CString& strProductId, const int nCameraThreadIdx = 1) override;

	void PushProcLogMainThreadSyncInFromCamreaThread(const int& nProductCount, const int& nMainThreadIdx, const int& nCameraThreadIdx, const CString& strProductId) override;

	void PushProcLogCameraThreadSyncOutToMainThread(const int nProductCount, const CString& strProductId, const int CameraIdx, const int nMainThreadIdx) override;

	void PushProcLogMainThreadSyncOutToImgProcThread(const int nProductCount, const int nMainThreadIdx, const int nImgProcThreadIdx, const CString & strProductId) override;

	void PushProcLogMainThreadSyncInFromImgProcThread(const int nProductCount, const int & nMainThreadIdx, const int & nImgProcThreadIdx, const CString & strProductId) override;

	void PushProcLogMainThreadEndJudgementResult(const int nProductCount, const int & nMainThreadIdx, const CString & strProductId) override;

	void PushProcLogMainThreadInterfaceInFromCellTrackerNoPostTag(const int nProductCount, const int nMainThreadIdx) override;

	void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx) override;

	void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx) override;

	void PushProcLogMainThreadInterfaceInFromCellTracker(const int nProductCount, const EPostTag postTag, const int nMainThreadIdx);

	void PushProcLogMainThreadSyncOutToSaveImgThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx, const int nSaveImgThreadIdx) override;

	void PushProcLogMainThreadSyncOutToSaveDataThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx, const int nSaveDataThreadIdx) override;

	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	void PushProcLogMainThreadSyncOutToSaveProcLogThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx) override;

	void PushProcLogSaveImgThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx) override;

	void PushProcLogSaveImgThreadSaveImgThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) override;

	void PushProcLogSaveImgThreadSaveImgThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) override;

	void PushProcLogSaveImgThreadSaveSpcImgThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) override;

	void PushProcLogSaveImgThreadSaveSpcImgThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) override;

	void PushProcLogSaveDataThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx, const int nMainThreadIdx) override;

	void PushProcLogSaveDataThreadSaveDataThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx = 1) override;

	void PushProcLogSaveDataThreadSaveDataThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx) override;

	void PushProcLogSaveDataThreadSaveSpcDataThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx) override;

	void PushProcLogSaveDataThreadSaveSpcDataThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx) override;

	void PushProcLogCellTrackingCallbackEnd(const int nProductCout, const CString & strProductId, const int nMainThreadIdx) override;

	void PushProcLogSaveDateThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx, const int nMainThreadIdx) override;

	void LookUpMainThreadIdx(const int nMainThreadIdx);

	void LookUpAlaramThreadIdx();

	void LookUpSystemThreadIdx();

	void LookUpResultThreadIdx();

	void LookUpCameraThreadIdx(const int nCameraThreadIdx);

	void LookUpImageProcessThreadIdxAndPushToListFile(const int nImgProcessThreadIdx);

	void ClearThreadTables();

	void PushResultSystemAlramToListFile(const CString& strFilePath);
	
	void LookUpSaveDataThreadIdx(const int nSaveDataThreadIdx);

	void LookUpSaveImgThreadIdx(const int nSaveImgThreadIdx);
	
	static UINT SaveLogThreading(LPVOID pParam);

	void StartSaveStandardLogThread();

	void StopSaveStandardLogThread();

	bool init();

	void PushLogItemToQueue(const std::shared_ptr<SLogItem>& pLogItem);

	bool PopLogItem(std::shared_ptr<SLogItem>& pLog);

	CCriticalSection m_csQueue;

	std::queue<std::shared_ptr<SLogItem>> m_queueLogItem;

	CCriticalSection m_csThreadRunningLock;

	std::atomic_bool m_bThreadRunning;

	CWinThread* m_pSaveStandardLogThread;
	
	CString m_strExeFileName;

	CString m_strVisionSystemMinorName;

	CString m_strVisionSystemMajorName;

	std::unordered_set<int> m_tableMainThreadIdx;

	std::unordered_set<int> m_tableCameraThreadIdx;

	std::unordered_set<int> m_tableImageProcessThreadIdx;

	std::unordered_set<int> m_tableSaveDataThreadIdx;

	std::unordered_set<int> m_tableSaveImgThreadIdx;	

	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	void PushSystemLogSaveProcLogThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx) override;
	void PushSystemLogSaveProcThreadSaveProcessLogThreadStartPreTag(const int nProductCount, const CString & strProductId) override;
	void PushSystemLogSaveProcThreadSaveProcessLogThreadEndPreTag(const int nProductCount, const CString & strProductId) override;
	void PushSystemLogSaveEtcThreadHeartBitOut(const int nProductCount, const CString & strProductId) override;
	void PushAlarmLogCellTrackingInputFailOn(const int nProductCount, const CString & strProductId) override;

	// CStandardizedLoggerPrivate을(를) 통해 상속됨

	virtual void PushAlarmLogGrabFail(const int nProductCount, const CString & strProductId, const int nCameraIdx) override;

	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushResultLog(const int nProductCount, const CString & strModuelId, const CString & strCellId, const CString & strImagePath, const std::vector<CString>& vctEtcLog) override;

	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceInFromPLCResetOn(const int nProductCount, const int nMainThreadId, const CString & strProductId) override;
	virtual void PushProcLogMainThreadInterfaceInFromPLCResetOff(const int nProductCount, const int nMainThreadId, const CString & strProductId) override;
	virtual void PushProcLogMainThreadInterfaceOutToPLCResetOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;
	virtual void PushProcLogMainThreadInterfaceOutToPLCResetOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;

	

	virtual void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;
	virtual void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;
	virtual void PushProcLogMainThreadSyncInFrom3DCamreaThread(const int nProductCount, const int & nMainThreadIdx, const int & n3DCamreaIdx, const CString & strProductId) override;
	
	// CStandardizedLoggerPrivate을(를) 통해 상속됨

	virtual void PushProcLogMainThreadInterfaceInFromPLCStepNumber(const int nProductCount, const int nMainThreadIdx, const int nStepNumber, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLog3DCameraThreadSyncOutToMainThread(const int nProductCount, const int n3DCameraThreadIdx, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOn(const int nProductCout, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadCheckStepNumber(const int nProductCount, const int nMainThreadIdx, const CString & strProductId, const int nStepNumber) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceInFromPLCTriggerOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;

	virtual void PushProcLogMainThreadInterfaceInFromPLCTriggerOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOff(const int nProductCout, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToTriggerEnable(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToPLCFinishOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;

	virtual void PushProcLogMainThreadInterfaceOutToPLCFinishOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceInFromPLCResultAckOn(const int nProductCount, const CString & strProductId, const int nMainThreadIdx) override;

	virtual void PushProcLogMainThreadInterfaceInFromPLCResultAckOff(const int nProductCount, const CString & strProductId, const int nMainThreadIdx) override;

	virtual void PushProcLogMainThreadInterfaceOutToPlcResultOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToPlcResultOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushProcLogMainThreadInterfaceOutToPLCResetAckOn(const int nProductCount, const int nMainThreadId, const CString & strProductId) override;


	// CStandardizedLoggerPrivate을(를) 통해 상속됨
	virtual void PushAlaramLogOperatorStopButton(const int nProductCount, const CString & strProductId, const int nMainThreadIdx) override;

	// CStandardizedLoggerPrivate을(를) 통해 상속됨
};


