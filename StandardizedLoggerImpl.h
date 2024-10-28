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
#include "StandardizedLogger.h"

#pragma comment(lib, "Shlwapi.lib")


class CStandardizedLoggerImpl : public CStandardizedLoggerPrivate
{
public:
	CStandardizedLoggerImpl();

private:

	

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

	struct SLogData
	{
		int nIndex = 0;

		CString strThreadName = _T("");
		CString strFile = _T("");
		
		CString strTime = _T("");
		CString strID = _T("");

		EPreTag ePreTag = EPreTag::None;
		CString strLogData = _T("");
		EPostTag ePostTag= EPostTag::None;


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

	void PushListLog(const CTime& curTime, const CString& strThreadName);

	void WriteProcessLog(const int nProductCount, const CString & strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString & strLogContent, const EPreTag ePreTag, const EPostTag ePostTag) override;

	void WriteAlarmLog(const int nProductCount, const CString & strProductId, const CString & strLogContent) override;

	void WriteResultLog(const int nProductCount, const CString & strModuleId, const CString & strCellId, const StandardizedLogging::EResultValue eResultValue, const CString & strImgPath, const std::vector<CString>& vctLogs) override;

	void WriteSystemLog(const int nProductCount, const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag) override;

	CString GetLogFilePath(const CTime& time, const ESystemName eName, const ELogFileType eLogType) const;

	CString GetSystemName(const ESystemName eSystem) const;

	void Clear() override;

	void SetVisionSystemMajorName(const CString& strMajorName) override;

	void SetVisionSystemMinorName(const CString& strMinorName) override;

	CString GetVisionSystemMajorName() const override;

	CString GetVisionSystemMinorName() const override;

	void WriteMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1) override;

	void WriteMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) override;

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

	HANDLE m_hThreadTerminatedEvent;

	CString m_strExeFileName;

	CString m_strVisionSystemMinorName;

	CString m_strVisionSystemMajorName;

	bool m_bCanWriteToDDrive;









	void SaveLogData(CString strID, int nProductIndex, SLogData sData);

	int m_nProductIndex = 0;
	//key = Thread Name
	std::map<CString, std::vector<SLogData>> m_mapLogData;
	std::map<CString, CString> m_mapBeforeID;
	std::map<CString, int> m_mapProductIndex;

	void StartMainLoop();

	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const CString strContent, ...);
	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID,  const EPreTag ePreTag, const CString strContent, ...);
	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, const EPostTag ePostTag, const CString strContent, ...);

	void WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const StandardizedLogging::EMacro eData);
};




