#include "pch.h"
#include "StandardizedLogger.h"

CCriticalSection CStandardizedLogger::s_lockSection;

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::s_pInstance = nullptr;

CString GetFormattedTime(const CTime& curTime);

void CharDeleter(char* ptr);

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::GetInstance()
{
	if(nullptr == s_pInstance)
	{
		CSingleLock lock(&s_lockSection, TRUE);
		if(nullptr == s_pInstance)
		{
			s_pInstance = std::shared_ptr<CStandardizedLogger>(new CStandardizedLogger());
			s_pInstance->init();
		}
	}

	return s_pInstance;
}

void CStandardizedLogger::WriteMainLoopStart(const int nMainThreadIdx)
{
	CString strMainLoopStart;
	strMainLoopStart.AppendFormat(_T("%s,[MLS]"), m_strVisionSystemMinorName);
	{
		CSingleLock lock(&m_csProductIdxTableLock, TRUE);
		CTime curTime = CTime::GetCurrentTime();
		CTimeSpan timeDiff = curTime - m_tmResetTime;
		if(timeDiff.GetTotalHours() >= 24)
		{
			m_tmResetTime = CTime(curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 6, 0, 0);
			m_nProductIndex = 0;
		}

		auto it = m_tableProductIdx.find(NULL_ID);
		it->second.first = ++m_nProductIndex;
	}

	WriteProcessLog(EProcessLogThread::MainThread, nMainThreadIdx, NULL_ID, strMainLoopStart);
}

void CStandardizedLogger::WriteMainLoopStartWithCount(const int nBarcodeCount, const int nMainThreadIdx)
{
	CString strMainLoopStart;
	strMainLoopStart.AppendFormat(_T("%s,[MLS]"), m_strVisionSystemMinorName);
	WriteProcessLogWithCount(EProcessLogThread::MainThread, nMainThreadIdx, nBarcodeCount, NULL_ID, strMainLoopStart);
}

void CStandardizedLogger::WriteMainLoopEnd(const CString& strProductId, const int nMainThreadIdx)
{
	CString strMainLoopEnd;
	strMainLoopEnd.AppendFormat(_T("%s,[MLE]"), m_strVisionSystemMinorName);
	writeProcessLogWithRecentCellInfo(EProcessLogThread::MainThread, nMainThreadIdx, strProductId, strMainLoopEnd);
	bool IsFirstLoopAfterProgramOn = m_bIsFirstLoopAfterProgramOn.load();
	if(IsFirstLoopAfterProgramOn)
	{
		WriteAlarmLog(strProductId, LOOP_FIRST_CELL_AFTER_PROGRAM_ON);
		m_bIsFirstLoopAfterProgramOn.store(false);
	}

	bool IsFirstLoopAfterAlarm = m_bIsFirstLoopAfterAlarm.load();
	if(IsFirstLoopAfterAlarm)
	{
		WriteAlarmLog(strProductId, LOOP_FIRST_CELL_AFTER_LAST_ALARM(_T("LAST ALARM")));
		m_bIsFirstLoopAfterAlarm.store(false);
	}

}

UINT CStandardizedLogger::saveLogThreading(LPVOID pParam)
{
	auto* pInstance = (CStandardizedLogger*)pParam;
	while(pInstance->m_bThreadRunning.load())
	{
		auto& queueLogData = pInstance->m_queLogData;
		bool bIsQueueEmpty = false;
		{
			CSingleLock lock(&pInstance->m_csLogQueue, TRUE);
			bIsQueueEmpty = queueLogData.empty();
		}

		if(bIsQueueEmpty)
		{
			Sleep(3000);
			continue;
		}

		std::shared_ptr<IFileData> pLogData{};
		{
			CSingleLock lock(&pInstance->m_csLogQueue, TRUE);
			pLogData = queueLogData.front();
			queueLogData.pop();
		}

		bool bSaveResult = pLogData->SaveToFile();
		if(!bSaveResult)
		{
			//ToDo Alarm Fail To Save StandardLog
		}
		Sleep(3);
	}

	SetEvent(pInstance->m_hThreadTerminatedEvent);
	return 0;
}

