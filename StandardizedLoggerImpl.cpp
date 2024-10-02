#include "pch.h"
#include "StandardizedLoggerImpl.h"

CString GetFormattedTime(const CTime& curTime);

void CStandardizedLoggerImpl::WriteMainLoopStart(const int nProductCount, const int nMainThreadIdx)
{
	WriteProcessLog(nProductCount, _T("Null"), EProcessLogThread::MainThread, nMainThreadIdx, m_strVisionSystemMinorName, EPreTag::None, EPostTag::MainLoopStart);
}

void CStandardizedLoggerImpl::WriteMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx)
{
	WriteProcessLog(nProductCount, strProductId, EProcessLogThread::MainThread, nMainThreadIdx, m_strVisionSystemMinorName, EPreTag::None, EPostTag::MainLoopEnd);
}

UINT CStandardizedLoggerImpl::SaveLogThreading(LPVOID pParam)
{
	auto* pInstance = (CStandardizedLoggerImpl*)pParam;
	while(pInstance->m_bThreadRunning)
	{
		std::shared_ptr<SLogItem> pLogItem { nullptr };
		bool bItemExist = pInstance->PopLogItem(pLogItem);
		if(!bItemExist)
		{
			Sleep(5000);
			continue;
		}
		else
		{
			bool bSaveResult = false;
			do
			{
				bSaveResult = pLogItem->Save();
				Sleep(3);
			}
			while(!bSaveResult);
		}
	}

	SetEvent(pInstance->m_hThreadTerminatedEvent);

	return 0;
}

void CStandardizedLoggerImpl::StartSaveStandardLogThread()
{
	m_bThreadRunning = true;
	m_hThreadTerminatedEvent = CreateEvent(NULL, TRUE, FALSE, _T("StandardLog Save Thread Terminated"));
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

void CStandardizedLoggerImpl::PushListLog(const CTime & curTime, const CString & strThreadName)
{
	const auto pListLogItem = std::make_shared<SListFileLogItem>();
	auto& strListLogContent = pListLogItem->strLogContent;
	auto& strListLogPath = pListLogItem->strFilePath;
	strListLogContent.Empty();
	strListLogPath.Empty();
	strListLogPath.AppendFormat(GetLogFilePath(curTime, ESystemName::Minor, ELogFileType::ThreadList));
	strListLogContent.AppendFormat(strThreadName);
	PushLogItemToQueue(pListLogItem);
}

void CStandardizedLoggerImpl::WriteProcessLog(const int nProductCount, const CString & strProductId, const EProcessLogThread eLogThread, const int nThreadIdx, const CString & strLogContent, const EPreTag ePreTag, const EPostTag ePostTag)
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
	strThreadName.AppendFormat(_T("[%s-%d],"), StandardizedLogging::GetProcessLogThreadName(eLogThread), nThreadIdx);
	strLogContents.AppendFormat(strThreadName);

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

	strLogPath.AppendFormat(GetLogFilePath(curTime, ESystemName::Minor, ELogFileType::ProcessLog));
	PushLogItemToQueue(pLogItem);
	PushListLog(curTime, strThreadName);
}

void CStandardizedLoggerImpl::WriteAlarmLog(const int nProductCount, const CString & strProductId, const CString & strLogContent)
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
	strLogPath.AppendFormat(GetLogFilePath(curTime, ESystemName::Minor, ELogFileType::AlarmLog));
	PushLogItemToQueue(pLogItem);
	PushListLog(curTime, strThreadName);
}

void CStandardizedLoggerImpl::WriteResultLog(const int nProductCount, const CString & strModuleId, const CString & strCellId, const StandardizedLogging::EResultValue eResultValue, const CString & strImgPath, const std::vector<CString>& vctLogs)
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
	strLogPath.AppendFormat(GetLogFilePath(curTime, ESystemName::Minor, ELogFileType::ResultLog));
	PushListLog(curTime, _T("RESULT"));
	PushLogItemToQueue(pLogItem);
}

void CStandardizedLoggerImpl::WriteSystemLog(const int nProductCount, const CString & strProductId, const StandardizedLogging::ESystemLogThread eLogThread, const CString & strLogContent, const StandardizedLogging::EPreTag ePreTag, const StandardizedLogging::EPostTag ePostTag)
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
	strThreadName.Format(_T("%s,"), StandardizedLogging::GetSystemLogThreadName(eLogThread));
	strLogContents.AppendFormat(strThreadName);

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

	strLogPath.AppendFormat(GetLogFilePath(curTime, ESystemName::Minor, ELogFileType::SystemLog));
	PushLogItemToQueue(pLogItem);
	PushListLog(curTime, strThreadName);
}

