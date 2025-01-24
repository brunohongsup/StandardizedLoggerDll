#include "stdafx.h"
#include <iomanip>
#include "StandardizedLogger.h"


std::mutex CStandardizedLogger::m_mtxInstance;

std::shared_ptr<CStandardizedLogger> CStandardizedLogger::s_pInstance = nullptr;

CString GetFormattedTime(const CTime& tmLastProduct);

void CharDeleter(const char* ptr);

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
			std::lock_guard<std::mutex> lock(m_mtxProductIdxTable);
			CTime tmLastProduct = CTime::GetCurrentTime();
			CTimeSpan timeDiff = tmLastProduct - m_tmResetTime;
			if (timeDiff.GetTotalHours() >= 24)
			{
				m_tmResetTime = CTime(tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay(), 6, 0,
				                      0);
				m_tableProductIdx.nCurProductIndex.store(0);
			}

			auto it = m_tableProductIdx.TableProductIdx.find(NULL_ID);
			if (it == std::end(m_tableProductIdx.TableProductIdx))
			{
				m_tableProductIdx.TableProductIdx.emplace(NULL_ID, std::make_pair(0, CTime::GetCurrentTime()));
				it = m_tableProductIdx.TableProductIdx.find(NULL_ID);
			}

			std::shared_ptr<SMainThreadStatus> pStatus{};
			{
				std::lock_guard<std::mutex> lockMainThreadStatusTable(m_mtxMainThreadStatus);
				auto findMainThreadStatus = m_tableMainThreadStatus.find(nMainThreadIdx);
				if (findMainThreadStatus == std::end(m_tableMainThreadStatus))
				{
					const auto pNewStatus = std::make_shared<SMainThreadStatus>();
					pNewStatus->eStatus = EMainThreadStatus::OutOfLoop;
					m_tableMainThreadStatus.emplace(std::make_pair(nMainThreadIdx,pNewStatus));
					findMainThreadStatus = m_tableMainThreadStatus.find(nMainThreadIdx);
				}

				pStatus = findMainThreadStatus->second;
			}

			if (pStatus->eStatus == EMainThreadStatus::OutOfLoop)
				m_tableProductIdx.nCurProductIndex.fetch_add(1);

			else
				m_tableProductIdx.nCurProductIndex = pStatus->nProductCount;

			pStatus->eStatus = EMainThreadStatus::InLoop;
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
	SMainThreadStatusLog sStatus;
	sStatus.nMainThreadIdx = nMainThreadIdx;
	sStatus.nProductCount = nProductIdx;
	bool bSave = sStatus.SaveToFile();
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
	
	{
		std::lock_guard<std::mutex> lock(m_mtxMainThreadStatus);
		auto findMainThreadStatus = m_tableMainThreadStatus.find(nMainThreadIdx);
		if (findMainThreadStatus == std::end(m_tableMainThreadStatus))
		{
			const auto pNewStatus = std::make_shared<SMainThreadStatus>();
			m_tableMainThreadStatus.emplace(std::make_pair(nMainThreadIdx, std::move(pNewStatus)));
			findMainThreadStatus = m_tableMainThreadStatus.find(nMainThreadIdx);
		}

		const auto pStatusData = findMainThreadStatus->second;
		pStatusData->eStatus = EMainThreadStatus::OutOfLoop;
	}
	
	SMainThreadStatusLog sStatus;
	sStatus.nMainThreadIdx = nMainThreadIdx;
	sStatus.nProductCount = getProductIdxFromTable(strProductId);
	bool bSave = sStatus.SaveToFile();
}