void CStandardizedLogger::stopSaveStandardLogThread()
{
	m_bThreadRunning.store(false);
}

void CStandardizedLogger::formatProcessLog(const std::shared_ptr<SProcessLogData>& pProcessLogData, EPreTag ePreTag, EPostTag ePostTag)
{
	CString strAddLog;
	strAddLog.AppendFormat(_T("L[%s],"), pProcessLogData->strTime);
	strAddLog.AppendFormat(_T("%010d,"), pProcessLogData->nIndex);
	strAddLog.AppendFormat(_T("%s,"), pProcessLogData->strID);
	strAddLog.AppendFormat(_T("[%s],"), pProcessLogData->strThreadName);

	if(ePreTag != StandardizedLogging::EPreTag::None)
	{
		CString strPreTag = GetPreTagString(ePreTag);
		strAddLog.AppendFormat(_T("%s,"), strPreTag);
	}

	strAddLog.AppendFormat(_T("%s"), pProcessLogData->strFileData);
	if(ePostTag != StandardizedLogging::EPostTag::None)
	{
		CString strPostTag = GetPostTagString(ePostTag);
		strAddLog.AppendFormat(_T(",%s"), strPostTag);
	}

	auto& strProcessLogFilePath = pProcessLogData->strFilePath;
	auto& strLogRow = pProcessLogData->strFileData;
	strProcessLogFilePath.AppendFormat(getLogFilePath(pProcessLogData->tmLogTime, ESystemName::Minor, ELogFileType::ProcessLog));
	strLogRow.Empty();
	strLogRow.AppendFormat(strAddLog);
}


bool CStandardizedLogger::init()
{
	bool bRet = true;
	do
	{
		auto findNullId = m_tableProductIdx.find(NULL_ID);
		if(findNullId != std::end(m_tableProductIdx))
		{
			AfxMessageBox(_T("[StandardizedLog] Inititlization Failed - NULL_ID Index is not set to 1"));
			return false;
		}

		m_tableProductIdx.emplace(NULL_ID, std::make_pair(0, CTime::GetCurrentTime()));
		TCHAR szPath[MAX_PATH];
		m_strExeFileName.Empty();
		if(GetModuleFileNameW(NULL, szPath, MAX_PATH))
		{
			auto* pFileName = PathFindFileName(szPath);
			m_strExeFileName.AppendFormat(pFileName);
		}

		else
			m_strExeFileName.AppendFormat(_T("ExeFileNameLookUpError"));

		TCHAR cDDrive = _T('D');
		DWORD dwDrives = GetLogicalDrives();
		bool bDdriveExist = dwDrives & (1 << (cDDrive - _T('A')));
		do
		{
			if(bDdriveExist)
			{
				CString strTestFilePath;
				strTestFilePath.AppendFormat(_T("D:\\Test.txt"));
				HANDLE hDrive = CreateFile(
					strTestFilePath,
					GENERIC_WRITE,
					FILE_SHARE_WRITE,
					NULL,
					OPEN_ALWAYS,
					0,
					NULL
				);

				if(hDrive == INVALID_HANDLE_VALUE)
				{
					DWORD dwError = GetLastError();
					TRACE(_T("The Error Code is %d\n"), dwError);
					break;
				}

				const TCHAR szTestData[] = _T("Test Data");
				const DWORD dwDataSize = (DWORD)(_tcslen(szTestData) * sizeof(TCHAR));
				DWORD dwBytesWritten = 0;
				BOOL bRet = WriteFile(hDrive, szTestData, dwDataSize, &dwBytesWritten, nullptr);
				if(bRet && dwDataSize == dwBytesWritten)
					m_bCanWriteToDDrive = true;

				bRet &= CloseHandle(hDrive);
				bRet &= DeleteFile(strTestFilePath);
				const CString strLogSwDirPath = _T("D:\\LOG_SW");
				if(!PathFileExists(strLogSwDirPath))
				{
					bRet &= CreateDirectory(_T("D:\\LOG_SW"), NULL);
					if(!bRet)
					{
						DWORD dwError = GetLastError();
						CString strError;
						strError.Format(_T("[Standardized Logging] Failed to Create Directory D:\\LOG_SW\\  - Error Code : %d"), dwError);
					}

					if(!bRet)
						return false;

				}

				else
				{
					const CString strRecentProductInfoFile = getLogFilePath(CTime::GetCurrentTime(), CStandardizedLogger::ESystemName::Minor, CStandardizedLogger::ELogFileType::RecentProductInfo);
					if(PathFileExists(strRecentProductInfoFile))
					{
						CFile file;
						BOOL bOpenFile = file.Open(strRecentProductInfoFile, CFile::modeRead);
						if(!bOpenFile)
						{
							DWORD dwError = GetLastError();
							CString strMsg;
							strMsg.Format(_T("[StandardizedLog] Failed To Read RecentProductInfo File. Error Code : %d"), dwError);
							return false;
						}

						unsigned long long ullFileSize = file.GetLength();
						std::string strBufRead(ullFileSize, '\0');
						strBufRead.reserve(150);
						BOOL bReadResult = file.Read(&strBufRead[0], (UINT)ullFileSize);
						file.Close();
						if(!bReadResult)
						{
							DWORD dwError = GetLastError();
							CString strMsg;
							strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);
							//CLogManager::Write(0, strMsg);
							return true;
						}

						std::vector<std::string> vctRecnet = Split(strBufRead, ',');
						if(vctRecnet.size() >= 3)
						{
							CString strRecentProductCount(vctRecnet[1].c_str());
							m_nProductIndex =  _ttoi(strRecentProductCount);
						}
						else
						{
							WriteAlarmLog(NULL_ID, RECENT_PRODUCT_INFO_FILE_FORMAT_WRONG);
						}
						
					}
					
					CTime curTime = CTime::GetCurrentTime();
					m_tmResetTime = CTime(curTime.GetYear(), curTime.GetMonth(), curTime.GetDay(), 6, 0, 0);
				}
			}

			m_bIsFirstLoopAfterProgramOn.store(true);
			m_bIsFirstLoopAfterAlarm.store(false);

		}
		while(false);

		m_hThreadTerminatedEvent = CreateEvent(NULL, TRUE, FALSE, _T("StandardLog Save Thread Terminated"));
		m_bThreadRunning.store(true);
		m_pSaveStandardLogThread = AfxBeginThread(saveLogThreading, this);
		m_vctImageNameElements.reserve(IMAGE_NAME_ELEMENT_MAXIMUM_COUNT);
	}
	while(false);

	return bRet;
}

