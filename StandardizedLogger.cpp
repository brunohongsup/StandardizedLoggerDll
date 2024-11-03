#include "pch.h"
#include "StandardizedLogger.h"

CCriticalSection CStandardizedLogger::s_lockSection;

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::s_instance = nullptr;

CString GetFormattedTime(const CTime& curTime);

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::GetInstance()
{
	if(s_instance == nullptr)
	{
		CSingleLock lock(&s_lockSection, TRUE);

		if(nullptr == s_instance)
		{
			s_instance = std::shared_ptr<CStandardizedLogger>(new CStandardizedLogger());
			s_instance->init();
		}
	}

	return s_instance;
}

void CStandardizedLogger::WriteMainLoopStart(const int nMainThreadIdx)
{
	CString strMainLoopStart;
	strMainLoopStart.AppendFormat(m_strVisionSystemMinorName);
	strMainLoopStart.AppendFormat(_T(",[MLS]"));
	{
		CSingleLock lock(&m_csTable, TRUE);
		auto it = m_tableProducts.find(NULL_ID);
		it->second = ++m_nProductIndex;
	}

	WriteProcessLog(EProcessLogThread::MainThread, nMainThreadIdx, NULL_ID, strMainLoopStart);
}

void CStandardizedLogger::WriteMainLoopEnd(const CString& strProductId, const int nMainThreadIdx)
{
	CString strMainLoopEnd;
	strMainLoopEnd.AppendFormat(m_strVisionSystemMinorName);
	strMainLoopEnd.AppendFormat(_T(",[MLE]"));
	WriteProcessLog(EProcessLogThread::MainThread, nMainThreadIdx, strProductId, strMainLoopEnd);
}

UINT CStandardizedLogger::saveLogThreading(LPVOID pParam)
{
	auto* pInstance = (CStandardizedLogger*)pParam;
	while(pInstance->m_bThreadRunning)
	{
		auto& queueLogData = pInstance->m_queLogData;
		bool bIsQueueEmpty = false;
		{
			CSingleLock lock(&pInstance->m_csLogDeque, TRUE);
			bIsQueueEmpty = queueLogData.empty();
		}

		if(bIsQueueEmpty)
		{
			Sleep(2000);
			continue;
		}

		CString strNextID;
		CString strID;
		std::shared_ptr<ILogData> pLogData {};
		{
			CSingleLock lock(&pInstance->m_csLogDeque, TRUE);
			pLogData = queueLogData.front();
			queueLogData.pop();
		}

		bool bSaveResult = false;
		do
		{
			bSaveResult = pLogData->SaveToFile();
			if(!bSaveResult)
				Sleep(1);

		}
		while(!bSaveResult);

		Sleep(3);
	}

	SetEvent(pInstance->m_hThreadTerminatedEvent);
	return 0;
}

void CStandardizedLogger::startSaveStandardLogThread()
{
	m_bThreadRunning = true;
	m_hThreadTerminatedEvent = CreateEvent(NULL, TRUE, FALSE, _T("StandardLog Save Thread Terminated"));
	m_pSaveStandardLogThread = AfxBeginThread(saveLogThreading, this);
}

void CStandardizedLogger::stopSaveStandardLogThread()
{
	m_bThreadRunning = false;
}

bool CStandardizedLogger::init()
{
	bool bRet = true;
	do
	{
		auto findNullId = m_tableProducts.find(NULL_ID);
		if(findNullId != std::end(m_tableProducts))
		{
			_ASSERT(false);
			AfxMessageBox(_T("[StandardizedLog] Inititlization Failed - NULL_ID Index is not set to 1"));
			return false;
		}

		m_tableProducts.emplace(NULL_ID, 0);

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

			}

		}
		while(false);

		startSaveStandardLogThread();
	}
	while(false);

	return bRet;
}

void CStandardizedLogger::PushLogItemToQueue(const std::shared_ptr<SLogItem>& pItem)
{
	CSingleLock lock(&m_csQueue, TRUE);
	pItem->strLogContent.AppendFormat(_T("\n"));
	m_queueLogItem.push(pItem);
}

