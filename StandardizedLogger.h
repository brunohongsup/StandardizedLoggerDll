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
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <tuple>

#pragma comment(lib, "Shlwapi.lib")

#define LIGHT_CONTROLLER_PORT_OPEN_FAIL _T("Light Controller_Port Open Fail")

#define _3D_PROFILER_INITIALIZATION_FAIL _T("3D Profiler_Intialization Fail")

#define SYNC_OUT _T("[S-O]")

#define SYNC_IN _T("[S-I]")

#define INTERFACE_IN _T("[I-I]") 

#define INTERFACE_OUT _T("[I-O]")

#define READ_BARCODE _T("Cetr,Rd Barcode ID")

#define READ_BARCODE_WITH_ID(t1) ([](const CString& strId) { CString str; str.Format(_T("Cetr,Rd Barcode ID %s"), strId); return str; })(t1)

#define FUNCTION_START _T("[FS]")

#define FUNCTION_END _T("[FE]")

#define PLC_STEP_NUM_ON(t1) ([](const int nStepNum) { CString str; str.Format(_T("Plc,Sig On Snb %d"), nStepNum); return str; })(t1)

#define VISION_STEP_NUM_CHECK(t1) ([](const int nStepNum) { CString str; str.Format(_T("Chk Snb %d"), nStepNum); return str; })(t1)

#define IMAGE_PROC_DOUBLE(t1, t2) ([](const int nOp1, const int nOp2) { CString str; str.Format(_T("Proc Img %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define IMAGE_PROC_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Proc Img-%d"), nOp1); return str; })(t1)

#define IMAGE_PROC _T("Proc Img")

#define CAM_GRAB_DOUBLE(t1,t2) ([](const int nCam1, const int nCam2){ CString str; str.Format(_T("Cam,Grb %d-%d"), nCam1, nCam2); return str; })(t1,t2)

#define CAM_GRAB_SINGLE(t1) ([](const int nCam) { CString str; str.Format(_T("Cam,Grb-%d"), nCam); return str; })(t1)

#define SAVE_IMAGE_TRIPLE(t1,t2,t3) ([](const int nOp1, const int nOp2, const int nOp3) { CString str; str.Format(_T("Sv Img %d-%d-%d"), nOp1, nOp2, nOp3); return str; })(t1, t2, t3)

#define SAVE_IMAGE_DOUBLE(t1,t2) ([](const int nOp1, const int nOp2){ CString str; str.Format(_T("Sv Img %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define SAVE_IMAGE_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sv Img %d"), nOp1); return str; })(t1)

#define SAVE_IMAGE _T("Sv Img")

#define SAVE_IMAGE_OVERLAY_TRIPLE(t1, t2,t3) ([](const int nOp1, const int nOp2, const int nOp3){ CString str; str.Format(_T("Sv Img Ovly %d-%d-%d"), nOp1, nOp2, nOp3); return str; })(t1, t2, t3)

#define SAVE_IMAGE_OVERLAY_DOUBLE(t1,t2) ([](const int nOp1, const int nOp2) { CString str; str.Format(_T("Sv Img Ovly %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define SAVE_IMAGE_OVERLAY_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sv Img Ovly-%d"), nOp1); return str; })(t1)

#define SAVE_IMAGE_OVERLAY _T("Sv Img Ovly")

#define SAVE_IMAGE_SPC_TRIPLE(t1, t2,t3) ([](const int nOp1, const int nOp2, const nOp3) { CString str; str.Format(_T("Sv Img Spc %d-%d-%d"), nOp1, nOp2, nOp3); return str; })(t1, t2, t3)

#define SAVE_IMAGE_SPC_DOUBLE(t1, t2) ([](const int nOp1, const int nOp2) { CString str; str.Format(_T("Sv Img Spc %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define SAVE_IMAGE_SPC_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sv Img Spc-%d"), nOp1); return str; })(t1)

#define SAVE_DATA_INSP_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sv Data Insp-%d"), nOp1); return str; })(t1)


#define SAVE_DATA_INSP_DOUBLE(t1, t2) ([](const int nOp1, const int nOp2) { CString str; str.Format(_T("Sv Data Insp %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define SAVE_DATA_INSP _T("Sv Data Insp")

#define PATH_PARSE_ERROR _T("Fail To Parse Img Path")


#define SAVE_DATA_SPC_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sv Data Spc-%d"), nOp1); return str; })(t1)

#define SEND_RESULT_TRIPLE(t1, t2, t3) ([](const int nOp1, const int nOp2, const int nOp3) { CString str; str.Format(_T("Sd Ret %d-%d-%d"), nOp1, nOp2, nOp3); return str; })(t1,t2,t3)

#define SEND_RESULT_DOUBLE(t1, t2) ([](const int nOp1, const int nOp2) { CString str; str.Format(_T("Plc,Sd Ret %d-%d"), nOp1, nOp2); return str; })(t1, t2)