std::vector<std::string> CStandardizedLogger::Split(const std::string & str, const char delimiter)
{
	std::vector<std::string> vctTokens;
	vctTokens.reserve(10);
	std::stringstream ss(str);
	std::string strToken;
	while(std::getline(ss, strToken, delimiter))
	{
		vctTokens.push_back(strToken);
	}

	return vctTokens;
}

void CStandardizedLogger::writeProcessLogWithRecentCellInfo(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductID);
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.AppendFormatV(strContent, args);
	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = getProductIdxFromTable(strProductID);
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
	
	auto pRecentProductInfo = std::make_shared<SRecentProductInfoData>();
	auto& tmProductTime = pLogData->tmLogTime;
	auto& strRecentProductInfo = pRecentProductInfo->strFileData;
	strRecentProductInfo.AppendFormat(_T("%04d-%02d-%02d"), tmProductTime.GetYear(), tmProductTime.GetMonth(), tmProductTime.GetDay());
	strRecentProductInfo.AppendFormat(_T("%010d,"), pLogData->nIndex);
	strRecentProductInfo.AppendFormat(_T("%s,"), pLogData->strID);
	pRecentProductInfo->strFilePath = getLogFilePath(pLogData->tmLogTime, ESystemName::Minor, StandardizedLogging::ELogFileType::RecentProductInfo);
	pushLogData(pRecentProductInfo);
}

