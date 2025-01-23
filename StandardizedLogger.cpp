#include "pch.h"
#include <iomanip>
#include "StandardizedLogger.h"

std::mutex CStandardizedLogger::m_mtxInstance;

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::s_pInstance = nullptr;

CString GetFormattedTime(const CTime& tmLastProduct);

void CharDeleter(char* ptr);

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::GetInstance()
{
	if (nullptr == s_pInstance)
	{
		std::lock_guard<std::mutex> lock(m_mtxInstance);
		if (nullptr == s_pInstance)
		{
			s_pInstance = std::shared_ptr<CStandardizedLogger>(new CStandardizedLogger());
			s_pInstance->init();
		}
	}

	return s_pInstance;
}

void CStandardizedLogger::WriteMainLoopStart(const int nMainThreadIdx, int* pNullIdIdx)
{
	CString strMainLoopStart;
	strMainLoopStart.AppendFormat(_T("%s,[MLS]"), m_strVisionSystemMinorName);
	int nProductIdx{};
	{
		try
		{
			std::lock_guard<std::mutex> lock(m_mtxTable);
			CTime tmLastProduct = CTime::GetCurrentTime();
			CTimeSpan timeDiff = tmLastProduct - m_tmResetTime;
			if (timeDiff.GetTotalHours() >= 24)
			{
				m_tmResetTime = CTime(tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay(), 6, 0, 0);
				m_tableProductIdx.nCurProductIndex.store(0);
			}

			auto it = m_tableProductIdx.TableProductIdx.find(NULL_ID);
			if (it == std::end(m_tableProductIdx.TableProductIdx))
			{
				m_tableProductIdx.TableProductIdx.emplace(NULL_ID, std::make_pair(0, CTime::GetCurrentTime()));
				it = m_tableProductIdx.TableProductIdx.find(NULL_ID);
			}

			m_tableProductIdx.nCurProductIndex.fetch_add(1);
			it->second.first = m_tableProductIdx.nCurProductIndex.load();
			it->second.second = tmLastProduct;
			if (pNullIdIdx != nullptr)
				*pNullIdIdx = m_tableProductIdx.nCurProductIndex.load();

			nProductIdx = m_tableProductIdx.nCurProductIndex.load();
		}
		catch (const std::exception& e)
		{
			auto szWhat = e.what();
		}
	}

	WriteProcessLogWithIdx(EProcessLogThread::MainThread, nMainThreadIdx, NULL_ID, nProductIdx, strMainLoopStart);
}

void CStandardizedLogger::WriteMainLoopEnd(const CString& strProductId, const int nMainThreadIdx)
{
	CString strMainLoopEnd;
	strMainLoopEnd.AppendFormat(_T("%s,[MLE]"), m_strVisionSystemMinorName);
	writeProcessLogWithRecentCellInfo(EProcessLogThread::MainThread, nMainThreadIdx, strProductId, strMainLoopEnd);
	const bool isFirstLoopAfterProgramOn = m_bIsFirstLoopAfterProgramOn.load();
	if (isFirstLoopAfterProgramOn)
	{
		WriteAlarmLog(strProductId, LOOP_FIRST_CELL_AFTER_PROGRAM_ON);
		m_bIsFirstLoopAfterProgramOn.store(false);
	}

	const bool isFirstLoopAfterAlarm = m_bIsFirstLoopAfterAlarm.load();
	if (isFirstLoopAfterAlarm)
	{
		WriteAlarmLog(strProductId, LOOP_FIRST_CELL_AFTER_LAST_ALARM(_T("LAST ALARM")));
		m_bIsFirstLoopAfterAlarm.store(false);
	}
}