UINT CStandardizedLogger::saveLogThreading()
{
	const auto pInstance = GetInstance();
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
				pInstance->pushListLogInternal(CTime::GetCurrentTime());		
				
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

BOOL CStandardizedLogger::init()
{
	BOOL bRet = FALSE;
	do
	{
		const auto findNullId = m_tableProductIdx.TableProductIdx.find(NULL_ID);
		if (findNullId != std::end(m_tableProductIdx.TableProductIdx))
		{
			AfxMessageBox(_T("[StandardizedLog] Initialization Failed - NULL_ID Index is not set to 1"));
			break;
		}

		m_tableProductIdx.TableProductIdx.emplace(NULL_ID, std::make_pair(0, CTime::GetCurrentTime()));

		TCHAR szPath[MAX_PATH];
		if (GetModuleFileNameW(NULL, szPath, MAX_PATH))
		{
			auto* pFileName = PathFindFileName(szPath);
			m_strExeFileName.Format(pFileName);
		}

		else
			m_strExeFileName.Format(_T("ExeFileNameLookUpError"));

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
				break;
		}
		const CString strRecentProductInfoFile = getLogFilePath(
			ESystemName::Minor, ELogFileType::RecentProductInfo, CTime::GetCurrentTime());

		//RecentProductInfo.txt
		{
			USES_CONVERSION;
			if (PathFileExists(strRecentProductInfoFile))
			{
				CStdioFile fileIo;
				BOOL bOpenFile = fileIo.Open(strRecentProductInfoFile, CFile::modeRead | CFile::typeText);
				if (!bOpenFile)
				{
					DWORD dwError = GetLastError();
					CString strMsg;
					strMsg.Format(
						_T("[StandardizedLog] Failed To Read RecentProductInfo File. Error Code : %d"),
						dwError);
					break;
				}

				CString strReadLine;
				BOOL bReadResult = fileIo.ReadString(strReadLine);
				if (!bReadResult)
				{
					fileIo.Close();
					DWORD dwError = GetLastError();
					CString strMsg;
					strMsg.Format(_T("[StandardizedLog] Failed To Read File. Error Code : %d"), dwError);
					//CLogManager::Write(0, strMsg);
					break;
				}

				std::string strBufRead{CT2A(strReadLine)};
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
						*pTmLastProductResetTimeStamp = CTime(
							tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay() - 1,
							6, 0, 0);

					else
						*pTmLastProductResetTimeStamp = CTime(
							tmLastProduct.GetYear(), tmLastProduct.GetMonth(), tmLastProduct.GetDay(), 6, 0,
							0);

					CTime tmCurrent = CTime::GetCurrentTime();
					const auto pTmCurrentResetTimeStamp = std::make_unique<CTime>();
					if (tmCurrent.GetHour() < 6)
						*pTmCurrentResetTimeStamp = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(),
						                                  tmCurrent.GetDay() - 1, 6, 0, 0);

					else
						*pTmCurrentResetTimeStamp = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(),
						                                  tmCurrent.GetDay(), 6, 0, 0);

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
					WriteAlarmLog(NULL_ID, RECENT_PRODUCT_INFO_FILE_FORMAT_WRONG);
			}
		}
		//MainThreadStatus.txt
		{
			m_tableMainThreadStatus.clear();
			CString strFindFilePath = strRecentProductInfoFile;
			PathRemoveFileSpec(strFindFilePath.GetBuffer());
			strFindFilePath.ReleaseBuffer();
			TCHAR buffer[MAX_PATH]{};
			PathCombine(buffer, strFindFilePath, _T("*.txt"));
			CFileFind finder;
			BOOL bWorking = finder.FindFile(buffer);
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				CString strFileName = finder.GetFileName();
				if (strFileName.Find(_T("STATUS")) != -1)
				{
					CString strFilePath = finder.GetFilePath();
					CStdioFile fileIo;
					BOOL bOpen = fileIo.Open(strFilePath, CFile::modeRead | CFile::typeText);
					if (!bOpen)
						continue;

					CString strLine;
					fileIo.ReadString(strLine);
					const auto vctString = SplitCString(strLine, _T(','));
					if (vctString.size() == 3)
					{
						int nThreadIdx = -1;
						auto& strThreadIdx = vctString[0];
						const int nStartIdx = strThreadIdx.Find(_T("STATUS-"));
						if (nStartIdx != -1)
						{
							const int nPosIdx = nStartIdx + 7;
							const int nDisplayThreadIdx = _ttoi(strThreadIdx.Mid(nPosIdx, 1));
							nThreadIdx = nDisplayThreadIdx - 1;
						}

						const auto pStatus = std::make_shared<SMainThreadStatus>();
						const int nProductCount = _ttoi(vctString[1]);
						pStatus->nProductCount = nProductCount;;
						auto& strStatus = vctString[2];
						if (strStatus.CompareNoCase(_T("InLoop")) == 0)
							pStatus->eStatus = EMainThreadStatus::InLoop;

						else if (strStatus.CompareNoCase(_T("OutOfLoop")) == 0)
							pStatus->eStatus = EMainThreadStatus::OutOfLoop;

						else
							pStatus->eStatus = EMainThreadStatus::OutOfLoop;

						const auto findThreadStatus = m_tableMainThreadStatus.find(nThreadIdx);
						if (findThreadStatus == std::end(m_tableMainThreadStatus))
							m_tableMainThreadStatus.emplace(nThreadIdx, std::move(pStatus));
					}

					fileIo.Close();
				}
			}
		}
		//Reset Time
		{
			CTime tmCurrent = CTime::GetCurrentTime();
			if (tmCurrent.GetHour() < 6)
			{
				m_tmResetTime = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay() - 1,
				                      6, 0, 0);
			}
			else
			{
				m_tmResetTime = CTime(tmCurrent.GetYear(), tmCurrent.GetMonth(), tmCurrent.GetDay(),
				                      6, 0, 0);
			}
		}


		m_bIsFirstLoopAfterProgramOn.store(true);
		m_bIsFirstLoopAfterAlarm.store(false);
		bRet = TRUE;
	}
	while (false);

	m_bThreadRunning.store(true);
	m_saveThread = std::thread(&CStandardizedLogger::saveLogThreading);

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
	strRecentProductInfo.AppendFormat(_T("%s"), pLogData->strId);
	pRecentProductInfo->strFilePath = getLogFilePath(ESystemName::Minor,
	                                                 ELogFileType::RecentProductInfo,
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

void CStandardizedLogger::pushListLog(const CTime& tmLogTime, const CString& strThreadName)
{
	{
		std::unique_lock<std::mutex> lock(m_mtxThreadNameTable, std::defer_lock_t());
		if (lock.try_lock())
		{
			const auto findThreadName = m_tableThreadName.find(strThreadName);
			if (findThreadName == std::end(m_tableThreadName))
				m_tableThreadName.emplace(strThreadName);
		}
	}

	pushListLogInternal(tmLogTime);
}

void CStandardizedLogger::pushListLogInternal(const CTime& tmLogTime)
{
	const auto pListLogItem = std::make_shared<SListLogData>();
	auto& strListLogPath = pListLogItem->strFilePath;
	strListLogPath.Format(getLogFilePath(ESystemName::Minor, ELogFileType::ListLog, tmLogTime));
	pushLogData(pListLogItem);
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
			std::lock_guard<std::mutex> lock(m_mtxProductIdxTable);
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
                                            const CTime& tmLastProduct, const int nIndex) const
{
	CString strLogFilePath = _T("D:\\");
	switch (eLogType)
	{
		case ELogFileType::RecentProductInfo:
		case ELogFileType::MainThreadStatus:

			{
				strLogFilePath.AppendFormat(
					_T("LOG_SW\\%s.txt"), StandardizedLogging::SLogFileType::ToString(eLogType, nIndex));
			}
			break;

		case ELogFileType::ResultLog:
		case ELogFileType::AlarmLog:
		case ELogFileType::ProcessLog:
		case ELogFileType::SystemLog:
		case ELogFileType::ListLog:
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
				const CString strLogFileType = StandardizedLogging::SLogFileType::ToString(eLogType, -1);

				strLogFilePath.AppendFormat(
					_T("I_INSPID1_%s-VISION_%s_%s_%04d%02d%02d%02d.txt"), strSystemName, strLogFileType, m_strExeFileName,
					nYear,
					nMonth, nDay, nHour);
			}
			break;
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
			std::lock_guard<std::mutex> lock(m_mtxProductIdxTable);
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
	CString strDirPath = strFilePath;
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

	CStdioFile file;
	BOOL bOpen = file.Open(strFilePath, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::typeText);
	if (!bOpen)
		return false;
	
	file.SeekToEnd();
	file.WriteString(strFileData);
	file.Close();
	return true;
}

bool CStandardizedLogger::SListLogData::SaveToFile()
{
	const auto pLogger = CStandardizedLogger::GetInstance();
	std::unique_lock<std::mutex> lock(pLogger->m_mtxThreadNameTable, std::defer_lock_t());
	if (lock.try_lock())
	{
		const std::set<CString>& tableThreadName = pLogger->m_tableThreadName;
		CStdioFile file;
		BOOL bOpen = file.Open(strFilePath, CFile::modeCreate | CFile::typeText | CFile::modeWrite);
		if (!bOpen)
			return false;

		for (const CString& threadName : tableThreadName)
		{
			CString strData = threadName;
			strData += _T("\n");
			file.WriteString(strData);
		}

		file.Close();
	}
	
	return true;
}

void CharDeleter(const char* ptr)
{
	delete[] ptr;
}

CString StandardizedLogging::SLogFileType::ToString(ELogFileType eLogFileType, int nIdx)
{
	switch (eLogFileType)
	{
		case ELogFileType::ProcessLog:
			return _T("PROCESS");

		case ELogFileType::SystemLog:
			return _T("SYSTEM");

		case ELogFileType::AlarmLog:
			return _T("ALARM");

		case ELogFileType::ResultLog:
			return _T("RESULT");

		case ELogFileType::ListLog:
			return _T("LIST");

		case ELogFileType::RecentProductInfo:
			return _T("RecentProductInfo");

		case ELogFileType::MainThreadStatus:
			{
				if (nIdx == -1)
					return _T("Index Error");

				CString strName;
				strName.Format(_T("MAIN-THREAD-STATUS-%02d"), nIdx + 1);
				return  strName;
			}

		default:
			return _T("LOG FILE TYPE NOT DEFINED");
	}
}

bool CStandardizedLogger::SFileData::SaveToFile()
{
	CString strDirPath = strFilePath;
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

	CStdioFile file;
	BOOL bOpen = file.Open(strFilePath, CFile::modeWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::typeText);
	if (!bOpen)
		return false;

	file.SeekToEnd();
	file.WriteString(strFileData);
	file.Close();
	
	return true;
}

bool CStandardizedLogger::SMainThreadStatusLog::SaveToFile()
{
	//ToDo MainLoopStatus Data
	const auto pLogger = GetInstance();
	strFilePath = pLogger->getLogFilePath(ESystemName::Minor,
	                                      ELogFileType::MainThreadStatus, CTime::GetCurrentTime(), nMainThreadIdx);
	
	{
		std::lock_guard<std::mutex> lock(pLogger->m_mtxMainThreadStatus);
		CStdioFile file;
		BOOL bOpen = file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		if (!bOpen)
			return false;

		std::map<int, std::shared_ptr<SMainThreadStatus>>& tableMainThreadStatus = pLogger->m_tableMainThreadStatus;
		const auto item = tableMainThreadStatus.find(nMainThreadIdx);
		if (item == std::end(tableMainThreadStatus))
		{
			file.Close();
			return false;
		}
		
		auto pStatus = item->second;
		const int nMainThreadIdxDisplay = item->first + 1;
		CString strMainThreadStatus;
		strMainThreadStatus.Format(_T("MAIN-THREAD-STATUS-%d,%d,%s\n"), nMainThreadIdxDisplay,nProductCount,
		                           SMainThreadStatus::ToString(pStatus->eStatus));
		file.WriteString(strMainThreadStatus);
		file.Close();
	}

	return true;
}

bool CStandardizedLogger::SRecentProductInfoData::SaveToFile()
{
	CString strDirPath = strFilePath;
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

	CStdioFile file;
	BOOL bOpen = file.Open(strFilePath, CFile::modeWrite | CFile::typeText | CFile::modeCreate );
	if (!bOpen)
		return false;

	file.WriteString(strFileData);
	file.Close();

	return true;
}

