#pragma once
#include <memory>
#include <vector>
#include <queue>
#include <Shlwapi.h>
#include <cassert>
#include <afxmt.h>
#include <afxwin.h>

class CStandardizedLoggerImpl;

class CStandardizedLoggerPrivate
{
public:
	virtual ~CStandardizedLoggerPrivate() = 0;
	CStandardizedLoggerPrivate()
	{

	}

private:
	friend class CStandardizedLogger;


	virtual void PushProcLogMainThreadMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1) = 0;

	virtual void PushProcLogMainThreadMainLoopEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) = 0;

	virtual void SetVisionSystemMajorName(const CString& strMachineName) = 0;

	virtual void SetVisionSystemMinorName(const CString& strMachineName) = 0;

	virtual void PushProcLogCameraThreadInterfaceInGrab(const int nProductCount, const CString& strProductId, const int nCameraThreadIdx = 1) = 0;

	virtual void PushProcLogCameraThreadSyncOutToMainThread(const int nProductCount, const CString& strProductId, const int CameraIdx, const int nMainThreadIdx) = 0;

	virtual void PushProcLogMainThreadSyncInFromCamreaThread(const int& nProductCount, const int& nMainThreadIdx, const int& nCameraThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadSyncOutToImgProcThread(const int nProductCount, const int nMainThreadIdx, const int nImgProcThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadSyncInFromImgProcThread(const int nProductCount, const int& nMainThreadIdx, const int& nImgProcThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadEndJudgementResult(const int nProductCount, const int& nMainThreadIdx, const CString& strProductId) = 0;

	virtual bool init() = 0;

	virtual void Clear() = 0;

	virtual void PushProcLogMainThreadInterfaceInFromCellTrackerNoPostTag(const int nProductCount, const int nMainThreadIdx = 1) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx = 1) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx = 1) = 0;


	virtual void PushProcLogMainThreadSyncOutToSaveImgThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx, const int nSaveImgThreadIdx) = 0;

	virtual void PushProcLogMainThreadSyncOutToSaveDataThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx, const int nSaveImgThreadIdx) = 0;

	virtual void PushProcLogMainThreadSyncOutToSaveProcLogThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;

	virtual void PushProcLogSaveImgThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx) = 0;

	virtual void PushProcLogSaveDateThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx) = 0;

	virtual void PushProcLogSaveImgThreadSaveImgThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) = 0;

	virtual void PushProcLogSaveImgThreadSaveImgThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) = 0;


	virtual void PushProcLogSaveImgThreadSaveSpcImgThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) = 0;

	virtual void PushProcLogSaveImgThreadSaveSpcImgThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) = 0;

	virtual void PushProcLogSaveDataThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx, const int nMainThreadIdx) = 0;

	virtual void PushProcLogSaveDataThreadSaveDataThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1) = 0;

	virtual void PushProcLogSaveDataThreadSaveDataThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx) = 0;

	virtual void PushProcLogSaveDataThreadSaveSpcDataThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx) = 0;

	virtual void PushProcLogSaveDataThreadSaveSpcDataThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx) = 0;

	virtual void PushProcLogCellTrackingCallbackEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;

	virtual void PushSystemLogSaveProcLogThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;

	virtual void PushSystemLogSaveProcThreadSaveProcessLogThreadStartPreTag(const int nProductCount, const CString& strProductId) = 0;

	virtual void PushSystemLogSaveProcThreadSaveProcessLogThreadEndPreTag(const int nProductCount, const CString& strProductId) = 0;

	virtual void PushSystemLogSaveEtcThreadHeartBitOut(const int nProductCount, const CString& strProductId) = 0;

	virtual void PushResultLog(const int nProductCount, const CString& strModuelId, const CString& strCellId, const CString& strImagePath, const std::vector<CString>& vctEtcLog) = 0;

	virtual void PushAlarmLogCellTrackingInputFailOn(const int nProductCount, const CString& strProductId) = 0;

	virtual void PushAlarmLogGrabFail(const int nProductCount, const CString& strProductId, const int nCameraIdx) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCResetOn(const int nProductCount, const int nMainThreadId, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCResetOff(const int nProductCount, const int nMainThreadId, const CString& strProductId) = 0;
		//PLC -> VISION RESET OFF
	virtual void PushProcLogMainThreadInterfaceOutToPLCResetOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;


	virtual void PushProcLogMainThreadInterfaceOutToPLCResetOff(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadSyncInFrom3DCamreaThread(const int nProductCount, const int& nMainThreadIdx, const int& n3DCamreaIdx, const CString& strProductId) = 0;

	virtual void PushProcLog3DCameraThreadSyncOutToMainThread(const int nProductCount, const int n3DCameraThreadIdx, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCStepNumber(const int nProductCount, const int nMainThreadIdx, const int nStepNumber, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOff(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadCheckStepNumber(const int nProductCount, const int nMainThreadIdx, const CString& strProductId, const int nStepNumber) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCTriggerOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCTriggerOff(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToTriggerEnable(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPLCFinishOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPLCFinishOff(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;


	virtual void PushProcLogMainThreadInterfaceInFromPLCResultAckOn(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;

	virtual void PushProcLogMainThreadInterfaceInFromPLCResultAckOff(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPlcResultOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPlcResultOff(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) = 0;

	virtual void PushProcLogMainThreadInterfaceOutToPLCResetAckOn(const int nProductCount, const int nMainThreadId, const CString& strProductId) = 0;

	virtual void PushAlaramLogOperatorStopButton(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) = 0;


protected:

};

class __declspec(dllexport) CStandardizedLogger : CObject
{
private:


public:

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

	void PushProcLogMainThreadMainLoopStart(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadMainLoopStart(nProductCount, nMainThreadIdx);
	}

	void PushProcLogMainThreadMainLoopEnd(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLogMainThreadMainLoopEnd(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushProcLogCameraThreadInterfaceInGrab(const int nProductCount, const int& nCameraThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogCameraThreadInterfaceInGrab(nProductCount, strProductId, nCameraThreadIdx);
	}

	void PushProcLogCameraThreadSyncOutToMainThread(const int nProductCount, const int nCameraIdx = 1, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogCameraThreadSyncOutToMainThread(nProductCount, strProductId, nCameraIdx, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceInFromPLCResetOn(const int nProductCount, const int nMainThreadId = 1, const CString& strProductId = _T("Null"))
	{
		//PLC -> VISION RESET ON
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCResetOn(nProductCount, nMainThreadId, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCResetAckOn(const int nProductCount, const int nMainThreadId = 1, const CString& strProductId = _T("Null"))
	{
		//PLC -> VISION RESET ON
	}


	void PushProcLogMainThreadInterfaceInFromPLCResetOff(const int nProductCount, const int nMainThreadId = 1, const CString& strProductId = _T("Null"))
	{
		//PLC -> VISION RESET OFF
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCResetOff(nProductCount, nMainThreadId, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCResetOn(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		//VISION -> PLC RESET ON
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCResetOn(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCResetOff(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		//VISION -> PLC RESET OFF
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCResetOff(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOn(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCVisionReadyOn(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCVisionReadyOff(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCVisionReadyOff(nProductCount, nMainThreadIdx, strProductId);
	}


	void PushProcLogMainThreadInterfaceInFromPLCStepNumber(const int nProductCount, const int nMainThreadIdx = 1, const int nStepNumber = 1, const CString& strProductId = _T("Null"))
	{
		//PLC -> VISION Step Number
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCStepNumber(nProductCount, nMainThreadIdx, nStepNumber, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		//m_pImpl->PushProcLogMainThreadInterfaceOutToEncoderResetFunctionStartPostTag(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToEncoderResetFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		//m_pImpl->PushProcLogMainThreadInterfaceOutToEncoderResetFunctionEndPostTag(nProductCount, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceInFromPLCTriggerOn(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCTriggerOn(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToTriggerEnable(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToTriggerEnable(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceInFromPLCTriggerOff(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCTriggerOff(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceInFromPLCResultAckOn(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCResultAckOn(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceInFromPLCResultAckOff(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromPLCResultAckOff(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceOutToPlcResultOn(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPlcResultOn(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPlcResultOff(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPlcResultOff(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadSyncInFromCamreaThread(const int nProductCount, const int& nMainThreadIdx = 1, const int& nCameraThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLogMainThreadSyncInFromCamreaThread(nProductCount, nMainThreadIdx, nCameraThreadIdx, strProductId);
	}

	void PushProcLogMainThreadSyncInFrom3DCamreaThread(const int nProductCount, const int& nMainThreadIdx = 1, const int& n3DCamreaIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLogMainThreadSyncInFrom3DCamreaThread(nProductCount, nMainThreadIdx, n3DCamreaIdx, strProductId);
	}

	void PushProcLog3DCameraThreadSyncOutToMainThread(const int nProductCount, const int& n3DCameraThreadIdx = 1, const int& nMainThreadIdx = 1, const CString& strProductId = _T("Null"))
	{
		m_pImpl->PushProcLog3DCameraThreadSyncOutToMainThread(nProductCount, n3DCameraThreadIdx, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadSyncOutToImgProcThread(const int nProductCount, const int& nMainThreadIdx = 1, const int& nImgProcThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadSyncOutToImgProcThread(nProductCount, nMainThreadIdx, nImgProcThreadIdx, strProductId);
	}

	void PushProcLogMainThreadSyncInFromImgProcThread(const int nProductCount, const int& nMainThreadIdx = 1, const int& nImgProcThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadSyncInFromImgProcThread(nProductCount, nMainThreadIdx, nImgProcThreadIdx, strProductId);
	}

	void PushProcLogMainThreadEndJudgementResult(const int nProductCount, const int& nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadEndJudgementResult(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCFinishOn(const int nProductCount, const int nMainThreadIdx, const CString& strProductId) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCFinishOn(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceOutToPLCFinishOff(const int nProductCount, const int nMainThreadIdx = 1, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadInterfaceOutToPLCFinishOff(nProductCount, nMainThreadIdx, strProductId);
	}

	void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionStartPostTag(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromCellTrackerFunctionStartPostTag(nProductCount, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceInFromCellTrackerFunctionEndPostTag(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromCellTrackerFunctionEndPostTag(nProductCount, nMainThreadIdx);
	}

	void PushProcLogMainThreadInterfaceInFromCellTrackerNoPostTag(const int nProductCount, const int nMainThreadIdx = 1)
	{
		m_pImpl->PushProcLogMainThreadInterfaceInFromCellTrackerNoPostTag(nProductCount, nMainThreadIdx);
	}

	void PushProcLogMainThreadSyncOutToSaveImgThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1, const int nSaveImgThreadIdx = 1) const
	{
		m_pImpl->PushProcLogMainThreadSyncOutToSaveImgThread(nProductCount, strProductId, nMainThreadIdx, nSaveImgThreadIdx);
	}

	void PushProcLogMainThreadSyncOutToSaveDataThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1, const int nSaveImgThreadIdx = 1) const
	{
		m_pImpl->PushProcLogMainThreadSyncOutToSaveDataThread(nProductCount, strProductId, nMainThreadIdx, nSaveImgThreadIdx);
	}

	void PushProcLogMainThreadSyncOutToSaveProcLogThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) const
	{
		m_pImpl->PushProcLogMainThreadSyncOutToSaveProcLogThread(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushProcLogSaveImgThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx) const
	{
		m_pImpl->PushProcLogSaveImgThreadSyncInFromMainThread(nProductCount, strProductId, nSaveImgThreadIdx, nMainThreadIdx);
	}

	void PushProcLogSaveDateThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nMainThreadIdx) const
	{
		m_pImpl->PushProcLogSaveDataThreadSyncInFromMainThread(nProductCount, strProductId, nSaveImgThreadIdx, nMainThreadIdx);
	}

	void PushProcLogSaveImgThreadSaveImgThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) const
	{
		m_pImpl->PushProcLogSaveImgThreadSaveImgThreadStartPreTag(nProductCount, strProductId, nSaveImgThreadIdx, nCameraThreadIdx);
	}

	void PushProcLogSaveImgThreadSaveImgThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) const
	{
		m_pImpl->PushProcLogSaveImgThreadSaveImgThreadEndPreTag(nProductCount, strProductId, nSaveImgThreadIdx, nCameraThreadIdx);
	}

	void PushProcLogSaveImgThreadSaveSpcImgThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx, const int nCameraThreadIdx) const
	{
		m_pImpl->PushProcLogSaveImgThreadSaveSpcImgThreadStartPreTag(nProductCount, strProductId, nSaveImgThreadIdx, nCameraThreadIdx);
	}

	void PushProcLogSaveImgThreadSaveSpcImgThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveImgThreadIdx = 1, const int nCameraThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveImgThreadSaveSpcImgThreadEndPreTag(nProductCount, strProductId, nSaveImgThreadIdx, nCameraThreadIdx);
	}

	void PushProcLogSaveDataThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1, const int nMainThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveDataThreadSyncInFromMainThread(nProductCount, strProductId, nSaveDataThreadIdx, nMainThreadIdx);
	}

	void PushProcLogSaveDataThreadSaveDataThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveDataThreadSaveDataThreadStartPreTag(nProductCount, strProductId, nSaveDataThreadIdx);
	}

	void PushProcLogSaveDataThreadSaveDataThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveDataThreadSaveDataThreadEndPreTag(nProductCount, strProductId, nSaveDataThreadIdx);
	}

	void PushProcLogMainThreadCheckStepNumber(const int nProductCount, const int nMainThreadIdx, const int nStepNumber, const CString& strProductId = _T("Null")) const
	{
		m_pImpl->PushProcLogMainThreadCheckStepNumber(nProductCount, nMainThreadIdx, strProductId, nStepNumber);
	}

	void PushProcLogSaveDataThreadSaveSpcDataThreadStartPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveDataThreadSaveSpcDataThreadStartPreTag(nProductCount, strProductId, nSaveDataThreadIdx);
	}

	void PushProcLogSaveDataThreadSaveSpcDataThreadEndPreTag(const int nProductCount, const CString& strProductId, const int nSaveDataThreadIdx = 1) const
	{
		m_pImpl->PushProcLogSaveDataThreadSaveSpcDataThreadEndPreTag(nProductCount, strProductId, nSaveDataThreadIdx);
	}

	void PushProcLogCellTrackingCallbackEnd(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) const
	{
		m_pImpl->PushProcLogCellTrackingCallbackEnd(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushSystemLogSaveProcLogThreadSyncInFromMainThread(const int nProductCount, const CString& strProductId, const int nMainThreadIdx) const
	{
		m_pImpl->PushSystemLogSaveProcLogThreadSyncInFromMainThread(nProductCount, strProductId, nMainThreadIdx);
	}

	void PushSystemLogSaveProcThreadSaveProcessLogThreadStartPreTag(const int nProductCount, const CString& strProductId) const
	{
		m_pImpl->PushSystemLogSaveProcThreadSaveProcessLogThreadStartPreTag(nProductCount, strProductId);
	}

	void PushSystemLogSaveProcThreadSaveProcessLogThreadEndPreTag(const int nProductCount, const CString& strProductId) const
	{
		m_pImpl->PushSystemLogSaveProcThreadSaveProcessLogThreadEndPreTag(nProductCount, strProductId);
	}

	void PushSystemLogSaveEtcThreadHeartBitOut(const int nProductCount, const CString& strProductId) const
	{
		m_pImpl->PushSystemLogSaveEtcThreadHeartBitOut(nProductCount, strProductId);
	}

	void PushResultLog(const int nProductCount, const CString& strModuleId, const CString& strCellId, const CString& strImagePath, const std::vector<CString>& vctEtcLogs) const
	{
		m_pImpl->PushResultLog(nProductCount, strModuleId, strCellId, strImagePath, vctEtcLogs);
	}

	void PushAlarmLogCellTrackingInputFailOn(const int nProductCount, const CString& strProductId) const
	{
		m_pImpl->PushAlarmLogCellTrackingInputFailOn(nProductCount, strProductId);
	}

	void PushAlarmLogGrabFail(const int nProductCount, const CString& strProductId, const int nCameraIdx) const
	{
		m_pImpl->PushAlarmLogGrabFail(nProductCount, strProductId, nCameraIdx);
	}

	void PushAlaramLogOperatorStopButton(const int nProductCount, const CString& strProductId, const int nMainThreadIdx = 1) const
	{
		m_pImpl->PushAlaramLogOperatorStopButton(nProductCount, strProductId, nMainThreadIdx);
	}

	static std::shared_ptr<CStandardizedLogger> GetInstance();
	CStandardizedLogger();

private:
	static std::shared_ptr<CStandardizedLogger> s_instance;
	static CCriticalSection s_lockSection;

	std::unique_ptr<CStandardizedLoggerPrivate> m_pImpl;

	CStandardizedLogger(const CStandardizedLogger&) = delete;

};