UINT CStandardizedLogger::saveLogThreading()
{
	const auto pInstance = CStandardizedLogger::GetInstance();
	while (pInstance->m_bThreadRunning.load())
	{
		std::queue<std::shared_ptr<IFileData>>& queueLogData = pInstance->m_queLogData;
		std::shared_ptr<IFileData> pLogData{};
		{
			std::unique_lock<std::mutex> lock(pInstance->m_mtxQueue);
			if (queueLogData.empty())
			{
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::seconds(3));
				{
					std::lock_guard<std::mutex> lockThreadTable(pInstance->m_mtxThreadTable);
					CTime tmCurrent = CTime::GetCurrentTime();
					for (const auto& it : pInstance->m_tableThreadList)
						pInstance->pushListLog(tmCurrent, it);
				}
				
				continue;
			}

			pLogData = queueLogData.front();
			queueLogData.pop();
		}

		if (pLogData == nullptr)
			continue;

		bool bSaveResult = pLogData->SaveToFile();
		if (!bSaveResult)
		{
			//ToDo Alarm Fail To Save StandardLog
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	return 0;
}

void CStandardizedLogger::formatProcessLog(const std::shared_ptr<SProcessLogData>& pProcessLogData, EPreTag ePreTag,
                                           EPostTag ePostTag)
{
	CString strAddLog;
	strAddLog.AppendFormat(_T("L[%s],"), pProcessLogData->strTime);
	strAddLog.AppendFormat(_T("%010d,"), pProcessLogData->nIndex);
	strAddLog.AppendFormat(_T("%s,"), pProcessLogData->strId);
	strAddLog.AppendFormat(_T("[%s],"), pProcessLogData->strThreadName);

	if (ePreTag != EPreTag::None)
	{
		CString strPreTag = GetPreTagString(ePreTag);
		strAddLog.AppendFormat(_T("%s,"), strPreTag);
	}

	strAddLog.AppendFormat(_T("%s"), pProcessLogData->strFileData);
	if (ePostTag != EPostTag::None)
	{
		CString strPostTag = GetPostTagString(ePostTag);
		strAddLog.AppendFormat(_T(",%s"), strPostTag);
	}

	auto& strProcessLogFilePath = pProcessLogData->strFilePath;
	auto& strLogRow = pProcessLogData->strFileData;
	strProcessLogFilePath.AppendFormat(getLogFilePath(ESystemName::Minor, ELogFileType::ProcessLog,
	                                                  pProcessLogData->tmLogTime));
	strLogRow.Empty();
	strLogRow.AppendFormat(strAddLog);
}


bool CStandardizedLogger::init()
{
	bool bRet = true;
	do
	{
		try
		{
			std::lock_guard<std::mutex> lock(m_mtxTable);
			const auto findNullId = m_tableProductIdx.TableProductIdx.find(NULL_ID);
			if (findNullId != std::end(m_tableProductIdx.TableProductIdx))
			{
				AfxMessageBox(_T("[StandardizedLog] Initialization Failed - NULL_ID Index is not set to 1"));
				return false;
			}

			m_tableProductIdx.TableProductIdx.emplace(NULL_ID, std::make_pair(0, CTime::GetCurrentTime()));
			TCHAR szPath[MAX_PATH];
			m_strExeFileName.Empty();
			if (GetModuleFileNameW(NULL, szPath, MAX_PATH))
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
				if (bDdriveExist)
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

					if (hDrive == INVALID_HANDLE_VALUE)
					{
						DWORD dwError = GetLastError();
						TRACE(_T("The Error Code is %d\n"), dwError);
						break;
					}

					constexpr TCHAR szTestData[] = _T("Test Data");
					const DWORD dwDataSize = static_cast<DWORD>((_tcslen(szTestData) * sizeof(TCHAR)));
					DWORD dwBytesWritten = 0;
					BOOL bRet = WriteFile(hDrive, szTestData, dwDataSize, &dwBytesWritten, nullptr);
					if (bRet && dwDataSize == dwBytesWritten)
						m_bCanWriteToDDrive = true;

					bRet &= CloseHandle(hDrive);
					bRet &= DeleteFile(strTestFilePath);
					const CString strLogSwDirPath = _T("D:\\LOG_SW");
					if (!PathFileExists(strLogSwDirPath))
					{
						bRet &= CreateDirectory(_T("D:\\LOG_SW"), NULL);
						if (!bRet)
						{
							DWORD dwError = GetLastError();
							CString strError;
							strError.Format(
								_T("[Standardized Logging] Failed to Create Directory D:\\LOG_SW\\  - Error Code : %d"),
								dwError);
						}

						if (!bRet)
							return false;
					}

					else
					{
						const CString strRecentProductInfoFile = getLogFilePath(
							ESystemName::Minor, ELogFileType::RecentProductInfo, CTime::GetCurrentTime());
						if (PathFileExists(strRecentProductInfoFile))
						{
							CFile file;
							BOOL bOpenFile = file.Open(strRecentProductInfoFile, CFile::modeRead);
							if (!bOpenFile)
							{
								DWORD dwError = GetLastError();
								CString strMsg;
								strMsg.Format(
									_T("[StandardizedLog] Failed To Read RecentProductInfo File. Error Code : %d"),
									dwError);
								return false;
							}

							unsigned long long ullFileSize = file.GetLength();
							std::string strBufRead(ullFileSize, '\0');
							strBufRead.reserve(150);
							BOOL bReadResult = file.Read(&strBufRead[0], (UINT)ullFileSize);
							file.Close();
							if (!bReadResult)
							{
								DWORD dwError = GetLastError();
								CString strMsg;
								strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);
								//CLogManager::Write(0, strMsg);
								return true;
							}

							std::vector<std::string> vctRecent = Split(strBufRead, ',');
							if (vctRecent.size() >= 3)
							{
								std::tm tm = {};
								std::istringstream ss(vctRecent[0]);
								ss >> std::get_time(&tm, "%Y-%m-%d-[%H:%M:%S]");
								time_t tmTimeObj = std::mktime(&tm);
								CTime tmLastProduct(tmTimeObj);
								const auto pTmLastProductResetTimeStamp = std::make_unique<CTime>();
								if (tmLastProduct.GetHour() < 6)
									*pTmLastProductResetTimeStamp = CTime(tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay() - 1, 6, 0, 0);

								else
									*pTmLastProductResetTimeStamp = CTime(tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay(), 6, 0, 0);

								CTime tmCurrent = CTime::GetCurrentTime();
								const auto pTmCurrentResetTimeStamp = std::make_unique<CTime>();
								if (tmCurrent.GetHour() < 6)
									*pTmCurrentResetTimeStamp = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay() - 1, 6, 0, 0);

								else
									*pTmCurrentResetTimeStamp = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay(), 6, 0, 0);
								
								CTimeSpan tsGap = *pTmCurrentResetTimeStamp - *pTmLastProductResetTimeStamp;
								if (tsGap.GetTotalHours() >= 24)
								{
									m_tableProductIdx.nCurProductIndex = 0;
									DeleteFile(strRecentProductInfoFile);
								}

								else
								{
									const std::string& strLastProductCount = vctRecent[1];
									const int nLastProductCount = std::stoi(strLastProductCount);
									m_tableProductIdx.nCurProductIndex = nLastProductCount;
								}
							}
							else
							{
								WriteAlarmLog(NULL_ID, RECENT_PRODUCT_INFO_FILE_FORMAT_WRONG);
							}
						}

						CTime tmLastProduct = CTime::GetCurrentTime();
						m_tmResetTime = CTime(tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay(), 6, 0, 0);
					}
				}

				m_bIsFirstLoopAfterProgramOn.store(true);
				m_bIsFirstLoopAfterAlarm.store(false);
			}
			while (false);

			m_bThreadRunning.store(true);
			m_saveThread = std::thread(&CStandardizedLogger::saveLogThreading);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}
	while (false);

	return bRet;
}

