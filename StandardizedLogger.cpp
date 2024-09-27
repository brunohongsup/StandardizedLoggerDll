#include "pch.h"
#include "StandardizedLogger.h"
#include "StandardizedLoggerImpl.h"

namespace StandardizedLogging
{
	CCriticalSection CStandardizedLogger::s_lockSection;
	std::shared_ptr<CStandardizedLogger> CStandardizedLogger::s_instance = nullptr;

	CStandardizedLogger::CStandardizedLogger()
		: m_pImpl(nullptr)
	{
		m_pImpl = std::make_unique<CStandardizedLoggerImpl>();
		m_pImpl->init();
	}

	std::shared_ptr<CStandardizedLogger> CStandardizedLogger::GetInstance()
	{

		if(s_instance == nullptr)
		{
			CSingleLock lock(&s_lockSection);
			lock.Lock();
			if(s_instance == nullptr)
				s_instance = std::shared_ptr<CStandardizedLogger>(new CStandardizedLogger());
		}

		return s_instance;
	}
}
