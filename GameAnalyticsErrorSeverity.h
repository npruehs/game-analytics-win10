#pragma once

#include <string>

namespace GameAnalytics
{
	namespace Severity
	{
		enum Severity
		{
			Critical,
			Error,
			Warning,
			Info,
			Debug
		};

		inline std::wstring ToWString(const Severity severity)
		{
			switch (severity)
			{
			case Critical:
				return L"critical";

			case Error:
				return L"error";

			case Warning:
				return L"warning";

			case Info:
				return L"info";

			case Debug:
				return L"debug";
			}

			// Unknown severity value.
			auto message = L"Unknown severity: " + std::to_wstring(severity);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}