std::vector<std::string> CStandardizedLogger::Split(const std::string& str, const char delimiter)
{
	std::vector<std::string> vctTokens;
	vctTokens.reserve(10);
	std::stringstream ss(str);
	std::string strToken;
	while (std::getline(ss, strToken, delimiter))
	{
		vctTokens.push_back(strToken);
	}

	return vctTokens;
}

void CStandardizedLogger::writeProcessLogWithRecentCellInfo(const StandardizedLogging::EProcessLogThread eLogThread,
                                                            const int nThreadIdx, const CString& strProductId,
                                                            CString strContent, ...)
{
	const auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.AppendFormatV(strContent, args);
	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = getProductIdxFromTable(strProductId);
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);

	const auto pRecentProductInfo = std::make_shared<SRecentProductInfoData>();
	const CTime& tmProductTime = pLogData->tmLogTime;
	auto& strRecentProductInfo = pRecentProductInfo->strFileData;
	strRecentProductInfo.AppendFormat(
		_T("%04d-%02d-%02d-[%02d-%02d-%02d],"), tmProductTime.GetYear(), tmProductTime.GetMonth(),
		tmProductTime.GetDay(), tmProductTime.GetHour(), tmProductTime.GetMinute(), tmProductTime.GetSecond());
	strRecentProductInfo.AppendFormat(_T("%010d,"), pLogData->nIndex);
	strRecentProductInfo.AppendFormat(_T("%s,"), pLogData->strId);
	pRecentProductInfo->strFilePath = getLogFilePath(ESystemName::Minor, StandardizedLogging::ELogFileType::RecentProductInfo,
	                                                 pLogData->tmLogTime);
	pushLogData(pRecentProductInfo);
}