#define SEND_RESULT_SINGLE(t1) ([](const int nOp1) { CString str; str.Format(_T("Sd Ret-%d"), nOp1); return str; })(t1)

#define SEND_RESULT _T("Sd Ret")

#define END_JUDGE_MENT _T("Ed Jdg Rt")

#define PLC_RESET_SIGNAL_ON _T("Plc,Sig On Rst")

#define PLC_RESET_SIGNAL_OFF _T("Plc,Sig Off Rst")

#define PLC_RESULT_SIGNAL_ON _T("Plc,Sig On Ret")

#define PLC_RESULT_SIGNAL_OFF _T("Plc,Sig Off Ret")

#define VISION_RESET_ACK_ON _T("Plc,Sig On Rst Ack")

#define VISION_RESET_ACK_OFF _T("Plc,Sig Off Rst Ack")

#define VISION_READY_ON _T("Plc,Sig On Rdy")

#define VISION_READY_OFF _T("Plc,Sig Off Rdy")

#define VISION_RESULT_SIGNAL_ON _T("Plc,Sig On Ret")

#define VISION_RESULT_SIGNAL_OFF _T("Plc,Sig Off Ret")

#define PLC_ACK_ON _T("Plc,Sig On Ack")

#define PLC_ACK_OFF _T("Plc,Sig Off Ack")

#define PLC_TRIGGER_OFF _T("Plc,Sig Off Trig")

#define SAVE_LOG _T("Sv Log")

#define PLC_TRIGGER_ON _T("Plc,Sig On Trig")

#define LIGHT_VAL_CHANGE _T("LightCon,Lightval Chg")

#define NULL_ID _T("Null")

#define VISION_COMPLETE_ON _T("Plc,Sig On Cmplt")

#define VISION_COMPLETE_OFF _T("Plc,Sig Off Cmplt")

#define MAIN_THREAD(t1) ([](const int t) { CString str; str.Format(_T("MAIN-THREAD-%d"), t); return str; })(t1)

#define SAVE_IMAGE_THREAD(t1) ([](const int t) { CString str; str.Format(_T("SAVE-IMAGE-%d"), t); return str; })(t1)

#define SAVE_DATA_THREAD(t1) ([](const int t) { CString str; str.Format(_T("SAVE-DATA-%d"), t); return str; })(t1)

#define IMAGE_PROCESS_THREAD(t1) ([](const int t) { CString str; str.Format(_T("IMAGE-PROCESS-%d"), t); return str; })(t1)

#define INSPECT_THREAD(t1) ([](const int t) { CString str; str.Format(_T("INSPECT-%d"), t); return str; })(t1)

#define CAMERA_THREAD(t1) ([](const int t) { CString str; str.Format(_T("CAM-THREAD-%d"), t); return str; })(t1)

#define _3DCAMERA_THREAD(t1) ([](const int t) { CString str; str.Format(_T("3DCAM-THREAD-%d"), t); return str; })(t1)