bool CStandardizedLogger::PopLogItem(std::shared_ptr<SLogItem>& pItem)
{
	CSingleLock lock(&m_csQueue, TRUE);
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

void CStandardizedLogger::pushListLog(const CTime & curTime, const CString & strThreadName)
{
	const auto pListLogItem = std::make_shared<SListFileLogItem>();
	auto& strListLogContent = pListLogItem->strLogContent;
	auto& strListLogPath = pListLogItem->strFilePath;
	strListLogContent.Empty();
	strListLogPath.Empty();
	strListLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ThreadList));
	strListLogContent.AppendFormat(strThreadName);
	PushLogItemToQueue(pListLogItem);
}

void CStandardizedLogger::pushLogData(const std::shared_ptr<ILogData>& pLogData)
{
	CSingleLock lock(&m_csLogDeque, TRUE);
	auto& queue = m_queLogData;
	queue.push(pLogData);
}

int CStandardizedLogger::getProductIdxFromTable(const CString & strProductId)
{
	CSingleLock lock(&m_csTable, TRUE);
	auto findProduct = m_tableProducts.find(strProductId);
	if(findProduct == std::end(m_tableProducts))
		return -1;

	else
		return findProduct->second;
}

void CStandardizedLogger::WriteProcessLog(const int nProductCount, const CString & strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString & strLogContent, const EPreTag ePreTag, const EPostTag ePostTag)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);

	CString strThreadName;
	strThreadName.AppendFormat(_T("%s-%d"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx);
	strLogContents.AppendFormat(_T("[%s],"), strThreadName);

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

	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
	pushListLog(curTime, strThreadName);
}

void CStandardizedLogger::WriteAlarmLog(const int nProductCount, const CString & strProductId, const CString & strLogContent)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("[ARM],"));
	strLogContents.AppendFormat(strLogContent);

	CString strThreadName;
	strThreadName.AppendFormat(_T("ALARM"));
	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::AlarmLog));
	PushLogItemToQueue(pLogItem);
	pushListLog(curTime, strThreadName);
}

void CStandardizedLogger::WriteResultLog(const int nProductCount, const CString & strModuleId, const CString & strCellId, const StandardizedLogging::EResultValue eResultValue, const CString & strImgPath, const std::vector<CString>& vctLogs)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,%s,"), strModuleId, strCellId);
	if(eResultValue == StandardizedLogging::EResultValue::OK)
		strLogContents.AppendFormat(_T("OK"));

	else if(eResultValue == StandardizedLogging::EResultValue::NG)
		strLogContents.AppendFormat(_T("NG"));

	else
		strLogContents.AppendFormat(_T("Not Set"));

	for(const auto& str : vctLogs)
	{
		strLogContents.AppendFormat(_T(",%s"), str);
	}

	strLogContents.AppendFormat(_T(",%s"), strImgPath);
	strLogPath.AppendFormat(getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ResultLog));
	PushLogItemToQueue(pLogItem);
	pushListLog(curTime, _T("RESULT"));
}

void CStandardizedLogger::WriteSystemLog(const int nProductCount, const CString & strProductId, const StandardizedLogging::ESystemLogThread eSystemLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag)
{
	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	const auto pLogItem = std::make_shared<SLogItem>();
	auto& strLogContents = pLogItem->strLogContent;
	auto& strLogPath = pLogItem->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	strLogContents.AppendFormat(_T("%010d,"), nProductCount);
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	CString strThreadName;
	strThreadName.Format(_T("%s"), StandardizedLogging::GetSystemLogThreadName(eSystemLogThread));
	strLogContents.AppendFormat(_T("%s,"), strThreadName);

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
	PushLogItemToQueue(pLogItem);
	pushListLog(curTime, _T("SYSTEM"));
}