void CStandardizedLogger::writeSystemLogInternal(const CString& strProductId,
                                                 const StandardizedLogging::ESystemLogThread eSystemLogThread,
                                                 const CString& strLogContent,
                                                 const StandardizedLogging::EPreTag ePreTag,
                                                 const StandardizedLogging::EPostTag ePostTag)
{
	const CTime tmLastProduct = CTime::GetCurrentTime();
	const CString strLogTime = GetFormattedTime(tmLastProduct);
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
	strThreadName.Format(_T("%s"), GetSystemLogThreadName(eSystemLogThread));
	strLogContents.AppendFormat(_T("%s,"), strThreadName);
	ASSERT(ePreTag == EPreTag::None || ePostTag == EPostTag::None);
	if (ePreTag != EPreTag::None)
	{
		CString strPreTag = GetPreTagString(ePreTag);
		strLogContents.AppendFormat(_T("%s,"), strPreTag);
	}

	strLogContents.AppendFormat(strLogContent);
	if (ePostTag != EPostTag::None)
	{
		CString strPostTag = GetPostTagString(ePostTag);
		strLogContents.AppendFormat(_T(",%s"), strPostTag);
	}

	strLogPath.AppendFormat(getLogFilePath(ESystemName::Minor, ELogFileType::SystemLog, tmLastProduct));
	pushLogData(pLogData);
	pushListLog(tmLastProduct, _T("SYSTEM"));
}

void CStandardizedLogger::pushListLog(const CTime& tmLastProduct, const CString& strThreadName)
{
	const auto pListLogItem = std::make_shared<SListLogData>();
	auto& strListLogContent = pListLogItem->strFileData;
	auto& strListLogPath = pListLogItem->strFilePath;
	strListLogContent.Empty();
	strListLogPath.Empty();
	strListLogPath.AppendFormat(getLogFilePath(ESystemName::Minor, ELogFileType::ListLog, tmLastProduct));
	strListLogContent.AppendFormat(strThreadName);
	pushLogData(pListLogItem);

	{
		std::lock_guard<std::mutex> lock(m_mtxThreadTable);
		const auto findThreadName = m_tableThreadList.find(strThreadName);
		if (findThreadName == std::end(m_tableThreadList))
			m_tableThreadList.insert(strThreadName);
	}
}

void CStandardizedLogger::pushLogData(const std::shared_ptr<IFileData>& pLogData)
{
	std::lock_guard<std::mutex> lock(m_mtxQueue);
	pLogData->strFileData.AppendFormat(_T("\n"));
	m_queLogData.push(pLogData);
}

int CStandardizedLogger::getProductIdxFromTable(const CString& strProductId)
{
	int nIdx{};
	{
		try
		{
			std::lock_guard<std::mutex> lock(m_mtxTable);
			const auto findProduct = m_tableProductIdx.TableProductIdx.find(strProductId);
			if (findProduct == std::end(m_tableProductIdx.TableProductIdx))
				nIdx = m_tableProductIdx.nCurProductIndex.load();

			else
			{
				const std::pair<int, CTime>& productIdx = findProduct->second;
				nIdx = productIdx.first;
			}
		}
		catch (const std::exception& e)
		{
			auto szWhat = e.what();
		}
	}

	return nIdx;
}