#define SAVE_ETC_THREAD _T("SAVE-ETC")

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
		ListLog,
		RecentProductInfo,
	};

	struct SLogFileType
	{	
		enum class ELogFileType eLogFileType;
		CString ToString();
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
			_T("RecentCellInfo")
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

struct CStringHash
{
	std::size_t operator()(const CString& str) const
	{
		return std::hash<std::wstring>()((LPCTSTR)str);
	}
};

struct CStringEqual
{
	bool operator()(const CString& lhs, const CString& rhs) const
	{
		return lhs.Compare(rhs) == 0;
	}
};

class CStandardizedLogger
{
public:
	static std::shared_ptr<CStandardizedLogger> GetInstance();

	typedef StandardizedLogging::EPostTag EPostTag;

	typedef StandardizedLogging::EPreTag EPreTag;

	typedef StandardizedLogging::EProcessLogThread EProcessLogThread;

	typedef StandardizedLogging::ELogFileType ELogFileType;

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

	enum class EFileExtensionType
	{
		Jpg,
		Bmp,
		Csv,
		Txt,
	};

	struct SFileExtensionType
	{
		enum class EFileExtensionType eType;
		
		CString ToString()
		{
			switch(eType)
			{
			case EFileExtensionType::Bmp:
				return _T(".bmp");
				break;

			case EFileExtensionType::Csv:
				return _T(".csv");
				break;

			case EFileExtensionType::Jpg:
				return _T(".jpg");
				break;

			case EFileExtensionType::Txt:
				return _T(".txt");
				break;

			default:

				break;
			}

			return _T("");
		}

	};

	struct IStandardLogData
	{
		CString strLogData;

		CString strFilePath = _T("");

		virtual bool SaveToFile() = 0;
	};

	struct SRecentProductInfoData : IStandardLogData
	{
		bool SaveToFile() override;
	};

	struct SLogData : IStandardLogData
	{
		int nIndex = 0;

		CString strTime = _T("");

		CString strID = _T("");

		bool SaveToFile() override;

		bool WriteToFile();

		bool operator<(const SLogData& sValue) const;

		SLogData()
		{
			strLogData.GetBuffer(100);
			strLogData.ReleaseBuffer();
		}
	};

	struct SResultLogData : SLogData
	{

	};

	struct SAlarmLogData : SLogData
	{

	};

	struct SSystemLogData : SLogData
	{

	};

	struct SProcessLogData : SLogData
	{
		CString strThreadName = _T("");

		EPreTag ePreTag = EPreTag::None;

		EPostTag ePostTag = EPostTag::None;

		CTime tmLogTime;
	};

	struct SListLogData : SLogData
	{
		SListLogData()
		{
			nIndex = -1;
			strID.Empty();
		}

		bool SaveToFile() override;
	};

	private:

public:

	void WriteAlarmLog(const CString& strProductId, const CString & strLogContent);	

	void WriteResultLog(const CString& strProductId, const int nViewNumber, bool bInspResult);

	void WriteResultLogWithValues(const CString& strProductId, const int nViewNumber, bool bInspResult, const std::vector<CString>& vctValues);

	void WriteSystemLog(const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent);

	void WriteSystemLogPreTag(const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag);

	void WriteSystemLogPostTag(const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPostTag ePostTag);

	void Clear();

	void SetVisionSystemMajorName(const CString& strMajorName);

	void SetVisionSystemMinorName(const CString& strMinorName);

	CString GetVisionSystemMajorName() const;

	CString GetVisionSystemMinorName() const;

	void WriteMainLoopStart(const int nMainThreadIdx = 0);

	void WriteMainLoopStartWithCount(const int nCount, const int nMainThreadIdx = 0);

	void WriteMainLoopEnd(const CString& strProductId, const int nMainThreadIdx = 0);

	void RegisterProductId(const CString& strID);


	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...);

	void WriteProcessLogWithCount(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const int nBarcodeCount, const CString strProductID, CString strContent, ...);

	void WriteProcessLogPreTag(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, CString strContent, ...);

	void WriteProcessLogDoubleTags(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, const EPostTag ePostTag, CString strContent, ...);

	static std::vector<CString> SplitCString(const CString& str, const TCHAR delimiter);

	bool AddProductImgPath(const CString& strProductId, const int nViewNumber, const CString& strImgPath);

	bool TryGetImgPath(const CString& strProductId, const int nViewNumber, CString& strImgPath);

	static CString GetFilePath(const CString& strProductId, const int nCamIdx, const int nImgIdx, bool bIsOk,bool bIsOverlay, EFileExtensionType eFileType);

private:

	CStandardizedLogger();

	bool init();

	bool ClearProductTable();

	static std::vector<std::string> Split(const std::string& str, const char delimiter);

	void WriteProcessLogWithRecentCellInfo(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...);

	void writeResultLogInternal(const CString & strModuleId, const CString& strCellId, const StandardizedLogging::EResultValue eResultValue, const CString & strImgPath, const std::vector<CString>& vctLogs = std::vector<CString> {});

	void writeSystemLogInternal(const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag);

	void startSaveStandardLogThread();

	void stopSaveStandardLogThread();

	void formatProcessLog(const std::shared_ptr<SProcessLogData>& pProcessLogData, EPreTag ePreTag, EPostTag ePostTag);

	void writeProcessLogInternal(const std::shared_ptr<SProcessLogData>& pProcessLogData, EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductId);

	static UINT saveLogThreading(LPVOID pParam);

	CString getLogFilePath(const CTime& time, const ESystemName eName, const ELogFileType eLogType) const;

	CString getSystemName(const ESystemName eSystem) const;

	void pushListLog(const CTime& curTime, const CString& strThreadName);

	void pushLogData(const std::shared_ptr<IStandardLogData>& pLogData);

	int getProductIdxFromTable(const CString& strProductId);

	CCriticalSection m_csLogQueue;

	CCriticalSection m_csProductIdxTableLock;

	CCriticalSection m_csImagePathTableLock;

	std::queue<std::shared_ptr<IStandardLogData>> m_queLogData;

	std::unordered_map<CString, std::pair<int,CTime>, CStringHash, CStringEqual> m_tableProductIdx;

	std::unordered_map<CString, std::pair<CTime,std::vector<std::pair<int, CString>>>, CStringHash, CStringEqual> m_tableImgPath;

	int m_nProductIndex;

	std::atomic_bool m_bThreadRunning;

	CWinThread* m_pSaveStandardLogThread;

	HANDLE m_hThreadTerminatedEvent;

	CString m_strExeFileName;

	CString m_strVisionSystemMinorName;

	CString m_strVisionSystemMajorName;

	bool m_bCanWriteToDDrive;

	CTime m_tmResetTime;

	constexpr static size_t MAXIMUM_TABLE_SIZE = 4000;

	static std::shared_ptr<CStandardizedLogger> s_pInstance;

	static CCriticalSection s_lockSection;
};




