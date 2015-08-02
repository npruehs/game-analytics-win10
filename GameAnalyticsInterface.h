#pragma once

#include <map>
#include <memory>
#include <string>

#include "GameAnalyticsErrorSeverity.h"
#include "GameAnalyticsProgressionStatus.h"
#include "GameAnalyticsReceiptInfo.h"
#include "GameAnalyticsResourceFlowType.h"
#include "GameAnalyticsUserData.h"

using namespace concurrency;

using namespace Windows::Data::Json;

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

		// Should be called when a new session starts.
		// Determines if the SDK should be disabled and gets the server timestamp otherwise.
		// That timestamp is used to calculate an offset, if client clock is not configured correctly. 
		task<JsonObject^> GameAnalyticsInterface::Init();

		bool IsInitialized() const;

		// Sends the business event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Purchase:RocketLauncher".
		// Check http://support.gameanalytics.com/hc/en-us/articles/200841576-Supported-currencies for a list of currencies that will populate the monetization dashboard.
		// For all other virtual currency strings, you will need to create your custom dashboards and widgets.
		// The amount is a numeric value which corresponds to the cost of the purchase in the monetary unit multiplied by 100.
		// For example, if the currency is "USD", the amount should be specified in cents.
		void SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount) const;

		// Sends the business event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Purchase:RocketLauncher".
		// Check http://support.gameanalytics.com/hc/en-us/articles/200841576-Supported-currencies for a list of currencies that will populate the monetization dashboard.
		// For all other virtual currency strings, you will need to create your custom dashboards and widgets.
		// The amount is a numeric value which corresponds to the cost of the purchase in the monetary unit multiplied by 100.
		// For example, if the currency is "USD", the amount should be specified in cents.
		// Includes a string representing the cart (the location) from which the purchase was made, i.e. menu_shop or end_of_level_shop.
		void SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const std::wstring & cartType) const;

		// Sends the business event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Purchase:RocketLauncher".
		// Check http://support.gameanalytics.com/hc/en-us/articles/200841576-Supported-currencies for a list of currencies that will populate the monetization dashboard.
		// For all other virtual currency strings, you will need to create your custom dashboards and widgets.
		// The amount is a numeric value which corresponds to the cost of the purchase in the monetary unit multiplied by 100.
		// For example, if the currency is "USD", the amount should be specified in cents.
		// Includes a JSON object that contains a receipt and an optional signature. Used for payment validation of receipts.
		void SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const ReceiptInfo & receiptInfo) const;

		// Sends the business event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Purchase:RocketLauncher".
		// Check http://support.gameanalytics.com/hc/en-us/articles/200841576-Supported-currencies for a list of currencies that will populate the monetization dashboard.
		// For all other virtual currency strings, you will need to create your custom dashboards and widgets.
		// The amount is a numeric value which corresponds to the cost of the purchase in the monetary unit multiplied by 100.
		// For example, if the currency is "USD", the amount should be specified in cents.
		// Includes a string representing the cart (the location) from which the purchase was made, i.e. menu_shop or end_of_level_shop.
		// Includes a JSON object that contains a receipt and an optional signature. Used for payment validation of receipts.
		void SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const std::wstring & cartType, const ReceiptInfo & receiptInfo) const;

		// Sends the design event with the specified id to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "PickedUpAmmo:Shotgun".
		void SendDesignEvent(const std::wstring & eventId) const;

		// Sends the design event with the specified id and value to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "PickedUpAmmo:Shotgun".
		void SendDesignEvent(const std::wstring & eventId, const float value) const;

		// Sends the error event with the specified message and severity to the GameAnalytics backend.
		// Event ids can be sub-categorized by using ":" notation, for example "Exception:NullReference".
		void SendErrorEvent(const std::wstring & message, const Severity::Severity severity) const;

		// Sends the progression event with the specified status to the GameAnalytics backend.
		// Progress event id can consist of 1-3 parts: Progression1:Progression2:Progression3.
		void SendProgressionEvent(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId) const;

		// Sends the progression event with the specified status to the GameAnalytics backend.
		// Progress event id can consist of 1-3 parts: Progression1:Progression2:Progression3.
		// Includes player score for the attempt. Use with status "Fail" or "Complete" only.
		void SendProgressionEvent(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId, const int score) const;

		// Sends the resource event with the specified flow type and currency and item data to the GameAnalytics backend.
		void SendResourceEvent(const FlowType::FlowType flowType, const std::wstring & ingameCurrency, const std::wstring & itemType, const std::wstring & itemId, float amount) const;

		// Sends the session end event to the GameAnalytics backend.
		// Should always be sent whenever a session is determined to be over, for example whenever the app is suspended.
		// Should be sent exactly once per session.
		void SendSessionEndEvent() const;

		// Sends the user event with the specified data to the GameAnalytics backend.
		void SendUserEvent(const User & user) const;

		void SetBirthYear(const int birthYear);

		// Sets the current version of the game being played. Defaults to the app package version.
		void SetBuild(const std::wstring & build);

		void SetFacebookId(const std::wstring & facebookId);

		void SetGender(const Gender::Gender gender);

		void SetGooglePlusId(const std::wstring & googlePlusId);

		// Sets the unique ID representing the user playing the game.
		// This ID should remain the same across different play sessions.
		// Defaults to the ASHWID.
		void SetUserId(const std::wstring & userId);
		
	private:
		bool initialized;
		long serverTimestamp;
		LARGE_INTEGER initializationTime;

		Windows::Web::Http::HttpClient^ httpClient;

		std::wstring gameKey;
		std::wstring secretKey;

		std::wstring build;
		std::wstring sessionId;
		std::wstring userId;

		std::shared_ptr<User> user;

		// Builds the event object for business analytics events.
		JsonObject^ BuildBusinessEventObject(const std::wstring & eventId, const std::wstring & currency, const int amount) const;

		// Builds an event object, adding GameAnalytics default annotations.
		JsonObject^ BuildEventObject(const std::wstring & category) const;

		// Builds the event object for design analytics events.
		JsonObject^ BuildDesignEventObject(const std::wstring & eventId) const;

		// Builds the event object for progression analytics events.
		JsonObject^ BuildProgressionEventObject(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId) const;

		// Builds an receipt info object, as used by some business events.
		JsonObject^ BuildReceiptObject(const ReceiptInfo & receiptInfo) const;

		// Generates a new GUID for the current session.
		std::wstring GenerateSessionId() const;

		// Gets the app package version.
		std::wstring GetAppVersion() const;

		// Gets the model of the device this app runs on.
		std::wstring GetDeviceModel() const;

		// Gets the Application Specific Hardware Identifier (ASHWID).
		// See https://msdn.microsoft.com/en-us/library/windows/apps/jj553431
		std::wstring GetHardwareId() const;

		// Gets the manufacturer of the device this app runs on.
		std::wstring GetManufacturer() const;

		// Gets the version of the operation systems this app runs on.
		std::wstring GetOSVersion() const;

		// Gets the platform this app runs on.
		std::wstring GetPlatform() const;

		// Gets the number of the current session.
		int GetSessionNumber() const;

		// Gets the version of this GameAnalytics SDK.
		std::wstring GetSDKVersion() const;

		// Get the elapsed time since initialization, in seconds.
		uint64 GetTimeSinceInit() const;

		// Gets the number of the current transaction.
		int GetTransactionNumber() const;

		JsonValue^ ToJsonValue(std::wstring s) const;
		JsonValue^ ToJsonValue(double d) const;

		// Sends the specified event to the GameAnalytics backend.
		task<JsonObject^> SendGameAnalyticsEvent(const std::wstring & route, JsonObject^ eventObject) const;
	};
}