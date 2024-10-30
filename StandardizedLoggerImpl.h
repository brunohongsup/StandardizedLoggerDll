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
#include <cctype>
#include <atlstr.h>
#include <cstdarg>
#include <map>
#include <algorithm>

#pragma comment(lib, "Shlwapi.lib")

#define SYNC_OUT _T("[S-O]")

#define SYNC_IN _T("[S-I]")

#define INTERFACE_IN _T("[I-I]") 

#define INTERFACE_OUT _T("[I-O]")

#define READ_BARCODE _T("Cetr,Rd Barcode ID")

#define FUNCTION_START _T("[FS]")

#define FUNCTION_END _T("[FE]")

#define PLC_STEP_NUM_ON(t1) ([t1]()-> CString { CString str; str.Format(_T("Plc,Sig On Snb %d"), t1); return str; }())

#define VISION_STEP_NUM_CHECK(t1) ([t1]()-> CString { CString str; str.Format(_T("Chk Snb %d"), t1); return str; }())

#define IMAGE_PROC(t1, t2) ([t1,t2]() -> CString { CString str; str.Format(_T("Proc Img %d-%d"), t1, t2); return str; }())

#define CAM_GRAB(t1,t2) ([t1,t2]()-> CString { CString str; str.Format(_T("Cam,Grb %d-%d"), t1, t2); return str; }())

#define SAVE_IMAGE(t1,t2,t3) ([t1,t2,t3]()-> CString { CString str; str.Format(_T("Sv Img %d-%d-%d"), t1, t2, t3); return str; }())

#define SAVE_IMAGE_OVERLAY(t1, t2,t3) ([t1,t2,t3]()-> CString { CString str; str.Format(_T("Sv Img Ovly %d-%d-%d"), t1, t2, t3); return str; }())

#define SAVE_IMAGE_SPC(t1, t2,t3) ([t1,t2,t3]()-> CString { CString str; str.Format(_T("Sv Img Spc %d-%d-%d"), t1, t2, t3); return str; }())

#define SAVE_DATA_INSP(t1) ([t1]()-> CString { CString str; str.Format(_T("Sv Data Insp-%d"), t1); return str; }())

#define SAVE_DATA_SPC(t1) ([t1]()-> CString { CString str; str.Format(_T("Sv Data Spc-%d"), t1); return str; }())

#define SEND_RESULT_TRIPLE(t1, t2, t3) ([t1, t2, t3]()-> CString { CString str; str.Format(_T("Sd Ret %d-%d-%d"), t1, t2,t3); return str; }())

#define SEND_RESULT_DOUBLE(t1, t2) ([t1, t2]()-> CString { CString str; str.Format(_T("Sd Ret %d-%d"), t1, t2); return str; }())

#define END_JUDGE_MENT _T("Ed Jdg Rt")

#define PLC_RESET_SIGNAL_ON _T("Plc,Sig On Rst")

#define PLC_RESET_SIGNAL_OFF _T("Plc,Sig Off Rst")

#define PLC_RESULT_SIGNAL_ON _T("Plc,Sig On Ret")

#define PLC_RESULT_SIGNAL_OFF _T("Plc,Sig Off Ret")

#define VISION_RESET_ACK_ON _T("Vp,Sig On Rst Ack")

#define VISION_RESET_ACK_OFF _T("Vp,Sig Off Rst Ack")

#define VISION_READY_ON _T("Vp,Sig On Rdy")

#define VISION_READY_OFF _T("Vp,Sig Off Rdy")

#define VISION_RESULT_SIGNAL_ON _T("Vp,Sig On Ret")

#define VISION_RESULT_SIGNAL_OFF _T("Vp,Sig Off Ret")

#define PLC_ACK_ON _T("Plc,Sig On Ack")

#define PLC_ACK_OFF _T("Plc,Sig Off Ack")

#define PLC_TRIGGER_OFF _T("Plc,Sig Off Trig")

#define SAVE_LOG _T("Sv Log")

#define PLC_TRIGGER_ON _T("Plc,Sig On Trig")

#define LIGHT_VAL_CHANGE _T("LightCon,Lightval Chg")

#define NULL_ID _T("Null")

#define VISION_COMPLETE_ON _T("Plc,Sig On Cmplt")

#define VISION_COMPLETE_OFF _T("Plc,Sig Off Cmplt")

#define MAIN_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("MAIN-THREAD-%d"), t1); return str; }())

#define SAVE_IMAGE_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("SAVE-IMAGE-%d"), t1); return str; }())

#define SAVE_DATA_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("SAVE-DATA-%d"), t1); return str; }())

#define IMAGE_PROCESS_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("IMAGE-PROCESS-%d"), t1); return str; }())

