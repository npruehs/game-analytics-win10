#pragma once

#include <string>

namespace GameAnalytics
{
	namespace FlowType
	{
		enum FlowType
		{
			Sink,
			Source
		};

		inline std::wstring ToWString(const FlowType flowType)
		{
			switch (flowType)
			{
			case Sink:
				return L"Sink";

			case Source:
				return L"Source";
			}

			// Unknown flow type.
			auto message = L"Unknown flow type: " + std::to_wstring(flowType);
			auto messageString = ref new Platform::String(message.c_str());
			throw ref new Platform::FailureException(messageString);
		}
	}
}