void CStandardizedLogger::writeSystemLogInternal(const CString & strProductId, const StandardizedLogging::ESystemLogThread eSystemLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogData = std::make_shared<SSystemLogData>();
	auto& strLogContents = pLogData->strFileData;
	auto& strLogPath = pLogData->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	const int nProductIdx = getProductIdxFromTable(strProductId);
	pLogData->nIndex = nProductIdx;
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductIdx);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	CString strThreadName;
	strThreadName.Format(_T("%s"), StandardizedLogging::GetSystemLogThreadName(eSystemLogThread));
	strLogContents.AppendFormat(_T("%s,"), strThreadName);
	ASSERT(ePreTag == EPreTag::None || ePostTag == EPostTag::None);
	if(ePreTag != StandardizedLogging::EPreTag::None)
	{
		CString strPreTag = GetPreTagString(ePreTag);
		strLogContents.AppendFormat(_T("%s,"), strPreTag);
	}

	strLogContents.AppendFormat(strLogContent);
	if(ePostTag != StandardizedLogging::EPostTag::None)
	{
		CString strPostTag = GetPostTagString(ePostTag);
		strLogContents.AppendFormat(_T(",%s"), strPostTag);
	}

	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::SystemLog));
	pushLogData(pLogData);
	pushListLog(curTime, _T("SYSTEM"));
}

void CStandardizedLogger::pushListLog(const CTime& curTime, const CString& strThreadName)
{
	const auto pListLogItem = std::make_shared<SListLogData>();
	auto& strListLogContent = pListLogItem->strFileData;
	auto& strListLogPath = pListLogItem->strFilePath;
	strListLogContent.Empty();
	strListLogPath.Empty();
	strListLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ListLog));
	strListLogContent.AppendFormat(strThreadName);
	pushLogData(pListLogItem);
}

void CStandardizedLogger::pushLogData(const std::shared_ptr<IFileData>& pLogData)
{
	CSingleLock lock(&m_csLogQueue, TRUE);
	pLogData->strFileData.AppendFormat(_T("\n"));
	m_queLogData.push(pLogData);
}

int CStandardizedLogger::getProductIdxFromTable(const CString& strProductId)
{
	CSingleLock lock(&m_csProductIdxTableLock, TRUE);
	auto findProduct = m_tableProductIdx.find(strProductId);
	if(findProduct == std::end(m_tableProductIdx))
		return m_nProductIndex;

	else
	{
		const std::pair<int, CTime>& productIdx = findProduct->second;
		return productIdx.first;
	}
}

void CStandardizedLogger::WriteAlarmLog(const CString& strProductId, const CString& strLogContent)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogData = std::make_shared<SAlarmLogData>();
	auto& strLogContents = pLogData->strFileData;
	auto& strLogPath = pLogData->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	const int nProductIdx = getProductIdxFromTable(strProductId);
	if(nProductIdx != -1)
		strLogContents.AppendFormat(_T("%010d,"), nProductIdx);

	else
		strLogContents.AppendFormat(_T("-"));

	pLogData->nIndex = nProductIdx;
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("[ARM],"));
	strLogContents.AppendFormat(strLogContent);

	CString strThreadName;
	strThreadName.AppendFormat(_T("ALARM"));
	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::AlarmLog));
	pushLogData(pLogData);
	pushListLog(curTime, strThreadName);
	m_bIsFirstLoopAfterAlarm.store(true);
}

void CStandardizedLogger::WriteResultLogFlush(const CString & strProductId)
{
	CSingleLock lock(&m_csImagePathTableLock, TRUE);
	for(auto it = std::begin(m_vctImageNameElements); it != std::end(m_vctImageNameElements); )
	{
		auto pImgElement = *it;
		if(strProductId == pImgElement->GetProductId())
		{
			writeResultLogInternal(strProductId, strProductId, pImgElement->GetImgResult(), pImgElement->GetImgPath(), pImgElement->GetValues());

			it = m_vctImageNameElements.erase(it);
		}
		else
			++it;
	}	
}

void CStandardizedLogger::AddImgNameElement(const std::shared_ptr<IResultLogElement>& pImgNameElement)
{
	CSingleLock lock(&m_csImagePathTableLock, TRUE);
	m_vctImageNameElements.emplace_back(pImgNameElement);
}

