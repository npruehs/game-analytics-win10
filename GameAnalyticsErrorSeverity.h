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
				return L"Critical";

			case Error:
				return L"Error";

			case Warning:
				return L"Warning";

			case Info:
				return L"Info";

			case Debug:
				return L"Debug";
			}

			// Unknown severity value.
			auto message = L"Unknown severity: " + std::to_wstring(severity);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}