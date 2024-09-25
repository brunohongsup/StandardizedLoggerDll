#pragma once
#include <memory>
#include <vector>
#include <queue>
#include <Shlwapi.h>
#include <cassert>
#include <afxmt.h>
#include <afxwin.h>

class CStandardizedLoggerImpl;

namespace StandardizedLogging
{
	enum class EPreTag
	{
		None = 0,
		InterfaceIn = 1,
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
		FunctionEnd,
	};

	enum class EProcessLogThread
	{
		MainThread = 0,
		SaveImgThread,
		SaveDataThread,
		ImgProcThread,
		InspectThread,
		CameraThread
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
		_ASSERT(tag != EPostTag::None);
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
		_ASSERT(tag != EPreTag::None);
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
		ASSERT(targetIdx >= static_cast<int>(EInterfaceTarget::PLC) && targetIdx <= static_cast<int>(EInterfaceTarget::CellTracker));


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
		ASSERT(nLogIdx >= static_cast<int>(ELogFileType::ProcessLog) && nLogIdx <= static_cast<int>(ELogFileType::ThreadList));
		return lpszLogType[nLogIdx];
	}


	static CString GetProcessLogThreadName(EProcessLogThread eThread)
	{
		static const LPCTSTR szThreadName[] =
		{
			_T("MAIN-THREAD"),
			_T("SAVE-IMAGE"),
			_T("SAVE-DATA"),
			_T("IMAGE-PROCESS"),
			_T("INSPECT"),
			_T("CAM-THREAD"),
		};

		const size_t nThreadIdx = static_cast<size_t>(eThread);
		const size_t nSize = sizeof(szThreadName);
		_ASSERT(nThreadIdx <= nSize);

		return szThreadName[nThreadIdx];
	}

	static CString GetSystemLogThreadName(ESystemLogThread eThread)
	{
		static const LPCTSTR szThreadName[] =
		{
			_T("SAVE-PROC-LOG"),
			_T("SYSTEM"),
		};

		const size_t nThreadIdx = static_cast<size_t>(eThread);
		const size_t nSize = sizeof(szThreadName);
		_ASSERT(nThreadIdx <= nSize);

		return szThreadName[nThreadIdx];
	}
}

class CStandardizedLoggerPrivate
{
public:


protected:

private:
	friend class CStandardizedLogger;

	virtual void PushMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1) = 0;

	virtual void PushMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) = 0;

	virtual void SetVisionSystemMajorName(const CString& strMachineName) = 0;

	virtual void SetVisionSystemMinorName(const CString& strMachineName) = 0;

	virtual bool init() = 0;

	virtual void Clear() = 0;

	virtual void PushProcessLog(const int nProductCount, const CString& strProductId, const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString& strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag) = 0;

	virtual void PushSystemLog(const int nProductCount, const CString& strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString& strLogContent, const StandardizedLogging::EPreTag = StandardizedLogging::EPreTag::None, const StandardizedLogging::EPostTag ePostTag = StandardizedLogging::EPostTag::None) = 0;

	virtual void PushResultLog(const int nProductCount, const CString& strModuleId, const CString& strCellId, const StandardizedLogging::EResultValue eResultValue, const CString& strImgPath, const std::vector<CString>& vctLogs) = 0;

	virtual void PushAlarmLog(const int nProductCount, const CString& strProductId, const CString& strLogContent) = 0;

protected:

};

class __declspec(dllexport) CStandardizedLogger
{
private:


public:

	typedef StandardizedLogging::EPostTag EPostTag;
	typedef StandardizedLogging::EPreTag EPreTag;
	typedef StandardizedLogging::EProcessLogThread EProcessLogThread;
	typedef StandardizedLogging::ESystemLogThread ESystemLogThread;
	typedef StandardizedLogging::EResultValue EResultValue;

	void Clear()
	{
		m_pImpl->Clear();
	}

	void SetVisionSystemMajorName(const CString& strMajorName)
	{
		m_pImpl->SetVisionSystemMajorName(strMajorName);
	}

	void SetVisionSystemMinorName(const CString& strMinorName)
	{
		m_pImpl->SetVisionSystemMinorName(strMinorName);
	}

	void PushMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushMainLoopStart(nProductCount, nMainThreadIdx);
	}

	void PushMainLoopEnd(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushMainLoopEnd(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushProcessLog(const int nProductCount, const CString& strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString& strLogContent, const EPreTag ePreTag = EPreTag::None, const EPostTag ePostTag = EPostTag::None)
	{
		m_pImpl->PushProcessLog(nProductCount, strProductId, eLogThread, nThreadIdx, strLogContent,ePreTag, ePostTag);
	}

	void PushSystemLog(const int nProductCount, const CString& strProductId, const ESystemLogThread eLogThread, const CString& strLogContent, const EPreTag ePreTag = EPreTag::None, const EPostTag ePostTag = EPostTag::None)
	{
		m_pImpl->PushSystemLog(nProductCount, strProductId, eLogThread, strLogContent, ePreTag, ePostTag);
	}

	void PushAlarmLog(const int nProductCount, const CString& strProductId, const CString& strLogContent)
	{
		m_pImpl->PushAlarmLog(nProductCount, strProductId, strLogContent);
	}

	void PushResultLog(const int nProductCount, const CString& strModuleId, const CString& strCellId, const EResultValue eResultValue, const CString& strImgPath, const std::vector<CString>& vctLogs = {})
	{
		m_pImpl->PushResultLog(nProductCount, strModuleId, strCellId, eResultValue, strImgPath, vctLogs);
	}

	static std::shared_ptr<CStandardizedLogger> GetInstance();

private:

	CStandardizedLogger();

	static std::shared_ptr<CStandardizedLogger> s_instance;

	static CCriticalSection s_lockSection;

	std::unique_ptr<CStandardizedLoggerPrivate> m_pImpl;

	CStandardizedLogger(const CStandardizedLogger&) = delete;

	friend std::shared_ptr<CStandardizedLogger> std::make_shared<CStandardizedLogger>();

};