void CStandardizedLogger::writeResultLogInternal(const CString& strModuleId, const CString& strCellId, const bool bResult, const CString& strImgPath, const std::vector<CString>& vctLogs)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogData = std::make_shared<SResultLogData>();
	auto& strLogContents = pLogData->strFileData;
	auto& strLogPath = pLogData->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	const int nModuleIdx = getProductIdxFromTable(strModuleId);
	strLogContents.AppendFormat(_T("%010d,"), nModuleIdx);
	pLogData->nIndex = nModuleIdx;
	strLogContents.AppendFormat(_T("%s,%s,"), strModuleId, strCellId);
	if(bResult)
		strLogContents.AppendFormat(_T("OK"));

	else 
		strLogContents.AppendFormat(_T("NG"));

	for(const auto& str : vctLogs)
	{
		strLogContents.AppendFormat(_T(",%s"), str);
	}

	strLogContents.AppendFormat(_T(",%s"), strImgPath);
	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ResultLog));
	pushLogData(pLogData);
	pushListLog(curTime, _T("RESULT"));
}

void CStandardizedLogger::WriteSystemLog(const CString & strProductId, const StandardizedLogging::ESystemLogThread eSystemLogThread, const CString & strLogContent)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, EPreTag::None, EPostTag::None);
}

void CStandardizedLogger::WriteSystemLogPreTag(const CString & strProductId, const StandardizedLogging::ESystemLogThread eSystemLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, ePreTag, EPostTag::None);
}

void CStandardizedLogger::WriteSystemLogPostTag(const CString & strProductId, const StandardizedLogging::ESystemLogThread eSystemLogThread, const CString & strLogContent, const StandardizedLogging::EPostTag ePostTag)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, EPreTag::None, ePostTag);
}

CString CStandardizedLogger::getLogFilePath(const CTime& curTime, const ESystemName eName, const ELogFileType eLogType) const
{
	CString strLogFilePath;
	if(m_bCanWriteToDDrive)
		strLogFilePath.AppendFormat(_T("D:\\"));

	else
		strLogFilePath.AppendFormat(_T("C:\\"));

	StandardizedLogging::SLogFileType sLogFileType {};
	sLogFileType.eLogFileType = eLogType;

	if(eLogType != ELogFileType::RecentProductInfo)
	{
		const int nYear = curTime.GetYear();
		const int nMonth = curTime.GetMonth();
		const int nDay = curTime.GetDay();
		const int nHour = curTime.GetHour();
		strLogFilePath.AppendFormat(_T("LOG_SW\\%04d%02d%02d%02d\\%02d\\"),
									nYear,
									nMonth,
									nDay,
									nHour,
									nHour);

		const CString strSystemName = getSystemName(eName);
		const CString strLogFileType = sLogFileType.ToString();

		strLogFilePath.AppendFormat(_T("I_INSPID_%s-VISION_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName, nYear, nMonth, nDay, nHour);

	}

	else
	{
		strLogFilePath.AppendFormat(_T("LOG_SW\\%s.txt"), sLogFileType.ToString());
	}	

	return strLogFilePath;
}

CString CStandardizedLogger::getSystemName(const ESystemName eSystem) const
{
	if(eSystem == ESystemName::Major)
		return m_strVisionSystemMajorName;

	else
		return m_strVisionSystemMinorName;
}

void CStandardizedLogger::Clear()
{
	m_bThreadRunning.store(false);
	const DWORD dwThreeSecondsInMil = 3000;
	WaitForSingleObject(m_hThreadTerminatedEvent, dwThreeSecondsInMil);
	CSingleLock lock(&m_csLogQueue, TRUE);
	while(!m_queLogData.empty())
	{
		auto item = m_queLogData.front();
		item->SaveToFile();
		m_queLogData.pop();
	}

	if(m_hThreadTerminatedEvent != INVALID_HANDLE_VALUE)
		CloseHandle(m_hThreadTerminatedEvent);
}

CStandardizedLogger::CStandardizedLogger()
	: m_strVisionSystemMinorName(CString { _T("MinorNameNotSet") })
	, m_strVisionSystemMajorName(CString { _T("MajorNameNotSet") })
	, m_bCanWriteToDDrive(false)
	, m_nProductIndex(0)
	, m_bIsFirstLoopAfterProgramOn(true)
	, m_bIsFirstLoopAfterAlarm(false)
{

}

void CStandardizedLogger::SetVisionSystemMajorName(const CString& strMajorName)
{
	m_strVisionSystemMajorName.Empty();
	m_strVisionSystemMajorName.AppendFormat(strMajorName);
}

void CStandardizedLogger::SetVisionSystemMinorName(const CString& strMinorName)
{
	m_strVisionSystemMinorName.Empty();
	m_strVisionSystemMinorName.AppendFormat(strMinorName);
}

CString CStandardizedLogger::GetVisionSystemMajorName() const
{
	return m_strVisionSystemMajorName;
}

CString CStandardizedLogger::GetVisionSystemMinorName() const
{
	return m_strVisionSystemMinorName;
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
			CString strDir;
			strDir.AppendFormat(szPath);
			bool bDrive = strDir[strDir.GetLength() - 1] == _T(':');
			if(!bDrive)
			{
				BOOL bCreateResult = CreateDirectory(szPath, NULL);
				if(!bCreateResult)
				{
					DWORD dwError = GetLastError();
					if(dwError != ERROR_ALREADY_EXISTS)
						return false;

				}
			}

			*p = _T('\\');
		}
	}

	return CreateDirectory(szPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

CString GetFormattedTime(const CTime& curTime)
{
	const auto now = std::chrono::system_clock::now();
	const auto duration = now.time_since_epoch();
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) - std::chrono::duration_cast<std::chrono::milliseconds>(seconds);
	const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration) - std::chrono::duration_cast<std::chrono::microseconds>(seconds);
	const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	CString formattedTime;
	formattedTime.AppendFormat(_T("%02d:%02d:%02d:%03d:%03d"),
							   curTime.GetHour(),
							   curTime.GetMinute(),
							   curTime.GetSecond(),
							   milliseconds.count(),
							   microseconds.count() % 1000
	);

	return formattedTime;
}

