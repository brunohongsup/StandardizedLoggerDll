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
	enum class EMacro
	{
		CamInit = 0,
		CamGrab,
		CamGrabEnd,
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

class CStandardizedLoggerPrivate
{
public:

protected:

private:
	friend class CStandardizedLogger;

	virtual void WriteMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1) = 0;

	virtual void WriteMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) = 0;

	virtual void SetVisionSystemMajorName(const CString& strMajorName) = 0;

	virtual void SetVisionSystemMinorName(const CString& strMinorName) = 0;

	virtual CString GetVisionSystemMajorName() const = 0;

	virtual CString GetVisionSystemMinorName() const = 0;

	virtual bool init() = 0;

	virtual void Clear() = 0;

	virtual void WriteProcessLog(const int nProductCount, const CString& strProductId, const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString& strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag) = 0;

	virtual void WriteSystemLog(const int nProductCount, const CString& strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString& strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag) = 0;

	virtual void WriteResultLog(const int nProductCount, const CString& strModuleId, const CString& strCellId, const StandardizedLogging::EResultValue eResultValue, const CString& strImgPath, const std::vector<CString>& vctLogs) = 0;

	virtual void WriteAlarmLog(const int nProductCount, const CString& strProductId, const CString& strLogContent) = 0;








	virtual void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EMacro eData) = 0;

	virtual void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const CString strContent, ...) = 0;
	virtual void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EPreTag ePreTag, const CString strContent, ...) = 0;
	virtual void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag, const CString strContent, ...) = 0;


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

	CString GetVisionSystemMajorName() const
	{
		return m_pImpl->GetVisionSystemMajorName();
	}

	CString GetVisionSystemMinorName() const
	{
		return m_pImpl->GetVisionSystemMinorName();
	}

	void WriteMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->WriteMainLoopStart(nProductCount, nMainThreadIdx);
	}

	void WriteMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1)
	{
		m_pImpl->WriteMainLoopEnd(nProductCount, strProductId, nMainThreadIdx);
	}

	void WriteProcessLog(const int nProductCount, const CString& strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString& strLogContent, const EPreTag ePreTag = EPreTag::None, const EPostTag ePostTag = EPostTag::None)
	{
		m_pImpl->WriteProcessLog(nProductCount, strProductId, eLogThread, nThreadIdx, strLogContent, ePreTag, ePostTag);
	}

	void WriteSystemLog(const int nProductCount, const CString& strProductId, const ESystemLogThread eLogThread, const CString& strLogContent, const EPreTag ePreTag = EPreTag::None, const EPostTag ePostTag = EPostTag::None)
	{
		m_pImpl->WriteSystemLog(nProductCount, strProductId, eLogThread, strLogContent, ePreTag, ePostTag);
	}

	void WriteAlarmLog(const int nProductCount, const CString& strProductId, const CString& strLogContent)
	{
		m_pImpl->WriteAlarmLog(nProductCount, strProductId, strLogContent);
	}

	void WriteResultLog(const int nProductCount, const CString& strModuleId, const CString& strCellId, const EResultValue eResultValue, const CString& strImgPath, const std::vector<CString>& vctLogs = std::vector<CString>{})
	{
		m_pImpl->WriteResultLog(nProductCount, strModuleId, strCellId, eResultValue, strImgPath, vctLogs);
	}

	static std::shared_ptr<CStandardizedLogger> GetInstance();





	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EMacro eData)
	{
		m_pImpl->WriteProcessLog(eLogThread, nThreadIdx, strProductID, eData);
	}



	template<typename... Args>
	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const CString strContent, Args&&... args)
	{
		m_pImpl->WriteProcessLog(eLogThread, nThreadIdx, strProductID, strContent, std::forward<Args>(args)...);
	}

	template<typename... Args2>
	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EPreTag ePreTag, const CString strContent, Args2&&... args)
	{
		m_pImpl->WriteProcessLog(eLogThread, nThreadIdx, strProductID, ePreTag, strContent, std::forward<Args>(args)...);
	}

	template<typename... Args3>
	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag, const CString strContent, Args3&&... args)
	{
		m_pImpl->WriteProcessLog(eLogThread, nThreadIdx, strProductID, ePreTag, ePostTag, strContent, std::forward<Args>(args)...);
	}




private:

	CStandardizedLogger();

	CStandardizedLogger(const CStandardizedLogger&) = delete;

	CStandardizedLogger& operator=(const CStandardizedLogger&) = delete;

	static std::shared_ptr<CStandardizedLogger> s_instance;

	static CCriticalSection s_lockSection;

	std::unique_ptr<CStandardizedLoggerPrivate> m_pImpl;
};