void CStandardizedLogger::WriteAlarmLog(const CString& strProductId, const CString& strLogContent)
{
	CTime tmLastProduct = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(tmLastProduct);
	const auto pLogData = std::make_shared<SAlarmLogData>();
	auto& strLogContents = pLogData->strFileData;
	auto& strLogPath = pLogData->strFilePath;
	strLogContents.Empty();
	strLogPath.Empty();
	strLogContents.AppendFormat(_T("L[%s],"), strLogTime);
	const int nProductIdx = getProductIdxFromTable(strProductId);
	if (nProductIdx != -1)
		strLogContents.AppendFormat(_T("%010d,"), nProductIdx);

	else
		strLogContents.AppendFormat(_T("0"));

	pLogData->nIndex = nProductIdx;
	strLogContents.AppendFormat(_T("%s,"), strProductId);
	strLogContents.AppendFormat(_T("[ARM],"));
	strLogContents.AppendFormat(strLogContent);

	CString strThreadName;
	strThreadName.AppendFormat(_T("ALARM"));
	strLogPath.AppendFormat(getLogFilePath(ESystemName::Minor, ELogFileType::AlarmLog, tmLastProduct));
	pushLogData(pLogData);
	pushListLog(tmLastProduct, strThreadName);
	m_bIsFirstLoopAfterAlarm.store(true);
}

void CStandardizedLogger::WriteResultLog(IResultLog& iResultLog)
{
	writeResultLogInternal(iResultLog.GetProductId(), iResultLog.GetProductId(), iResultLog.GetIndividualResult(),
	                       iResultLog.GetPath(), iResultLog.vctValue);
}

void CStandardizedLogger::writeResultLogInternal(const CString& strModuleId, const CString& strCellId, bool bResult,
                                                 const CString& strImgPath, const std::vector<CString>& vctLogs)
{
	CTime tmLastProduct = CTime::GetCurrentTime();
	CString strLogTime = GetFormattedTime(tmLastProduct);
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
	if (bResult)
		strLogContents.AppendFormat(_T("OK"));

	else
		strLogContents.AppendFormat(_T("NG"));

	for (const CString& str : vctLogs)
	{
		strLogContents.AppendFormat(_T(",%s"), str);
	}

	strLogContents.AppendFormat(_T(",%s"), strImgPath);
	strLogPath.AppendFormat(getLogFilePath(ESystemName::Minor, ELogFileType::ResultLog, tmLastProduct));
	pushLogData(pLogData);
	pushListLog(tmLastProduct, _T("RESULT"));
}

void CStandardizedLogger::WriteSystemLog(const CString& strProductId,
                                         const StandardizedLogging::ESystemLogThread eSystemLogThread,
                                         const CString& strLogContent)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, EPreTag::None, EPostTag::None);
}

void CStandardizedLogger::WriteSystemLogPreTag(const CString& strProductId,
                                               const StandardizedLogging::ESystemLogThread eSystemLogThread,
                                               const CString& strLogContent, const StandardizedLogging::EPreTag ePreTag)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, ePreTag, EPostTag::None);
}

void CStandardizedLogger::WriteSystemLogPostTag(const CString& strProductId,
                                                const StandardizedLogging::ESystemLogThread eSystemLogThread,
                                                const CString& strLogContent,
                                                const StandardizedLogging::EPostTag ePostTag)
{
	writeSystemLogInternal(strProductId, eSystemLogThread, strLogContent, EPreTag::None, ePostTag);
}

