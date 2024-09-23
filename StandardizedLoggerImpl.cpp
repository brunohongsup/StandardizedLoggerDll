#include "pch.h"
#include "StandardizedLoggerImpl.h"

CString GetFormattedTime()
{
	const auto now = std::chrono::system_clock::now();
	const auto duration = now.time_since_epoch();
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - std::chrono::duration_cast<std::chrono::milliseconds>(seconds);
	const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration) - std::chrono::duration_cast<std::chrono::microseconds>(seconds);

	const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	CTime time = CTime::GetCurrentTime();

	CString formattedTime;
	formattedTime.AppendFormat(_T("%02d:%02d:%02d:%03d:%03d"),
							   time.GetHour(),
							   time.GetMinute(),
							   time.GetSecond(),
							   milliseconds.count(),
							   microseconds.count() % 1000
	);

	return formattedTime;
}

void CStandardizedLoggerImpl::PushProcLogMainThreadMainLoopStart(const int nProductCount, const int nMainThreadIdx)
{
	LookUpMainThreadIdx(nMainThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPostTag = GetPostTagString(EPostTag::MainLoopStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("Null,"));
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadString(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), m_strVisionSystemMinorName);
	strLogContents.AppendFormat(_T("%s"), strPostTag);
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx)
{
	LookUpMainThreadIdx(nMainThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPostTag = GetPostTagString(EPostTag::MainLoopEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"),strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadString(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), m_strVisionSystemMinorName);
	strLogContents.AppendFormat(_T("%s"), strPostTag);
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogCameraThreadInterfaceInGrab(const int nProductCount, const CString & strProductId, const int nCameraThreadIdx)
{
	LookUpCameraThreadIdx(nCameraThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	CString strInterfaceTarget = GetInterfaceTargetString(EInterfaceTarget::Camera);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetCameraThreadStringThreadName(nCameraThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), strInterfaceTarget);
	strLogContents.AppendFormat(_T("%s"), GetGrabShortenedSting(nCameraThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncInFromCamreaThread(const int & nProductCount, const int & nMainThreadIdx, const int & nCameraThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetCameraThreadString(nCameraThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogCameraThreadSyncOutToMainThread(const int nProductCount, const CString & strProductId, const int nCameraThreadIdx, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetCameraThreadStringThreadName(nCameraThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

UINT CStandardizedLoggerImpl::SaveLogThreading(LPVOID pParam)
{
	auto* pInstance = (CStandardizedLoggerImpl*)pParam;
	while(pInstance->m_bThreadRunning)
	{
		std::shared_ptr<SLogItem>  pLogItem { nullptr };
		bool bItemExist = pInstance->PopLogItem(pLogItem);
		if(!bItemExist)
		{
			Sleep(5000);
			continue;
		}
		else
		{
			bool bSaveResult = pLogItem->Save();
			if(!bSaveResult)
				pInstance->PushLogItemToQueue(pLogItem);

			Sleep(3);
		}
	}

	return 0;
}

void CStandardizedLoggerImpl::StartSaveStandardLogThread()
{
	m_bThreadRunning = true;
	m_pSaveStandardLogThread = AfxBeginThread(SaveLogThreading, this);
}

void CStandardizedLoggerImpl::StopSaveStandardLogThread()
{
	m_bThreadRunning = false;
}

bool CStandardizedLoggerImpl::init()
{
	bool bRet = true;

	do
	{
		const int nEnumIdx = 0;
		TCHAR path[MAX_PATH];
		m_strExeFileName.Empty();
		if(GetModuleFileNameW(NULL, path, MAX_PATH))
		{
			auto* pFileName = PathFindFileName(path);
			m_strExeFileName.AppendFormat(pFileName);
		}

		else
			m_strExeFileName.AppendFormat(_T("ExeFileNameLookUpError"));

		StartSaveStandardLogThread();
	}
	while(false);

	return bRet;
}

void CStandardizedLoggerImpl::PushLogItemToQueue(const std::shared_ptr<SLogItem>& pItem)
{
	CSingleLock lock(&m_csQueue);
	lock.Lock();
	
	pItem->strLogContent.AppendFormat(_T("\n"));
	m_queueLogItem.push(pItem);
}

bool CStandardizedLoggerImpl::PopLogItem(std::shared_ptr<SLogItem>& pItem)
{
	CSingleLock lock(&m_csQueue);
	lock.Lock();
	if(m_queueLogItem.empty())
	{
		pItem = nullptr;
		return false;
	}

	else
	{
		pItem = m_queueLogItem.front();
		m_queueLogItem.pop();
		return true;
	}
}

void CStandardizedLoggerImpl::PushSystemLogSaveProcLogThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	LookUpSystemThreadIdx();
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetSaveProcLogThreadStringName());
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveProcLogThreadSting());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::SystemLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushSystemLogSaveProcThreadSaveProcessLogThreadStartPreTag(const int nProductCount, const CString & strProductId)
{
	LookUpSystemThreadIdx();
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetSaveProcLogThreadStringName());
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("Sv Log St"));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::SystemLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushSystemLogSaveProcThreadSaveProcessLogThreadEndPreTag(const int nProductCount, const CString & strProductId)
{
	LookUpSystemThreadIdx();
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetSaveProcLogThreadStringName());
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("Sv Log Ed"));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::SystemLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushSystemLogSaveEtcThreadHeartBitOut(const int nProductCount, const CString & strProductId)
{
	LookUpSystemThreadIdx();
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::HeartBeatOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveEtcThreadStringName());
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("Heart Bit Out"));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushAlarmLogCellTrackingInputFailOn(const int nProductCount, const CString & strProductId)
{
	LookUpAlaramThreadIdx();
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetAlaramLogAlarmString());
	strLogContents.AppendFormat(_T("Cell Tracking Input Fail On"));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::AlarmLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushAlarmLogGrabFail(const int nProductCount, const CString & strProductId, const int nCameraIdx)
{
	LookUpAlaramThreadIdx();
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetAlaramLogAlarmString());
	strLogContents.AppendFormat(_T("Camera %d Grab Fail"), nCameraIdx);

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::AlarmLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushResultLog(const int nProductCount, const CString & strModuelId, const CString & strCellId, const CString & strImagePath, const std::vector<CString>& vctEtcLog)
{
	LookUpResultThreadIdx();
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,%s"), strModuelId, strCellId);
	for(const auto& str : vctEtcLog)
		strLogContents.AppendFormat(_T(",%s"),str);

	strLogContents.AppendFormat(_T(",%s"),strImagePath);
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ResultLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCResetOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"),GetResetSignalOnString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCResetOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResetSignalOffString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCResetOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResetSignalOnString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCResetOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResetSignalOffString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}


void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToEncoderResetFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	CString strPostTag = GetPostTagString(EPostTag::FunctionStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::Encoder));
	strLogContents.AppendFormat(_T("%s"), strPostTag);
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToEncoderResetFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	CString strPostTag = GetPostTagString(EPostTag::FunctionStart);

	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResetSignalOffString());
	strLogContents.AppendFormat(_T("%s"), strPostTag);
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncInFrom3DCamreaThread(const int nProductCount, const int & nMainThreadIdx, const int & n3DCamreaIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), Get3DCameraThreadString(n3DCamreaIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}


void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCStepNumber(const int nProductCount, const int nMainThreadIdx, const int nStepNumber, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetWriteStepNumberString(nStepNumber));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLog3DCameraThreadSyncOutToMainThread(const int nProductCount, const int n3DCameraThreadIdx, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), Get3DCameraThreadStringThreadName(n3DCameraThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCVisionReadyOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetVisionReadyString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadCheckStepNumber(const int nProductCount, const int nMainThreadIdx, const CString & strProductId, const int nStepNumber)
{
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s"), GetCheckStepNumber(nStepNumber));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCTriggerOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetTriggerOnString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCTriggerOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s"), GetTriggerOffString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCVisionReadyOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s"), GetTriggerOffString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToTriggerEnable(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetTriggerOnString());
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCFinishOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetVisionFinishSignalOnString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCFinishOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetVisionFinishSignalOffString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCResultAckOn(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResultAckOnString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromPLCResultAckOff(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResultAckOffString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPlcResultOn(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResultSignalOnString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPlcResultOff(const int nProductCount, const int nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResultSignalOffString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceOutToPLCResetAckOn(const int nProductCount, const int nMainThreadId, const CString & strProductId)
{
	
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadId));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::PLC));
	strLogContents.AppendFormat(_T("%s"), GetResultAckOffString());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushAlaramLogOperatorStopButton(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	LookUpAlaramThreadIdx();
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetAlaramLogAlarmString());
	strLogContents.AppendFormat(_T("User_Stop Btn Clicked"));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::AlarmLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncOutToSaveImgThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx, const int nSaveImgThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveImgThreadString(nSaveImgThreadIdx));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncOutToSaveDataThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx, const int nSaveDataThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveDataThreadString(nSaveDataThreadIdx));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncOutToSaveProcLogThread(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveProcLogThreadSting());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveImgThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx)
{
	LookUpSaveImgThreadIdx(nSaveImgThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveImgThreadStringThreadName(nSaveImgThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDateThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpImageProcessThreadIdxAndPushToListFile(const int nImgProcessThreadIdx)
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	if(!PathFileExists(strListFilePath))
		  ClearThreadTables();

	auto findImgProcThread = m_tableImageProcessThreadIdx.find(nImgProcessThreadIdx);
	if(findImgProcThread != std::end(m_tableImageProcessThreadIdx))
		return;

	m_tableImageProcessThreadIdx.emplace(nImgProcessThreadIdx);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(GetImgProcessThreadString(nImgProcessThreadIdx));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::ClearThreadTables()
{
	m_tableCameraThreadIdx.clear();
	m_tableImageProcessThreadIdx.clear();
	m_tableMainThreadIdx.clear();
	m_tableSaveDataThreadIdx.clear();
	m_tableSaveImgThreadIdx.clear();
}

void CStandardizedLoggerImpl::PushResultSystemAlramToListFile(const CString & strFilePath)
{
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogPath.AppendFormat(strFilePath);
	strLogContents.AppendFormat(_T("Result\nSystem\nAlarm"));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpSaveDataThreadIdx(const int nSaveDataThreadIdx)
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	if(!PathFileExists(strListFilePath))
	{
		ClearThreadTables();
	}

	auto findSaveDataThread = m_tableSaveDataThreadIdx.find(nSaveDataThreadIdx);
	if(findSaveDataThread != std::end(m_tableSaveDataThreadIdx))
		return;

	m_tableSaveDataThreadIdx.emplace(nSaveDataThreadIdx);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(GetSaveDataThreadString(nSaveDataThreadIdx));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpSaveImgThreadIdx(const int nSaveImgThreadIdx)
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	if(!PathFileExists(strListFilePath))
	{
		ClearThreadTables();
	}

	auto findSaveImgThread = m_tableSaveImgThreadIdx.find(nSaveImgThreadIdx);
	if(findSaveImgThread != std::end(m_tableSaveImgThreadIdx))
		return;

	m_tableSaveImgThreadIdx.emplace(nSaveImgThreadIdx);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(GetSaveImgThreadString(nSaveImgThreadIdx));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpMainThreadIdx(const int nMainThreadIdx)
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	if(!PathFileExists(strListFilePath))
	{
		ClearThreadTables();
	}

	auto findMainThread = m_tableMainThreadIdx.find(nMainThreadIdx);
	if(findMainThread != std::end(m_tableMainThreadIdx))
		return;

	m_tableMainThreadIdx.emplace(nMainThreadIdx);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(GetMainThreadString(nMainThreadIdx));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpAlaramThreadIdx()
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("ALARM"));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpSystemThreadIdx()
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("SYSTEM"));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpResultThreadIdx()
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("RESULT"));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::LookUpCameraThreadIdx(const int nCameraThreadIdx)
{
	const CString strListFilePath = GetLogFilePath(ESystemName::Minor, ELogFileType::ThreadList);
	if(!PathFileExists(strListFilePath))
	{
		ClearThreadTables();
	}

	auto findCamThread = m_tableCameraThreadIdx.find(nCameraThreadIdx);
	if(findCamThread != std::end(m_tableCameraThreadIdx))
		return;

	m_tableCameraThreadIdx.emplace(nCameraThreadIdx);
	const auto pLogItem = std::make_shared<SListFileLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(GetCameraThreadString(nCameraThreadIdx));
	strLogPath.AppendFormat(strListFilePath);

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveImgThreadSaveImgThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveImgThreadStringThreadName(nSaveImgThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveImgLogContents(nCameraThreadIdx, EImgType::Normal));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveImgThreadSaveImgThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveImgThreadStringThreadName(nSaveImgThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveImgLogContents(nCameraThreadIdx, EImgType::Normal));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveImgThreadSaveSpcImgThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveImgThreadStringThreadName(nSaveImgThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveImgLogContents(nCameraThreadIdx, EImgType::Spc));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveImgThreadSaveSpcImgThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveImgThreadStringThreadName(nSaveImgThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveImgLogContents(nCameraThreadIdx, EImgType::Spc));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDataThreadSyncInFromMainThread(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx, const int nMainThreadIdx)
{
	LookUpSaveDataThreadIdx(nSaveDataThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDataThreadSaveDataThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveDataLogContents(EDataType::Normal));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDataThreadSaveDataThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveDataLogContents(EDataType::Normal));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDataThreadSaveSpcDataThreadStartPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadStart);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveDataLogContents(EDataType::Spc));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogSaveDataThreadSaveSpcDataThreadEndPreTag(const int nProductCount, const CString & strProductId, const int nSaveDataThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::ThreadEnd);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetSaveDataThreadStringThreadName(nSaveDataThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetSaveDataLogContents(EDataType::Spc));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogCellTrackingCallbackEnd(const int nProductCount, const CString & strProductId, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::CellTracker));
	strLogContents.AppendFormat(_T("%s"), GetCellTrackingCallBackEndLogContents());

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromCellTrackerNoPostTag(const int nProductCount, const int nMainThreadIdx)
{
	PushProcLogMainThreadInterfaceInFromCellTracker(nProductCount, EPostTag::None, nMainThreadIdx);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromCellTrackerFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx)
{
	PushProcLogMainThreadInterfaceInFromCellTracker(nProductCount, EPostTag::FunctionStart, nMainThreadIdx);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromCellTrackerFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx)
{
	PushProcLogMainThreadInterfaceInFromCellTracker(nProductCount, EPostTag::FucntionEnd, nMainThreadIdx);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadInterfaceInFromCellTracker(const int nProductCount, const EPostTag postTag, const int nMainThreadIdx)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::InterfaceIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s,"), GetInterfaceTargetString(EInterfaceTarget::CellTracker));
	strLogContents.AppendFormat(_T("Rd Cell ID"));

	if(postTag == EPostTag::FucntionEnd || postTag == EPostTag::FucntionEnd)
		strLogContents.AppendFormat(_T(",%s"),GetPostTagString(postTag));

	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncOutToImgProcThread(const int nProductCount, const int nMainThreadIdx, const int nImgProcThreadIdx, const CString & strProductId)
{
	LookUpMainThreadIdx(nMainThreadIdx);
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncOut);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetImgProcessThreadString(nImgProcThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetMainThreadString(nMainThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadSyncInFromImgProcThread(const int nProductCount, const int & nMainThreadIdx, const int & nImgProcThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	CString strPreTag = GetPreTagString(EPreTag::SyncIn);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("%s,"), strPreTag);
	strLogContents.AppendFormat(_T("%s"), GetImgProcessThreadString(nImgProcThreadIdx));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::PushProcLogMainThreadEndJudgementResult(const int nProductCount, const int & nMainThreadIdx, const CString & strProductId)
{
	CString strLogTime = GetFormattedTime();
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("%s,"), GetMainThreadStringThreadName(nMainThreadIdx));
	strLogContents.AppendFormat(_T("Ed Jdg Ret"));
	strLogPath.AppendFormat(GetLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog));

	PushLogItemToQueue(pLogItem);
}

CString CStandardizedLoggerImpl::GetPostTagString(const EPostTag tag)
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

CString CStandardizedLoggerImpl::GetPreTagString(const EPreTag tag)
{
	static const LPCTSTR lpszPreTags[] =
	{
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

CString CStandardizedLoggerImpl::GetTriggerOnString()
{
	return _T("Sig On Trig");
}

CString CStandardizedLoggerImpl::GetTriggerOffString()
{
	return _T("Sig Off Trig");
}

CString CStandardizedLoggerImpl::GetLogTypeString(const ELogFileType logType)
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

CString CStandardizedLoggerImpl::GetMainThreadString(const int nMainThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("MAIN-THREAD-%d"), nMainThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetCameraThreadString(const int nCamThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("CAM-THREAD-%d"), nCamThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::Get3DCameraThreadString(const int n3DCameraThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("3DCAM-THREAD-%d"), n3DCameraThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::Get3DCameraThreadStringThreadName(const int n3DCameraThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), Get3DCameraThreadString(n3DCameraThreadIdx));

	return strRet;
}

CString CStandardizedLoggerImpl::GetMainThreadStringThreadName(const int nMainThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetMainThreadString(nMainThreadIdx));

	return strRet;

}

CString CStandardizedLoggerImpl::GetCameraThreadStringThreadName(const int nCamThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetCameraThreadString(nCamThreadIdx));

	return strRet;
}

CString CStandardizedLoggerImpl::GetImgProcessThreadStringThreadName(const int nImgProcThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetImgProcessThreadString(nImgProcThreadIdx));

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveImgThreadStringThreadName(const int nSaveImgThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetSaveImgThreadString(nSaveImgThreadIdx));

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveImgLogContents(const int nImgThreadIdx, EImgType eImgTpye)
{
	CString strRet;
	strRet.AppendFormat(_T("Sv Img "));

	if(eImgTpye == EImgType::Spc)
		strRet.AppendFormat(_T("Spc "));

	strRet.AppendFormat(_T("Cam-%d"), nImgThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveDataLogContents(EDataType eDataType)
{
	CString strRet;
	strRet.AppendFormat(_T("Sv Data "));

	if(eDataType == EDataType::Spc)
		strRet.AppendFormat(_T("Spc "));

	return strRet;
}

CString CStandardizedLoggerImpl::GetCellTrackingCallBackEndLogContents()
{
	return _T("Ed Cback");
}

CString CStandardizedLoggerImpl::GetSaveDataThreadStringThreadName(const int nSaveDataThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetSaveDataThreadString(nSaveDataThreadIdx));

	return strRet;
}

CString CStandardizedLoggerImpl::GetAlaramLogAlarmString()
{
	return _T("[ARM]");
}


CString CStandardizedLoggerImpl::GetInterfaceTargetString(const EInterfaceTarget eTarget)
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

CString CStandardizedLoggerImpl::GetGrabShortenedSting(const int nCamIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("Grb-%d"), nCamIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetImgProcessThreadString(const int nImgProcThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("IMAGE-PROCESS-%d"), nImgProcThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveImgThreadString(const int nSaveImgThreadIdx)
{
	CString strRet;
	strRet.AppendFormat(_T("SAVE-IMAGE-%d"), nSaveImgThreadIdx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetVisionFinishSignalOnString()
{
	return _T("Sig On Fnsh");
}

CString CStandardizedLoggerImpl::GetVisionFinishSignalOffString()
{
	return _T("Sig Off Fnsh");
}

CString CStandardizedLoggerImpl::GetResultSignalOnString()
{
	return _T("Sig On Ret");
}

CString CStandardizedLoggerImpl::GetResultSignalOffString()
{
	return _T("Sig Off Ret");
}

CString CStandardizedLoggerImpl::GetResultAckOnString()
{
	return _T("Sig On ack");
}

CString CStandardizedLoggerImpl::GetResultAckOffString()
{
	return _T("Sig Off ack");
}

CString CStandardizedLoggerImpl::GetWriteStepNumberString(const int nStepNumber)
{
	CString strRet;
	strRet.AppendFormat(_T("Sig On Snb-%d"), nStepNumber);

	return strRet;
}

CString CStandardizedLoggerImpl::GetCheckStepNumber(const int nStepNumber)
{
	CString strRet;
	strRet.AppendFormat(_T("Chk Snb-%d"), nStepNumber);

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveDataThreadString(const int nSaveDataThreadidx)
{
	CString strRet;
	strRet.AppendFormat(_T("SAVE-DATA-%d"), nSaveDataThreadidx);

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveProcLogThreadSting()
{
	CString strRet;
	strRet.AppendFormat(_T("SAVE-PROC-LOG"));

	return strRet;
}

bool DoesDriveExist(char driveLetter)
{
	DWORD drives = GetLogicalDrives();
	if(drives & (1 << (driveLetter - 'A')))
		return true;

	else
		return false;
}

CString CStandardizedLoggerImpl::GetSaveProcLogThreadStringName()
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetSaveProcLogThreadSting());

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveEtcThreadStringName()
{
	CString strRet;
	strRet.AppendFormat(_T("[%s]"), GetSaveEtcThread());

	return strRet;
}

CString CStandardizedLoggerImpl::GetSaveEtcThread()
{
	return _T("SAVE ETC");
}

CString CStandardizedLoggerImpl::GetResetSignalOnString()
{
	return _T("Sig On Rst");
}

CString CStandardizedLoggerImpl::GetResetSignalOffString()
{
	return _T("Sig Off Rst");
}

CString CStandardizedLoggerImpl::GetVisionReadyString()
{
	return _T("Sig On Rdy");
}

CString CStandardizedLoggerImpl::GetLogFilePath(const ESystemName eName, const ELogFileType eLogType) const
{
	CString strLogFilePath;
	const char d_Drive = 'D';
	bool d_DriveExist = DoesDriveExist(d_Drive);
	if(d_DriveExist)
		strLogFilePath.AppendFormat(_T("D:\\"));

	else
		strLogFilePath.AppendFormat(_T("C:\\"));

	SYSTEMTIME st;
	GetLocalTime(&st);

	const auto nYear = st.wYear;
	const auto nMonth = st.wMonth;
	const auto nDay = st.wDay;
	const auto nHour = st.wHour;
	strLogFilePath.AppendFormat(_T("LOG_SW\\%04d%02d%02d%02d\\%02d\\"),
								nYear,
								nMonth,
								nDay,
								nHour,
								nHour);

	const CString strSystemName = GetSystemName(eName);
	const CString strLogFileType = GetLogTypeString(eLogType);

	strLogFilePath.AppendFormat(_T("INSPID_%s_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName, nYear, nMonth, nDay, nHour);

	return strLogFilePath;
}

CString CStandardizedLoggerImpl::GetSystemName(const ESystemName eSystem) const
{
	if(eSystem == ESystemName::Major)
		return m_strVisionSystemMajorName;

	else
		return m_strVisionSystemMinorName;
}

void CStandardizedLoggerImpl::Clear()
{
	m_bThreadRunning = false;
	WaitForSingleObject(m_pSaveStandardLogThread->m_hThread, INFINITE);
	CSingleLock lock(&m_csQueue);
	while(!m_queueLogItem.empty())
	{
		auto item = m_queueLogItem.front();
		item->Save();
		m_queueLogItem.pop();
	}
}

CStandardizedLoggerImpl::CStandardizedLoggerImpl()
	: CStandardizedLoggerPrivate()
	, m_strVisionSystemMinorName(CString { _T("MinorNameNotSet") })
	, m_strVisionSystemMajorName(CString { _T("MajorNameNotSet") })
{

}

CStandardizedLoggerImpl::~CStandardizedLoggerImpl()
{

}

void CStandardizedLoggerImpl::SetVisionSystemMajorName(const CString& strMachineName)
{
	m_strVisionSystemMinorName.Empty();
	m_strVisionSystemMinorName.AppendFormat(strMachineName);
}

void CStandardizedLoggerImpl::SetVisionSystemMinorName(const CString& strMinorName)
{
	m_strVisionSystemMinorName.Empty();
	m_strVisionSystemMinorName.AppendFormat(strMinorName);
}

BOOL CreateDirectoryRecursive(const CString& strPath)
{
	TCHAR szPath[MAX_PATH];
	_tcscpy_s(szPath, strPath);

	for(TCHAR* p = szPath; *p; ++p)
	{
		if(*p == _T('\\') || *p == _T('/'))
		{
			*p = _T('\0');
			if(!CreateDirectory(szPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
			{
				return FALSE;
			}
			*p = _T('\\');
		}
	}

	return CreateDirectory(szPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool CStandardizedLoggerImpl::SLogItem::Save()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogContents = this->strLogContent;
	CString strDirPath;
	strDirPath.AppendFormat(strFilePath);
	LPTSTR buffer = strDirPath.GetBuffer();
	PathRemoveFileSpec(buffer);
	strDirPath.ReleaseBuffer();
	if(!PathFileExists(strDirPath)) 
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if(ret == FALSE)
			return false;
	}

	HANDLE hFile = CreateFile(strFilePath,                // name of the write
							  FILE_APPEND_DATA,          // open for writings
							  0,                      // do not share
							  NULL,                   // default security
							  OPEN_ALWAYS,             // create new file only
							  FILE_ATTRIBUTE_NORMAL,  // normal file
							  NULL);                  // no attr. template

	if(hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		return false;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD dwByesWritten {};
	buffer = strLogContent.GetBuffer();
	const DWORD nBufferSize = static_cast<DWORD>(_tcslen(buffer) * sizeof(TCHAR));
	BOOL bWriteResult = WriteFile(hFile, buffer, nBufferSize, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strLogContent.ReleaseBuffer();
	if(TRUE == bWriteResult)
		return true;

	else
		return false;
}

bool CStandardizedLoggerImpl::SListFileLogItem::Save()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogContents = this->strLogContent;
	if(PathFileExists(strFilePath))
	{
		HANDLE hFile = CreateFile(strFilePath,                // name of the write
								  GENERIC_READ,          // open for writings
								  0,                      // do not share
								  NULL,                   // default security
								  OPEN_EXISTING,             // create new file only
								  FILE_ATTRIBUTE_NORMAL,  // normal file
								  NULL);                  // no attr. template

		if(hFile == INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			DWORD dwError = GetLastError();
			return false;
		}

		TCHAR buffer[1024];
		const size_t nBufSize = 1024 * sizeof(TCHAR);
		DWORD bytesRead = 0;
		BOOL result = ReadFile(
			hFile,                  // Handle to the file
			buffer,                 // Buffer to receive data
			nBufSize,         // Number of bytes to read
			&bytesRead,             // Number of bytes read
			NULL                    // Overlapped structure
		);
		CloseHandle(hFile);

		if(result == FALSE)
		{
			return false;
		}

		CString strBufRead {};
		strBufRead.AppendFormat(buffer);
		int nfoundIdx = strBufRead.Find(strLogContents, 0);
		if(nfoundIdx == -1)
			return SLogItem::Save();
		
		else
			return true;
	}

	return SLogItem::Save();
}


