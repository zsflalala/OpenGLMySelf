#pragma once

#include <chrono>
#include <iostream>

namespace validation_utils
{
	class CTimer
	{
	public:
		CTimer() :m_StartTimePoint(std::chrono::high_resolution_clock::now())
		{
		}

		void setMaxRecordCount(size_t vMaxRecordCount) { m_MaxRecordCount = vMaxRecordCount; }

		void reset()
		{
			m_StartTimePoint = std::chrono::high_resolution_clock::now();
			m_TotalDeltaTimes = 0.0f;
			m_CurrRecordCount = 0;
		}

		void start()
		{
			m_StartTimePoint = std::chrono::high_resolution_clock::now();
		}

		double getDeltaTime()
		{
			const auto Delta = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_StartTimePoint);
			const double DeltaTime = static_cast<double>(Delta.count()) / 1e6;
			m_TotalDeltaTimes += DeltaTime;
			m_CurrRecordCount += 1;
			__printAvgDeltaTime();
			return DeltaTime;
		}

		[[nodiscard]] double getAverageDeltaTime() const
		{
			if (m_CurrRecordCount == 0)
				return 0.0;
			return m_TotalDeltaTimes / static_cast<double>(m_CurrRecordCount);
		}

	private:
		void __printAvgDeltaTime()
		{
			if (m_CurrRecordCount < m_MaxRecordCount) return;
			std::cout << "[Timer] Run " << m_CurrRecordCount << " times, Avg Cost: " << getAverageDeltaTime() << "ms\n";
			reset();
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
		double m_TotalDeltaTimes = 0.0;
		std::uint32_t m_CurrRecordCount = 0;
		size_t m_MaxRecordCount = 1000;
	};
}