CString CStandardizedLogger::getLogFilePath(const ESystemName eName, const ELogFileType eLogType,
                                            const CTime& tmLastProduct) const
{
	CString strLogFilePath;
	if (m_bCanWriteToDDrive)
		strLogFilePath.AppendFormat(_T("D:\\"));

	else
		strLogFilePath.AppendFormat(_T("C:\\"));

	StandardizedLogging::SLogFileType sLogFileType{};
	sLogFileType.eLogFileType = eLogType;
	if (eLogType != ELogFileType::RecentProductInfo)
	{
		const int nYear = tmLastProduct.GetYear();
		const int nMonth = tmLastProduct.GetMonth();
		const int nDay = tmLastProduct.GetDay();
		const int nHour = tmLastProduct.GetHour();
		strLogFilePath.AppendFormat(_T("LOG_SW\\%04d%02d%02d\\%02d\\"),
		                            nYear,
		                            nMonth,
		                            nDay,
		                            nHour);

		const CString strSystemName = getSystemName(eName);
		const CString strLogFileType = sLogFileType.ToString();

		strLogFilePath.AppendFormat(
			_T("I_INSPID1_%s-VISION_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName, nYear,
			nMonth, nDay, nHour);
	}

	else
	{
		strLogFilePath.AppendFormat(_T("LOG_SW\\%s.txt"), sLogFileType.ToString());
	}

	return strLogFilePath;
}

CString CStandardizedLogger::getSystemName(const ESystemName eSystem) const
{
	if (eSystem == ESystemName::Major)
		return m_strVisionSystemMajorName;

	else
		return m_strVisionSystemMinorName;
}

void CStandardizedLogger::Clear()
{
	m_bThreadRunning.store(false);
	m_saveThread.join();
	std::lock_guard<std::mutex> queueLock(m_mtxQueue);
	while (!m_queLogData.empty())
	{
		const auto pLogItem = m_queLogData.front();
		pLogItem->SaveToFile();
		m_queLogData.pop();
	}
}

CStandardizedLogger::CStandardizedLogger()
	: m_tableProductIdx()
	  , m_bThreadRunning()
	  , m_saveThread{}
	  , m_strVisionSystemMinorName(CString{_T("MinorNameNotSet")})
	  , m_strVisionSystemMajorName(CString{_T("MajorNameNotSet")})
	  , m_bCanWriteToDDrive(false)
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

	for (TCHAR* p = szPath; *p; ++p)
	{
		if (*p == _T('\\') || *p == _T('/'))
		{
			*p = _T('\0');
			CString strDir;
			strDir.AppendFormat(szPath);
			bool bDrive = strDir[strDir.GetLength() - 1] == _T(':');
			if (!bDrive)
			{
				BOOL bCreateResult = CreateDirectory(szPath, NULL);
				if (!bCreateResult)
				{
					DWORD dwError = GetLastError();
					if (dwError != ERROR_ALREADY_EXISTS)
						return false;
				}
			}

			*p = _T('\\');
		}
	}

	return CreateDirectory(szPath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

CString GetFormattedTime(const CTime& tmLastProduct)
{
	const auto now = std::chrono::system_clock::now();
	const auto duration = now.time_since_epoch();
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
	const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) -
	std::chrono::duration_cast<std::chrono::milliseconds>(seconds);
	const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration) -
	std::chrono::duration_cast<std::chrono::microseconds>(seconds);
	const std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

	CString formattedTime;
	formattedTime.AppendFormat(_T("%02d:%02d:%02d:%03d:%03d"),
	                           tmLastProduct.GetHour(),
	                           tmLastProduct.GetMinute(),
	                           tmLastProduct.GetSecond(),
	                           milliseconds.count(),
	                           microseconds.count() % 1000
	);

	return formattedTime;
}

void CStandardizedLogger::writeProcessLogInternal(const std::shared_ptr<SProcessLogData>& pProcessLogData,
                                                  EProcessLogThread eLogThread, const int nThreadIdx,
                                                  const CString& strProductId)
{
	if (nullptr == pProcessLogData)
		return;

	pProcessLogData->tmLogTime = CTime::GetCurrentTime();
	pProcessLogData->strTime = GetFormattedTime(pProcessLogData->tmLogTime);
	pProcessLogData->strThreadName.Format(_T("%s-%d"), GetProcessLogThreadName(eLogThread), nThreadIdx + 1);
	pProcessLogData->strId = strProductId;
}

