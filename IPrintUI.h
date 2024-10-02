#pragma once
#include "pch.h"

class IPrintUI
{
public:
	virtual void StartCycle() = 0;
	virtual void EndCycle() = 0;
	virtual void SetCycleTimeLog(const CString& strContent) = 0;
	virtual CString CalcCycleTime() = 0;

private:

};