#define INSPECT_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("INSPECT-%d"), t1); return str; }())

#define CAMERA_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("CAM-THREAD-%d"), t1); return str; }())

#define _3DCAMERA_THREAD(t1) ([t1]()-> CString { CString str; str.Format(_T("3DCAM-THREAD-%d"), t1); return str; }())

#define SAVE_ETC_THREAD ([]()-> CString { CString str; str.Format(_T("SAVE-ETC")); return str; }())


namespace StandardizedLogging
{
	enum class EMacro
	{
		CamInit = 0,
		CamGrab,
		CamGrabEnd,

	};

	static CString GetMacroString(const EMacro eData)
	{
		static const LPCTSTR lpszData[] =
		{
			_T("[S-O],Cam,Init"),
			_T("[I-I],Cam,Grb"),
			_T("[S-O],Cam,Ed"),
		};

		const int nIndesx = static_cast<int>(eData);
		return lpszData[nIndesx];
	};

	enum class EPreTag
	{
		None = 0,
		InterfaceIn,
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
		MainLoopStart,
		MainLoopEnd,
		FunctionStart,
		FunctionEnd,
	};

	enum class EProcessLogThread
	{
		MainThread = 0,
		SaveImgThread,
		SaveDataThread,
		SaveEtcThread,
		ImgProcThread,
		InspectThread,
		CameraThread,
		_3DCameraThread
	};

	enum class ESystemLogThread
	{
		SaveProcessThread = 0,
		SystemThread
	};

	enum class EResultValue
	{
		OK,
		NG
	};

	enum class ELogFileType
	{
		ProcessLog,
		SystemLog,
		AlarmLog,
		ResultLog,
		ThreadList
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

	static CString GetPostTagString(const EPostTag tag)
	{
		static const LPCTSTR lpszPostTags[] =
		{
			_T("No-Post-Tag-This Shouldn't be printed on the log file"),
			_T("[MLS]"),
			_T("[MLE]"),
			_T("[FS]"),
			_T("[FE]")
		};

		const int nTagIdx = static_cast<int>(tag);
		return lpszPostTags[nTagIdx];
	}

	static CString GetPreTagString(const EPreTag tag)
	{
		static const LPCTSTR lpszPreTags[] =
		{
			_T("No-Pre-Tag-This Shouldn't be printed on the log file"),
			_T("[I-I]"),
			_T("[I-O]"),
			_T("[TS]"),
			_T("[TE]"),
			_T("[S-I]"),
			_T("[S-O]"),
			_T("[USER]"),
			_T("[H-I]"),
			_T("[H-O]"),
		};

		const int nTagIdx = static_cast<int>(tag);
		return lpszPreTags[nTagIdx];
	}

	static CString GetInterfaceTargetString(const EInterfaceTarget eTarget)
	{
		static const LPCTSTR lpszInterfaceTarget[] =
		{
			_T("Plc"),
			_T("Spc"),
			_T("Cam"),
			_T("Umac"),
			_T("LightCon"),
			_T("Enc"),
			_T("AutoCorr"),
			_T("Ags"),
			_T("Vp"),
			_T("Brkr"),
			_T("Mavn"),
			_T("Clasfr"),
			_T("TrgBd"),
			_T("Db"),
			_T("Cetr"),
		};

		const int targetIdx = static_cast<int>(eTarget);
		return lpszInterfaceTarget[targetIdx];
	}

	static CString GetLogTypeString(const ELogFileType logType)
	{
		static const LPCTSTR lpszLogType[] =
		{
			_T("PROCESS"),
			_T("SYSTEM"),
			_T("ALARM"),
			_T("RESULT"),
			_T("LIST")
		};

		const int nLogIdx = static_cast<int>(logType);
		return lpszLogType[nLogIdx];
	}

	static CString GetProcessLogThreadName(EProcessLogThread eThread)
	{
		static const LPCTSTR szThreadName[] =
		{
			_T("MAIN-THREAD"),
			_T("SAVE-IMAGE"),
			_T("SAVE-DATA"),
			_T("SAVE-ETC"),
			_T("IMAGE-PROCESS"),
			_T("INSPECT"),
			_T("CAM-THREAD"),
			_T("3DCAM-THREAD")
		};

		const int nThreadIdx = static_cast<int>(eThread);
		const size_t nSize = sizeof(szThreadName);

		return szThreadName[nThreadIdx];
	}

	static CString GetSystemLogThreadName(ESystemLogThread eThread)
	{
		static const LPCTSTR szThreadName[] =
		{
			_T("SAVE-PROC-LOG"),
			_T("SYSTEM"),
		};

		const int nThreadIdx = static_cast<int>(eThread);
		const int nSize = sizeof(szThreadName);

		return szThreadName[nThreadIdx];
	}
}

class CStandardizedLoggerImpl
{
public:
	static std::shared_ptr<CStandardizedLoggerImpl> GetInstance();