CString CStandardizedLogger::getLogFilePath(const CTime& curTime, const ESystemName eName, const ELogFileType eLogType) const
{
	CString strLogFilePath;
	if(m_bCanWriteToDDrive)
		strLogFilePath.AppendFormat(_T("D:\\"));

	else
		strLogFilePath.AppendFormat(_T("C:\\"));

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
	const CString strLogFileType = GetLogTypeString(eLogType);

	strLogFilePath.AppendFormat(_T("I_INSPID_%s-VISION_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName, nYear, nMonth, nDay, nHour);

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
	m_bThreadRunning = false;
	const DWORD dwThreeSecondsInMil = 3000;
	WaitForSingleObject(m_hThreadTerminatedEvent, dwThreeSecondsInMil);
	CSingleLock lock(&m_csQueue, TRUE);
	while(!m_queLogData.empty())
	{
		auto item = m_queLogData.front();
		item->SaveToFile();
		m_queueLogItem.pop();
	}

	if(m_hThreadTerminatedEvent != INVALID_HANDLE_VALUE)
	{
		BOOL bRet = CloseHandle(m_hThreadTerminatedEvent);
		if(!bRet)
		{
			_ASSERT(false);
		}
	}
}

CStandardizedLogger::CStandardizedLogger()
	: m_strVisionSystemMinorName(CString { _T("MinorNameNotSet") })
	, m_strVisionSystemMajorName(CString { _T("MajorNameNotSet") })
	, m_bCanWriteToDDrive(false)
	, m_bIsProductEverRegistered(false)
	, m_nProductIndex(0)
{

}

void CStandardizedLogger::SetVisionSystemMajorName(const CString& strMachineName)
{
	m_strVisionSystemMinorName.Empty();
	m_strVisionSystemMinorName.AppendFormat(strMachineName);
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
					{
						return false;
					}
				}


			}

			*p = _T('\\');
		}
	}

	return CreateDirectory(szPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool CStandardizedLogger::SLogItem::Save()
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
		if(!ret)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[S"), dwError);
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
		CString strMessage;
		strMessage.Format(_T("[StandardizedLog] Failed To Open File. Error Code : %d"), dwError);

		return false;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD dwByesWritten {};
	buffer = strLogContent.GetBuffer();
	int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, &strWrite[0], dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strLogContent.ReleaseBuffer();

	if(TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}

bool CStandardizedLogger::SListFileLogItem::Save()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogContents = this->strLogContent;
	if(PathFileExists(strFilePath))
	{
		CFile file;
		BOOL bOpenFile = file.Open(strFilePath, CFile::modeRead);
		if(!bOpenFile)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);

			return false;
		}

		unsigned long long ullFileSize = file.GetLength();
		std::string strBufRead(ullFileSize, '\0');
		BOOL bReadResult = file.Read(&strBufRead[0], (UINT)ullFileSize);
		file.Close();
		if(!bReadResult)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);

			return false;
		}

		auto buffer = strLogContents.GetBuffer();
		int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
		char* szFindContent = new char[nUtf8Len];
		WideCharToMultiByte(CP_UTF8, 0, buffer, -1, szFindContent, nUtf8Len, NULL, NULL);
		size_t nfoundIdx = strBufRead.find(szFindContent);
		if(nfoundIdx == std::string::npos)
		{
			bool bContainsNumber = false;
			char* tmp = szFindContent;
			int nNumberIdx = 0;
			while(*tmp)
			{
				if(std::isdigit(*tmp))
				{
					bContainsNumber = true;
					break;
				}

				++tmp;
				++nNumberIdx;
			}

			if(!bContainsNumber)
			{
				delete[] szFindContent;
				return SLogItem::Save();
			}

			else
			{
				std::string strTmp(szFindContent);
				const int nWriteThreadIdx = szFindContent[nNumberIdx] - '0';
				strTmp.resize(nNumberIdx);
				size_t nFindNext = strBufRead.find(strTmp);
				size_t nFindCur = strBufRead.length();
				while(nFindNext != std::string::npos)
				{
					const int nWrittenSameThreadIdx = strBufRead[nFindNext + nNumberIdx] - '0';
					nFindCur = nFindNext;
					_ASSERT(nWrittenSameThreadIdx != nWriteThreadIdx);
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

				strBufRead.insert(nFindCur, szFindContent);
				file.Open(strFilePath, CFile::modeWrite);
				file.Write(strBufRead.c_str(), (UINT)strBufRead.size());
				file.Close();
				delete[] szFindContent;

				return true;
			}
		}

		else
		{
			delete[] szFindContent;
			return true;
		}
	}
	else
	{
		return SLogItem::Save();
	}

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