bool DoesDriveExist(char driveLetter)
{
	DWORD drives = GetLogicalDrives();
	if(drives & (1 << (driveLetter - 'A')))
		return true;

	else
		return false;
}

CString CStandardizedLoggerImpl::GetLogFilePath(const CTime& curTime, const ESystemName eName, const ELogFileType eLogType) const
{
	CString strLogFilePath;
	const TCHAR d_Drive = _T('D');
	bool bCanWriteToDDrive = false;
	DWORD drives = GetLogicalDrives();
	if(drives & (1 << (d_Drive - _T('A'))))
	{
		CString strTestFilePath;
		strTestFilePath.AppendFormat(_T("%s:\\Test.txt"), d_Drive);
		HANDLE hDrive = CreateFile(
			strTestFilePath,
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
		);

		if(hDrive == INVALID_HANDLE_VALUE)
		{
			DWORD dwError = GetLastError();
			TRACE(_T("The Error Code is %d\n"), dwError);
		}

		const TCHAR szTestData[] = _T("Test Data");
		const DWORD dwDataSize = (DWORD)(_tcslen(szTestData) * sizeof(TCHAR));
		DWORD dwBytesWritten = 0;
		BOOL bRet = WriteFile(hDrive, szTestData, dwDataSize, &dwBytesWritten, nullptr);
		if(bRet && dwDataSize == dwBytesWritten)
		{
			if(dwDataSize == dwBytesWritten)
				bCanWriteToDDrive = true;
		}
	}

	if(bCanWriteToDDrive)
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

	const CString strSystemName = GetSystemName(eName);
	const CString strLogFileType = GetLogTypeString(eLogType);

	strLogFilePath.AppendFormat(_T("I_INSPID_%s-VISION_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName, nYear, nMonth, nDay, nHour);

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
	const DWORD dwThreeSecondsInMil = 3000;
	WaitForSingleObject(m_hThreadTerminatedEvent, dwThreeSecondsInMil);
	CSingleLock lock(&m_csQueue);
	lock.Lock();
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

CString CStandardizedLoggerImpl::GetVisionSystemMajorName() const
{
	return m_strVisionSystemMajorName;
}

CString CStandardizedLoggerImpl::GetVisionSystemMinorName() const
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
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[표준화 로그] 경로 생성에 실패하였습니다. 에러 코드 %d"), dwError);
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
		strMessage.Format(_T("[표준화 로그] 파일 생성에 실패했습니다. 에러 코드 %d"), dwError);
		AfxMessageBox(strMessage, MB_ICONWARNING);

		return false;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD dwByesWritten {};
	buffer = strLogContent.GetBuffer();
	const DWORD nBufferSize = static_cast<DWORD>(_tcslen(buffer) * sizeof(TCHAR));
	BOOL bWriteResult = WriteFile(hFile, buffer, nBufferSize, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strLogContent.ReleaseBuffer();
	if(TRUE == bWriteResult && nBufferSize == dwByesWritten)
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
								  FILE_SHARE_READ,                      // do not share
								  NULL,                   // default security
								  OPEN_EXISTING,             // create new file only
								  FILE_ATTRIBUTE_NORMAL,  // normal file
								  NULL);                  // no attr. template

		if(hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[표준화 로그] 리스트 파일 저장 - 파일 열기에 실패하였습니다. 에러 코드 %d"), dwError);
			AfxMessageBox(strMsg, MB_ICONWARNING);

			return false;
		}

		const DWORD nLenToRead = 1024;
		TCHAR buffer[nLenToRead] {};
		const DWORD nBytesToRead = nLenToRead * sizeof(TCHAR);
		DWORD bytesRead = 0;
		BOOL result = ReadFile(
			hFile,                  // Handle to the file
			buffer,                 // Buffer to receive data
			nBytesToRead,         // Number of bytes to read
			&bytesRead,             // Number of bytes read
			NULL                    // Overlapped structure
		);
		CloseHandle(hFile);

		if(result == FALSE)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[표준화 로그] 리스트 파일 읽기 - 파일 읽기에 실패하였습니다. 에러 코드 %d"), dwError);
			AfxMessageBox(strMsg, MB_ICONWARNING);

			return false;
		}

		CString strBufRead {};
		strBufRead.AppendFormat(buffer);
		const int nfoundIdx = strBufRead.Find(strLogContents, 0);
		if(nfoundIdx == -1)
			return SLogItem::Save();

		else
			return true;
	}

	return SLogItem::Save();
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