void CStandardizedLogger::writeProcessLogInternal(const std::shared_ptr<SProcessLogData>& pProcessLogData, EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductId)
{
	if(nullptr != pProcessLogData)
	{
		pProcessLogData->tmLogTime = CTime::GetCurrentTime();
		pProcessLogData->strTime = GetFormattedTime(pProcessLogData->tmLogTime);
		pProcessLogData->strThreadName.Format(_T("%s-%d"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx + 1);
		pProcessLogData->strID = strProductId;
	}
}

void CStandardizedLogger::WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductID);
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.AppendFormatV(strContent, args);
	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = getProductIdxFromTable(strProductID);
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogWithCount(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const int nBarcodeCount, const CString strProductID, CString strContent, ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductID);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	strResult.FormatV(strContent, args);
	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = nBarcodeCount;
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogPreTag(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, CString strContent, ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductID);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch(...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->nIndex = getProductIdxFromTable(strProductID);
	formatProcessLog(pLogData, ePreTag, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogDoubleTags(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, const EPostTag ePostTag, CString strContent, ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductID);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch(...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->ePostTag = ePostTag;
	pLogData->nIndex = getProductIdxFromTable(strProductID);
	formatProcessLog(pLogData, ePreTag, ePostTag);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

std::vector<CString> CStandardizedLogger::SplitCString(const CString& strInput, const TCHAR tcDelimiter)
{
	std::vector<CString> vctResult;
	int nStart = 0;
	int nEnd = 0;
	while((nEnd = strInput.Find(tcDelimiter, nStart)) != -1)
	{
		CString token = strInput.Mid(nStart, nEnd - nStart);
		vctResult.push_back(token);
		nStart = nEnd + 1;
	}

	vctResult.push_back(strInput.Mid(nStart));
	return vctResult;
}


CString CStandardizedLogger::GetFilePath(const CString& strProductId, const int nCamIdx, const int nImgIdx, bool bIsOk, bool bIsOverlay, EFileExtensionType eFileType)
{
	if((bIsOverlay && eFileType == EFileExtensionType::Jpg) || (!bIsOverlay))
	{
		return _T("ERROR");
	}

	CString strFilePath;
	CString strFileName;
	CTime tm = CTime::GetCurrentTime();
	const int nYear = tm.GetYear();
	const int nMonth = tm.GetMonth();
	const int nDay = tm.GetDay();
	strFilePath.AppendFormat(_T("D:\\DAT\\IMAGE\\%04d\\%02d\\%02d"), nYear, nMonth, nDay);
	strFileName.AppendFormat(_T("%s_%d_%d_"), strProductId, nCamIdx + 1, nImgIdx + 1);
	if(bIsOk)
	{
		strFilePath.AppendFormat(_T("\\OK\\%s\\"), strProductId);
		strFileName.AppendFormat(_T("OK"));
	}

	else
	{
		strFilePath.AppendFormat(_T("\\NG\\%s\\"), strProductId);
		strFileName.AppendFormat(_T("NG"));
	}
	
	if(bIsOverlay)
		strFileName.AppendFormat(_T("_Overlay"));

	SFileExtensionType sFileExtension{};
	sFileExtension.eType = eFileType;
	strFileName.AppendFormat(sFileExtension.ToString());
	strFilePath.AppendFormat(strFileName);

	return strFilePath;
}

void CStandardizedLogger::RegisterProductId(const CString& strID)
{
	if(strID.GetLength() < 1)
	{
		WriteAlarmLog(NULL_ID, PLC_SERIAL_NUM_ERROR);
		return;
	}

	if(strID.Compare(NULL_ID) == 0)
		return;

	CSingleLock lock(&m_csProductIdxTableLock, TRUE);
	auto findProduct = m_tableProductIdx.find(strID);
	if(findProduct == std::end(m_tableProductIdx))
	{
		if(m_tableProductIdx.size() > MAXIMUM_TABLE_SIZE)
		{
			std::vector<std::pair<CString, CTime>> vctPathTime {};
			vctPathTime.reserve(MAXIMUM_TABLE_SIZE + 10);
			for(auto it : m_tableProductIdx)
			{
				CString strProductId = it.first;
				CTime tmProduct = it.second.first;
				vctPathTime.emplace_back(std::make_pair(strProductId, tmProduct));
			}

			std::sort(std::begin(vctPathTime), std::end(vctPathTime));
			const int nEraseCount = (int)vctPathTime.size() - 50;
			for(int i = 0; i < nEraseCount; i++)
			{
				const std::pair<CString, CTime>& oldData = vctPathTime[i];
				auto findImgPath = m_tableProductIdx.find(oldData.first);
				if(findImgPath != std::end(m_tableProductIdx))
					m_tableProductIdx.erase(findImgPath);
			}
		}

		m_tableProductIdx.emplace(strID, std::make_pair(m_nProductIndex, CTime::GetCurrentTime()));
	}	
}

bool CStandardizedLogger::SLogData::SaveToFile()
{
	return WriteToFile();
}

bool CStandardizedLogger::SLogData::WriteToFile()
{
	CString strDirPath;
	strDirPath = strFilePath;
	LPTSTR buffer = strDirPath.GetBuffer();
	PathRemoveFileSpec(buffer);
	strDirPath.ReleaseBuffer();
	if(!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if(!ret)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Make Directory Path. Error Code : %d"), dwError);
			return false;
		}
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
		CString strMsg;
		strMsg.Format(_T("[StandardizedLog] Failed To Create File. Error Code : %d"), dwError);
		return false;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD dwByesWritten {};
	buffer = strFileData.GetBuffer();
	const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, &strWrite[0], dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if(TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}

bool CStandardizedLogger::SListLogData::SaveToFile()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogContents = this->strFileData;
	if(PathFileExists(strFilePath))
	{
		CFile file;
		BOOL bOpenFile = file.Open(strFilePath, CFile::modeRead);
		if(!bOpenFile)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);
			//CLogManager::Write(0, _T("[StandardizedLogging] FAIL TO OPEN FILE FOR LIST LOG"));
			return true;
		}

		unsigned long long ullFileSize = file.GetLength();
		std::string strBufRead(ullFileSize, '\0');
		strBufRead.reserve(200);
		BOOL bReadResult = file.Read(&strBufRead[0], (UINT)ullFileSize);
		file.Close();
		if(!bReadResult)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);
			//CLogManager::Write(0, strMsg);
			return true;
		}

		auto buffer = strLogContents.GetBuffer();
		const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
		char* szFindContent = new char[nUtf8Len];
		std::shared_ptr<char> pFindContent(szFindContent, CharDeleter);
		WideCharToMultiByte(CP_UTF8, 0, buffer, -1, pFindContent.get(), nUtf8Len, NULL, NULL);
		const size_t nfoundIdx = strBufRead.find(pFindContent.get());
		if(nfoundIdx == std::string::npos)
		{
			bool bDoesThreadNameContainNumber = false;
			char* pChTmp = pFindContent.get();
			int nThreadNumberPos = 0;
			while(*pChTmp)
			{
				if(std::isdigit(*pChTmp))
				{
					bDoesThreadNameContainNumber = true;
					break;
				}

				++pChTmp;
				++nThreadNumberPos;
			}

			if(!bDoesThreadNameContainNumber)
			{
				return WriteToFile();
			}

			else
			{
				std::string strTmp(pFindContent.get());
				const int nWriteThreadIdx = strTmp[nThreadNumberPos] - '0';
				strTmp.resize(nThreadNumberPos);
				size_t nFindNext = strBufRead.find(strTmp);
				size_t nFindCur = strBufRead.length();
				while(nFindNext != std::string::npos)
				{
					const int nWrittenSameThreadIdx = strBufRead[nFindNext + nThreadNumberPos] - '0';
					nFindCur = nFindNext;
					if(nWrittenSameThreadIdx > nWriteThreadIdx)
					{
						break;
					}

					else
					{
						nFindCur = nFindNext + strTmp.size();
						nFindCur += 2;
						nFindNext = strBufRead.find(strTmp, nFindNext + 1);
					}
				}

				if(nFindCur <= strBufRead.size())
					strBufRead.insert(nFindCur, szFindContent);

				else
					strBufRead.append(szFindContent);

				file.Open(strFilePath, CFile::modeWrite);
				file.Write(strBufRead.c_str(), (UINT)strBufRead.size());
				file.Close();
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return WriteToFile();
	}
}

void CharDeleter(char* ptr)
{
	delete[] ptr;
}

bool CStandardizedLogger::SRecentProductInfoData::SaveToFile()
{
	CString strDirPath;
	strDirPath = strFilePath;
	LPTSTR buffer = strDirPath.GetBuffer();
	PathRemoveFileSpec(buffer);
	strDirPath.ReleaseBuffer();
	if(!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if(!ret)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Make Directory Path. Error Code : %d"), dwError);
			return false;
		}
	}
	
	HANDLE hFile = CreateFile(
		strFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwByesWritten {};
	buffer = strFileData.GetBuffer();
	const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, &strWrite[0], dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if(TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;

}

CString StandardizedLogging::SLogFileType::ToString()
{
	switch(eLogFileType)
	{
	case StandardizedLogging::ELogFileType::ProcessLog:
		return _T("PROCESS");

	case StandardizedLogging::ELogFileType::SystemLog:
		return _T("SYSTEM");

		break;
	case StandardizedLogging::ELogFileType::AlarmLog:
		return _T("ALARM");

		break;
	case StandardizedLogging::ELogFileType::ResultLog:
		return _T("RESULT");

		break;
	case StandardizedLogging::ELogFileType::ListLog:
		return _T("LIST");

		break;
	case StandardizedLogging::ELogFileType::RecentProductInfo:
		return _T("RecentProductInfo");

		break;
	default:
		return _T("LOG FILE TYPE NOT DEFINED");
		break;
	}
}

bool CStandardizedLogger::SFileData::SaveToFile()
{
	CString strDirPath;
	strDirPath = strFilePath;
	LPTSTR buffer = strDirPath.GetBuffer();
	PathRemoveFileSpec(buffer);
	strDirPath.ReleaseBuffer();
	if(!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if(!ret)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Make Directory Path. Error Code : %d"), dwError);
			return false;
		}
	}

	HANDLE hFile = CreateFile(
		strFilePath,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwByesWritten {};
	buffer = strFileData.GetBuffer();
	DWORD dwBytesToWrite = sizeof(TCHAR) * strFileData.GetLength();
	BOOL bWriteResult = WriteFile(hFile, buffer, dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if(TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}
