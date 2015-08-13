#pragma once

#include <string>

namespace GameAnalytics
{
	namespace ProgressionStatus
	{
		enum ProgressionStatus
		{
			Start,
			Fail,
			Complete
		};

		inline std::wstring ToWString(const ProgressionStatus status)
		{
			switch (status)
			{
			case Start:
				return L"Start";

			case Fail:
				return L"Fail";

			case Complete:
				return L"Complete";
			}

			// Unknown progression status.
			auto message = L"Unknown status: " + std::to_wstring(status);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}