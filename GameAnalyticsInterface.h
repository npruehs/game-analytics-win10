#pragma once

#include <map>
#include <memory>
#include <string>

namespace GameAnalytics
{
	class GameAnalyticsInterface
	{
	public:
		// Initializes a new instance of the GameAnalytics interface
		// for the game with the specified game key and secret key.
		// Uses the app package version as build id,
		// uses the Application Specific Hardware Identifier (ASHWID) as user id,
		// and generates a new GUID for the session.
		GameAnalyticsInterface(const std::wstring & gameKey, const std::wstring & secretKey);

		// Sends the business event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Purchase:RocketLauncher".
		void SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const float amount) const;

		// Sends the design event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "PickedUpAmmo:Shotgun".
		void SendDesignEvent(const std::wstring & eventId) const;

		// Sends the design event with the specified id and value to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "PickedUpAmmo:Shotgun".
		void SendDesignEvent(const std::wstring & eventId, const float value) const;

		// Sets the area to be associated with each subsequent design, error and business event.
		void SetArea(const std::wstring & area);

		// Sets the current version of the game being played. Defaults to the app package version.
		void SetBuild(const std::wstring & build);

		// Sets the unique ID representing the user playing the game.
		// This ID should remain the same across different play sessions.
		// Defaults to the ASHWID.
		void SetUserId(const std::wstring & userId);
		
	private:
		Windows::Web::Http::HttpClient^ httpClient;

		std::wstring gameKey;
		std::wstring secretKey;

		std::wstring area;
		std::wstring build;
		std::wstring sessionId;
		std::wstring userId;

		// Builds the event parameter map for design analytics events.
		std::map<std::wstring, std::wstring> BuildDesignParameterMap(const std::wstring & eventId) const;

		// Generates a new GUID for the current session.
		std::wstring GenerateSessionId() const;

		// Gets the app package version.
		std::wstring GetAppVersion() const;

		// Gets the Application Specific Hardware Identifier (ASHWID).
		// See https://msdn.microsoft.com/en-us/library/windows/apps/jj553431
		std::wstring GetHardwareId() const;
		
		// Sends the event with the specified category and parameters to the GameAnalytics backend.
		void SendGameAnalyticsEvent(const std::wstring & category, const std::map<std::wstring, std::wstring> & parameters) const;
	};
}