	typedef StandardizedLogging::EPostTag EPostTag;

	typedef StandardizedLogging::EPreTag EPreTag;

	typedef StandardizedLogging::EProcessLogThread EProcessLogThread;

	typedef StandardizedLogging::ELogFileType ELogFileType;

	struct SPreTagInfo
	{
		EPreTag eTagFlag;
		enum EInterfaceTarget eTarget;
	};

	enum class ESystemName
	{
		Major = 0,
		Minor
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
		SaveEtc,
		Inspect,
		_3DCamera,
		Result,
		System,
		Alarm,
		SaveProcess,
	};

	struct SLogItem
	{
		CString strFilePath;
		CString strLogContent;

		virtual bool Save();
	};

	struct ILogData
	{
		CString strID = _T("");
		CString strFilePath = _T("");
		CString strTime = _T("");
		CString strLogData;
		int nIndex = 0;

		virtual bool SaveToFile() = 0; 

		bool WriteToFile();

	};

	struct SLogData : ILogData
	{

		CString strThreadName = _T("");


		EPreTag ePreTag = EPreTag::None;
		EPostTag ePostTag = EPostTag::None;

		bool SaveToFile() override;

		bool operator <(const SLogData &sValue) const
		{
			if(nIndex != sValue.nIndex)
			{
				return nIndex < sValue.nIndex;
			}
			else
			{
				return nIndex < sValue.nIndex;
			}
		}
	};

	struct SListFileLogItem : public SLogItem
	{
		bool Save() override;
	};

public:

	void WriteProcessLog(const int nProductCount, const CString & strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString & strLogContent, const EPreTag ePreTag, const EPostTag ePostTag);

	void WriteAlarmLog(const int nProductCount, const CString & strProductId, const CString & strLogContent);

	void WriteResultLog(const int nProductCount, const CString & strModuleId, const CString & strCellId, const StandardizedLogging::EResultValue eResultValue, const CString & strImgPath, const std::vector<CString>& vctLogs);

	void WriteSystemLog(const int nProductCount, const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag);

	void Clear();

	void SetVisionSystemMajorName(const CString& strMajorName) ;

	void SetVisionSystemMinorName(const CString& strMinorName) ;

	CString GetVisionSystemMajorName() const;

	CString GetVisionSystemMinorName() const;

	void WriteMainLoopStart(const int nMainThreadIdx = 1);

	void WriteMainLoopEnd(const CString& strProductId, const int nMainThreadIdx = 1);

	void PushLogItemToQueue(const std::shared_ptr<SLogItem>& pLogItem);

	bool PopLogItem(std::shared_ptr<SLogItem>& pLog);

	void RegisterProductId(const CString& strID);

	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...);

	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, CString strContent, ...);

	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, const EPostTag ePostTag, CString strContent, ...);

private:

	CStandardizedLoggerImpl();

	bool init();

	void StartSaveStandardLogThread();

	void StopSaveStandardLogThread();

	void SaveLogData(int nProductIndex, SLogData& sData);

	static UINT SaveLogThreading(LPVOID pParam);

	CString GetLogFilePath(const CTime& time, const ESystemName eName, const ELogFileType eLogType) const;

	CString GetSystemName(const ESystemName eSystem) const;

	void PushListLog(const CTime& curTime, const CString& strThreadName);

	void PushLogData(const std::shared_ptr<ILogData>& pLogData);

	int GetProductIdxFromTable(const CString& strProductId);

	CCriticalSection m_csLogDeque;

	CCriticalSection m_csTable;

	std::queue<std::shared_ptr<ILogData>> m_queLogData;

	std::map<CString, int> m_tableProducts;

	int m_nProductIndex;

	CCriticalSection m_csQueue;

	std::queue<std::shared_ptr<SLogItem>> m_queueLogItem;

	CCriticalSection m_csThreadRunningLock;

	std::atomic_bool m_bThreadRunning;

	CWinThread* m_pSaveStandardLogThread;

	HANDLE m_hThreadTerminatedEvent;

	CString m_strExeFileName;

	CString m_strVisionSystemMinorName;

	CString m_strVisionSystemMajorName;

	bool m_bCanWriteToDDrive;

	bool m_bIsProductEverRegistered;

	constexpr static size_t MAXIMUM_TABLE_SIZE = 1000; 

	static std::shared_ptr<CStandardizedLoggerImpl> s_instance;

	static CCriticalSection s_lockSection;
};