void CStandardizedLogger::WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, CString strContent, ...)
{
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.AppendFormatV(strContent, args);
	va_end(args);

	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	auto pLogData = std::make_shared<SLogData>();
	pLogData->strThreadName.Format(_T("%s-%d"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx);
	pLogData->strTime = strLogTime;
	pLogData->strID = strProductID;
	pLogData->strLogData = strResult;
	pLogData->nIndex = getProductIdxFromTable(strProductID);

	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, CString strContent, ...)
{
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


	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	auto pLogData = std::make_shared<SLogData>();
	pLogData->strThreadName.Format(_T("%s-%d"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx);
	pLogData->strTime = strLogTime;
	pLogData->strID = strProductID;
	pLogData->strLogData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->nIndex = getProductIdxFromTable(strProductID);

	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx, const CString strProductID, const EPreTag ePreTag, const EPostTag ePostTag, CString strContent, ...)
{
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.FormatV(strContent, args);
	va_end(args);

	CTime curTime = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(curTime);
	auto pLogData = std::make_shared<SLogData>();
	pLogData->strThreadName.Format(_T("%s-%d"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx);
	pLogData->strTime = strLogTime;
	pLogData->strID = strProductID;
	pLogData->strLogData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->ePostTag = ePostTag;
	pLogData->nIndex = getProductIdxFromTable(strProductID);

	pushLogData(pLogData);
}

void CStandardizedLogger::RegisterProductId(const CString& strID)
{
	if(strID.GetLength() < 1)
		return;

	if(!m_bIsProductEverRegistered)
		m_bIsProductEverRegistered = true;

	CSingleLock lock(&m_csTable, TRUE);
	auto findProduct = m_tableProducts.find(strID);
	if(findProduct == std::end(m_tableProducts))
		m_tableProducts.emplace(strID, m_nProductIndex);

	if(m_tableProducts.size() > MAXIMUM_TABLE_SIZE)
	{
		std::vector<int> vctValues {};
		vctValues.reserve(MAXIMUM_TABLE_SIZE + 10);
		for(auto it = m_tableProducts.begin(); it != m_tableProducts.end(); it++)
			vctValues.push_back(it->second);

		std::sort(std::begin(vctValues), std::end(vctValues));
		std::vector<int> vctRemoveTarget(std::begin(vctValues), std::begin(vctValues) + 500);

		for(auto it = m_tableProducts.begin(); it != m_tableProducts.end();)
		{
			auto findComp = [it](const int nValue)
			{
				const int nCompValue = it->second;
				const auto& strKey = it->first;
				if(nCompValue == nValue)
					return true;

				else
					return false;
			};

			auto findCompValue = std::find_if(std::begin(vctRemoveTarget), std::end(vctRemoveTarget), findComp);
			if(findCompValue != std::end(vctRemoveTarget))
				it = m_tableProducts.erase(it);

			else
				++it;
		}
	}
}

bool CStandardizedLogger::ILogData::SaveToFile()
{
	SetLogDataAndPath();
	return WriteToFile();
}

bool CStandardizedLogger::ILogData::WriteToFile()
{
	CTime curTime = CTime::GetCurrentTime();
	CString strDirPath;
	auto pInstance = CStandardizedLogger::GetInstance();
	strDirPath = strFilePath;

	LPTSTR buffer = strDirPath.GetBuffer();
	PathRemoveFileSpec(buffer);
	strDirPath.ReleaseBuffer();
	if(!PathFileExists(buffer))
	{
		BOOL ret = CreateDirectoryRecursive(buffer);
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
	strLogData.AppendFormat(_T("\n"));
	buffer = strLogData.GetBuffer();
	const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, &strWrite[0], dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strLogData.ReleaseBuffer();

	if(TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}

void CStandardizedLogger::SLogData::SetLogDataAndPath()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogData = this->strLogData;
	CString strAddLog;
	strAddLog.AppendFormat(_T("L[%s],"), strTime);
	strAddLog.AppendFormat(_T("%010d,"), this->nIndex);
	strAddLog.AppendFormat(_T("%s,"), strID);
	strAddLog.AppendFormat(_T("[%s],"), strThreadName);

	if(ePreTag != StandardizedLogging::EPreTag::None)
	{
		CString strPreTag = GetPreTagString(ePreTag);
		strAddLog.AppendFormat(_T("%s,"), strPreTag);
	}

	strAddLog.AppendFormat(_T("%s"), strLogData);
	if(ePostTag != StandardizedLogging::EPostTag::None)
	{
		CString strPostTag = GetPostTagString(ePostTag);
		strAddLog.AppendFormat(_T(",%s"), strPostTag);
	}

	CTime curTime = CTime::GetCurrentTime();
	auto pLogger = CStandardizedLogger::GetInstance();
	strFilePath.AppendFormat(pLogger->getLogFilePath(curTime, ESystemName::Minor, ELogFileType::ProcessLog));
	strLogData.Empty();
	strLogData.AppendFormat(strAddLog);
}