void CStandardizedLogger::WriteProcessLog(const StandardizedLogging::EProcessLogThread eLogThread, const int nThreadIdx,
                                          const CString& strProductId, CString strContent, ...)
{
	const auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	va_list args;
	va_start(args, strContent);
	CString strResult;

	strResult.AppendFormatV(strContent, args);
	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = getProductIdxFromTable(strProductId);
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogWithIdx(const StandardizedLogging::EProcessLogThread eLogThread,
                                                 const int nThreadIdx, const CString& strProductId,
                                                 const int nProductIdx, CString strContent, ...)
{
	const auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch (...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->nIndex = nProductIdx;
	formatProcessLog(pLogData, EPreTag::None, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogPreTag(const StandardizedLogging::EProcessLogThread eLogThread,
                                                const int nThreadIdx, const CString& strProductId,
                                                const EPreTag ePreTag, CString strContent, ...)
{
	const auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch (...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->nIndex = getProductIdxFromTable(strProductId);
	formatProcessLog(pLogData, ePreTag, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogPreTagWithIdx(StandardizedLogging::EProcessLogThread eLogThread,
                                                       const int nThreadIdx, const CString& strProductId,
                                                       const int nProductIdx, EPreTag ePreTag, CString strContent, ...)
{
	const auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch (...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->nIndex = nProductIdx;
	formatProcessLog(pLogData, ePreTag, EPostTag::None);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

void CStandardizedLogger::WriteProcessLogDoubleTags(const StandardizedLogging::EProcessLogThread eLogThread,
                                                    const int nThreadIdx, const CString& strProductId,
                                                    const EPreTag ePreTag, const EPostTag ePostTag, CString strContent,
                                                    ...)
{
	auto pLogData = std::make_shared<SProcessLogData>();
	writeProcessLogInternal(pLogData, eLogThread, nThreadIdx, strProductId);
	CString strResult;
	va_list args;
	va_start(args, strContent);
	try
	{
		strResult.FormatV(strContent, args);
	}
	catch (...)
	{
		va_end(args);
		return;
	}

	va_end(args);

	pLogData->strFileData = strResult;
	pLogData->ePreTag = ePreTag;
	pLogData->ePostTag = ePostTag;
	pLogData->nIndex = getProductIdxFromTable(strProductId);
	formatProcessLog(pLogData, ePreTag, ePostTag);
	pushListLog(pLogData->tmLogTime, pLogData->strThreadName);
	pushLogData(pLogData);
}

std::vector<CString> CStandardizedLogger::SplitCString(const CString& strInput, const TCHAR tcDelimiter)
{
	std::vector<CString> vctResult;
	int nStart = 0;
	int nEnd = 0;
	while ((nEnd = strInput.Find(tcDelimiter, nStart)) != -1)
	{
		CString token = strInput.Mid(nStart, nEnd - nStart);
		vctResult.push_back(token);
		nStart = nEnd + 1;
	}

	vctResult.push_back(strInput.Mid(nStart));
	return vctResult;
}

CString CStandardizedLogger::GetFilePath(const CString& strProductId, const int nCamIdx, const int nImgIdx,
                                         const bool bIsOk, const bool bIsOverlay, const EFileExtensionType eFileType)
{
	if ((bIsOverlay && eFileType == EFileExtensionType::Jpg) || (!bIsOverlay))
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
	if (bIsOk)
	{
		strFilePath.AppendFormat(_T("\\OK\\%s\\"), strProductId);
		strFileName.AppendFormat(_T("OK"));
	}

	else
	{
		strFilePath.AppendFormat(_T("\\NG\\%s\\"), strProductId);
		strFileName.AppendFormat(_T("NG"));
	}

	if (bIsOverlay)
		strFileName.AppendFormat(_T("_Overlay"));

	SFileExtensionType sFileExtension{};
	sFileExtension.eType = eFileType;
	strFileName.AppendFormat(sFileExtension.ToString());
	strFilePath.AppendFormat(strFileName);

	return strFilePath;
}

void CStandardizedLogger::RegisterProductId(const CString& strId, const int nBarcodeCount)
{
	if (strId.GetLength() < 1)
	{
		WriteAlarmLog(NULL_ID, PLC_SERIAL_NUM_ERROR);
		return;
	}

	if (strId.Compare(NULL_ID) == 0)
		return;

	{
		try
		{
			std::lock_guard<std::mutex> lock(m_mtxTable);
			const auto findProduct = m_tableProductIdx.TableProductIdx.find(strId);
			if (findProduct != std::end(m_tableProductIdx.TableProductIdx))
				return;

			if (m_tableProductIdx.TableProductIdx.size() > MAXIMUM_TABLE_SIZE)
			{
				std::vector<std::pair<CString, CTime>> vctPathTime{};
				vctPathTime.reserve(MAXIMUM_TABLE_SIZE + 10);
				for (const auto& it : m_tableProductIdx.TableProductIdx)
				{
					const CString& strProductId = it.first;
					const CTime& tmProduct = it.second.second;
					if (strProductId == NULL_ID)
						continue;

					vctPathTime.emplace_back(std::make_pair(strProductId, tmProduct));
				}

				auto sortByTimeStamp = [](const std::pair<CString, CTime>& p1, const std::pair<CString, CTime>& p2)
				{
					return p1.second < p2.second;
				};

				std::sort(std::begin(vctPathTime), std::end(vctPathTime), sortByTimeStamp);
				const int nEraseCount = static_cast<int>(vctPathTime.size()) - 50;
				for (int i = 0; i < nEraseCount; i++)
				{
					const std::pair<CString, CTime>& oldData = vctPathTime[i];
					if (oldData.first == NULL_ID)
						continue;

					const auto findProductIdx = m_tableProductIdx.TableProductIdx.find(oldData.first);
					if (findProductIdx != std::end(m_tableProductIdx.TableProductIdx))
						m_tableProductIdx.TableProductIdx.erase(findProductIdx);
				}
			}

			int nProductBarcodeCount;
			if (nBarcodeCount != -1)
				nProductBarcodeCount = nBarcodeCount;

			else
				nProductBarcodeCount = m_tableProductIdx.nCurProductIndex.load();

			m_tableProductIdx.TableProductIdx.emplace(
				strId, std::make_pair(nProductBarcodeCount, CTime::GetCurrentTime()));
		}

		catch (const std::exception& e)
		{
			auto sz = e.what();
		}
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
	if (!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if (!ret)
		{
			DWORD dwError = GetLastError();
			CString strMsg;
			strMsg.Format(_T("[StandardizedLog] Failed To Make Directory Path. Error Code : %d"), dwError);
			return false;
		}
	}

	HANDLE hFile = CreateFile(strFilePath, // name of the write
	                          FILE_APPEND_DATA, // open for writings
	                          0, // do not share
	                          NULL, // default security
	                          OPEN_ALWAYS, // create new file only
	                          FILE_ATTRIBUTE_NORMAL, // normal file
	                          NULL); // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		DWORD dwError = GetLastError();
		CString strMsg;
		strMsg.Format(_T("[StandardizedLog] Failed To Create File. Error Code : %d"), dwError);
		return false;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	DWORD dwByesWritten{};
	buffer = strFileData.GetBuffer();
	const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, strWrite.data(), dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if (TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}

bool CStandardizedLogger::SListLogData::SaveToFile()
{
	auto& strFilePath = this->strFilePath;
	auto& strLogContents = this->strFileData;
	if (PathFileExists(strFilePath))
	{
		CFile file;
		BOOL bOpenFile = file.Open(strFilePath, CFile::modeRead);
		if (!bOpenFile)
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
		if (!bReadResult)
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
		if (nfoundIdx == std::string::npos)
		{
			bool bDoesThreadNameContainNumber = false;
			char* pChTmp = pFindContent.get();
			int nThreadNumberPos = 0;
			while (*pChTmp)
			{
				if (std::isdigit(*pChTmp))
				{
					bDoesThreadNameContainNumber = true;
					break;
				}

				++pChTmp;
				++nThreadNumberPos;
			}

			if (!bDoesThreadNameContainNumber)
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
				while (nFindNext != std::string::npos)
				{
					const int nWrittenSameThreadIdx = strBufRead[nFindNext + nThreadNumberPos] - '0';
					nFindCur = nFindNext;
					if (nWrittenSameThreadIdx > nWriteThreadIdx)
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

				if (nFindCur <= strBufRead.size())
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
	if (!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if (!ret)
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

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwByesWritten{};
	buffer = strFileData.GetBuffer();
	const int nUtf8Len = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, NULL, 0, NULL, NULL);
	std::string strWrite;
	strWrite.reserve(nUtf8Len);
	const DWORD dwBytesToWrite = nUtf8Len - 1;
	WideCharToMultiByte(CP_UTF8, 0, buffer, -1, &strWrite[0], nUtf8Len, NULL, NULL);
	BOOL bWriteResult = WriteFile(hFile, &strWrite[0], dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if (TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}

CString StandardizedLogging::SLogFileType::ToString()
{
	switch (eLogFileType)
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
	if (!PathFileExists(strDirPath))
	{
		BOOL ret = CreateDirectoryRecursive(strDirPath);
		if (!ret)
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

	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwByesWritten{};
	buffer = strFileData.GetBuffer();
	DWORD dwBytesToWrite = sizeof(TCHAR) * strFileData.GetLength();
	BOOL bWriteResult = WriteFile(hFile, buffer, dwBytesToWrite, &dwByesWritten, NULL);
	CloseHandle(hFile);
	strFileData.ReleaseBuffer();

	if (TRUE == bWriteResult && dwBytesToWrite == dwByesWritten)
		return true;

	else
		return false;
}
