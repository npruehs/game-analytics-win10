#include "pch.h"

#include "GameAnalyticsInterface.h"

#include <ppltasks.h>

using namespace GameAnalytics;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Data::Json;
using namespace Windows::Foundation;
using namespace Windows::Security::Cryptography;
using namespace Windows::Security::Cryptography::Core;
using namespace Windows::Storage;
using namespace Windows::Web::Http;
using namespace Windows::Web::Http::Headers;


GameAnalyticsInterface::GameAnalyticsInterface(const std::wstring & gameKey, const std::wstring & secretKey)
	: initialized(false), 
	httpClient(ref new Windows::Web::Http::HttpClient()),
	gameKey(gameKey),
	secretKey(secretKey),
	build(this->GetAppVersion()),
	sessionId(this->GenerateSessionId()),
	userId(this->GetHardwareId()),
	user(std::make_shared<User>())
{
}

task<JsonObject^> GameAnalyticsInterface::Init()
{
	// Increase session counter.
	auto localSettings = ApplicationData::Current->LocalSettings;
	auto hasSessionCounter = localSettings->Values->HasKey("GameAnalytics::Session");

	if (hasSessionCounter)
	{
		this->sessionNumber = safe_cast<IPropertyValue^>(localSettings->Values->Lookup("GameAnalytics::Session"))->GetInt32();
	}
	else
	{
		this->sessionNumber = 0;
	}

	++this->sessionNumber;
	
	localSettings->Values->Insert("GameAnalytics::Session", dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(this->sessionNumber)));

	// Build event object.
	auto jsonObject = ref new JsonObject();

	jsonObject->Insert(L"platform", this->ToJsonValue(this->GetPlatform()));
	jsonObject->Insert(L"os_version", this->ToJsonValue(this->GetOSVersion()));
	jsonObject->Insert(L"sdk_version", this->ToJsonValue(this->GetSDKVersion()));

	// Send event.
	return this->SendGameAnalyticsEvent(L"init", jsonObject).then([this](JsonObject^ response)
	{
		// Verify response.
		auto enabled = response->GetNamedBoolean(L"enabled");

		if (!enabled)
		{
			auto message = L"Error initializing GameAnalytics.";
			auto messageString = ref new Platform::String(message);
			throw ref new Platform::FailureException(messageString);
		}

		// Set server timestamp.
		this->initialized = true;
		this->serverTimestamp = response->GetNamedNumber(L"server_ts");

		if (!QueryPerformanceCounter(&this->initializationTime))
		{
			throw ref new Platform::FailureException("Unable to get system time.");
		}

		return response;
	});
}

bool GameAnalyticsInterface::IsInitialized() const
{
	return this->initialized;
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount) const
{
	// Build event object.
	auto jsonObject = this->BuildBusinessEventObject(eventId, currency, amount);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const std::wstring & cartType) const
{
	// Build event object.
	auto jsonObject = this->BuildBusinessEventObject(eventId, currency, amount);
	jsonObject->Insert(L"cart_type", this->ToJsonValue(cartType));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const ReceiptInfo & receiptInfo) const
{
	// Build event object.
	auto jsonObject = this->BuildBusinessEventObject(eventId, currency, amount);
	auto receiptObject = this->BuildReceiptObject(receiptInfo);

	jsonObject->Insert(L"receipt_info", receiptObject);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendBusinessEvent(const std::wstring & eventId, const std::wstring & currency, const int amount, const std::wstring & cartType, const ReceiptInfo & receiptInfo) const
{
	// Build event object.
	auto jsonObject = this->BuildBusinessEventObject(eventId, currency, amount);
	auto receiptObject = this->BuildReceiptObject(receiptInfo);

	jsonObject->Insert(L"cart_type", this->ToJsonValue(cartType));
	jsonObject->Insert(L"receipt_info", receiptObject);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId) const
{
	// Build event object.
	auto jsonObject = this->BuildDesignEventObject(eventId);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendDesignEvent(const std::wstring & eventId, const float value) const
{
	// Build event object.
	auto jsonObject = this->BuildDesignEventObject(eventId);
	jsonObject->Insert(L"value", this->ToJsonValue(value));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendErrorEvent(const std::wstring & message, const Severity::Severity severity) const
{
	// Build event object.
	auto jsonObject = this->BuildEventObject(L"error");

	jsonObject->Insert(L"message", this->ToJsonValue(message));
	jsonObject->Insert(L"severity", this->ToJsonValue(Severity::ToWString(severity)));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendProgressionEvent(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId) const
{
	// Build event object.
	auto jsonObject = this->BuildProgressionEventObject(status, eventId);

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendProgressionEvent(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId, const int score) const
{
	// Build event object.
	auto jsonObject = this->BuildProgressionEventObject(status, eventId);
	jsonObject->Insert(L"score", this->ToJsonValue(score));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendResourceEvent(const FlowType::FlowType flowType, const std::wstring & ingameCurrency, const std::wstring & itemType, const std::wstring & itemId, float amount) const
{
	// Build event object.
	auto jsonObject = this->BuildEventObject(L"resource");

	auto eventId = ToWString(flowType) + L":" + ingameCurrency + L":" + itemType + L":" + itemId;
	jsonObject->Insert(L"event_id", this->ToJsonValue(eventId));
	jsonObject->Insert(L"amount", this->ToJsonValue(amount));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendSessionEndEvent() const
{
	// Build event object.
	auto jsonObject = this->BuildEventObject(L"session_end");
	jsonObject->Insert(L"length", this->ToJsonValue(this->GetTimeSinceInit()));

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SendUserEvent(const User & user) const
{
	// Store user data.
	if (user.birthYear >= 0)
	{
		this->user->birthYear = user.birthYear;
	}

	if (!user.facebookId.empty())
	{
		this->user->facebookId = user.facebookId;
	}

	if (user.gender != Gender::Unknown)
	{
		this->user->gender = user.gender;
	}

	if (!user.googlePlusId.empty())
	{
		this->user->googlePlusId = user.googlePlusId;
	}

	// Build event object.
	auto jsonObject = this->BuildEventObject(L"user");

	// Send event.
	this->SendGameAnalyticsEvent(L"events", jsonObject);
}

void GameAnalyticsInterface::SetBirthYear(const int birthYear)
{
	this->user->birthYear = birthYear;
}

void GameAnalyticsInterface::SetBuild(const std::wstring & build)
{
	this->build = build;
}

void GameAnalyticsInterface::SetFacebookId(const std::wstring & facebookId)
{
	this->user->facebookId = facebookId;
}

void GameAnalyticsInterface::SetGender(const Gender::Gender gender)
{
	this->user->gender = gender;
}

void GameAnalyticsInterface::SetGooglePlusId(const std::wstring & googlePlusId)
{
	this->user->googlePlusId = googlePlusId;
}

void GameAnalyticsInterface::SetUserId(const std::wstring & userId)
{
	this->userId = userId;
}

JsonObject^ GameAnalyticsInterface::BuildBusinessEventObject(const std::wstring & eventId, const std::wstring & currency, const int amount) const
{
	auto jsonObject = this->BuildEventObject(L"business");

	jsonObject->Insert(L"event_id", this->ToJsonValue(eventId));
	jsonObject->Insert(L"currency", this->ToJsonValue(currency));
	jsonObject->Insert(L"amount", this->ToJsonValue(amount));
	jsonObject->Insert(L"transaction_num", this->ToJsonValue(this->GetTransactionNumber()));

	return jsonObject;
}

JsonObject^ GameAnalyticsInterface::BuildEventObject(const std::wstring & category) const
{
	// Check if initialized.
	if (!this->initialized)
	{
		throw ref new Platform::FailureException("GameAnalytics has not been initialized yet. Call Init() first.");
	}

	// Build event object.
	auto jsonObject = ref new JsonObject();

	// Add category.
	jsonObject->Insert(L"category", this->ToJsonValue(category));

	// Add device model.
	jsonObject->Insert(L"device", this->ToJsonValue(this->GetDeviceModel()));

	// Add GameAnalytics API version.
	jsonObject->Insert(L"v", this->ToJsonValue(2));

	// Add user id.
	jsonObject->Insert(L"user_id", this->ToJsonValue(this->userId));

	// Add timestamp.
	auto clientTimestamp = this->serverTimestamp + this->GetTimeSinceInit();
	jsonObject->Insert(L"client_ts", this->ToJsonValue(clientTimestamp));

	// Add SDK version.
	jsonObject->Insert(L"sdk_version", this->ToJsonValue(this->GetSDKVersion()));

	// Add OS version.
	jsonObject->Insert(L"os_version", this->ToJsonValue(this->GetOSVersion()));

	// Add manufacturer.
	jsonObject->Insert(L"manufacturer", this->ToJsonValue(this->GetManufacturer()));

	// Add platform.
	jsonObject->Insert(L"platform", this->ToJsonValue(this->GetPlatform()));

	// Add session ID.
	jsonObject->Insert(L"session_id", this->ToJsonValue(this->sessionId));

	// Add session number.
	jsonObject->Insert(L"session_num", this->ToJsonValue(this->GetSessionNumber()));

	// Add Google+ id.
	if (!this->user->googlePlusId.empty())
	{
		jsonObject->Insert(L"googleplus_id", this->ToJsonValue(this->user->googlePlusId));
	}

	// Add Facebook id.
	if (!this->user->facebookId.empty())
	{
		jsonObject->Insert(L"facebook_id", this->ToJsonValue(this->user->facebookId));
	}

	// Add gender.
	if (this->user->gender != Gender::Unknown)
	{
		jsonObject->Insert(L"gender", this->ToJsonValue(Gender::ToWString(this->user->gender)));
	}

	// Add birthyear.
	if (this->user->birthYear >= 0)
	{
		jsonObject->Insert(L"birth_year", this->ToJsonValue(this->user->birthYear));
	}

	// Add build.
	jsonObject->Insert(L"build", this->ToJsonValue(this->build));

	return jsonObject;
}

JsonObject^ GameAnalyticsInterface::BuildDesignEventObject(const std::wstring & eventId) const
{
	auto jsonObject = this->BuildEventObject(L"design");
	jsonObject->Insert(L"event_id", this->ToJsonValue(eventId));

	return jsonObject;
}

JsonObject^ GameAnalyticsInterface::BuildProgressionEventObject(const ProgressionStatus::ProgressionStatus status, const std::wstring & eventId) const
{
	auto jsonObject = this->BuildEventObject(L"progression");

	auto eventIdString = ToWString(status) + L":" + eventId;
	jsonObject->Insert(L"event_id", this->ToJsonValue(eventIdString));

	return jsonObject;
}

JsonObject^ GameAnalyticsInterface::BuildReceiptObject(const ReceiptInfo & receiptInfo) const
{
	auto receiptObject = ref new JsonObject();

	receiptObject->Insert(L"receipt", this->ToJsonValue(receiptInfo.receipt));
	
	// TODO: Set correct store as soon as available in GameAnalytics.
	receiptObject->Insert(L"store", this->ToJsonValue(L"unknown"));

	if (!receiptInfo.signature.empty())
	{
		receiptObject->Insert(L"signature", this->ToJsonValue(receiptInfo.signature));
	}

	return receiptObject;
}

std::wstring GameAnalyticsInterface::GenerateSessionId() const
{
	GUID result;
	HRESULT hr = CoCreateGuid(&result);

	if (SUCCEEDED(hr))
	{
		// Generate new GUID.
		Guid guid(result);
		auto guidString = std::wstring(guid.ToString()->Data());

		// Remove curly brackets.
		auto sessionId = guidString.substr(1, guidString.length() - 2);
		return sessionId;
	}

	throw Exception::CreateException(hr);
}

std::wstring GameAnalyticsInterface::GetAppVersion() const
{
	auto thisPackage = Windows::ApplicationModel::Package::Current;
	auto version = thisPackage->Id->Version;
	return std::wstring(std::to_wstring(version.Major)
		+ L"." + std::to_wstring(version.Minor)
		+ L"." + std::to_wstring(version.Build)
		+ L"." + std::to_wstring(version.Revision));
}

std::wstring GameAnalyticsInterface::GetDeviceModel() const
{
	auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
	return std::wstring(info->SystemProductName->Data());
}

std::wstring GameAnalyticsInterface::GetHardwareId() const
{
	auto packageSpecificToken = Windows::System::Profile::HardwareIdentification::GetPackageSpecificToken(nullptr);
	auto hardwareId = packageSpecificToken->Id;
	auto hardwareIdString = CryptographicBuffer::EncodeToHexString(hardwareId);
	return std::wstring(hardwareIdString->Data());
}

std::wstring GameAnalyticsInterface::GetManufacturer() const
{
	auto info = ref new Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation();
	return std::wstring(info->SystemManufacturer->Data());
}


std::wstring GameAnalyticsInterface::GetOSVersion() const
{
	// TODO: Get correct OS version as soon as available in Windows Store app.s
	// https://social.msdn.microsoft.com/Forums/windowsapps/en-US/9270b1d2-50c0-4fb7-b20d-07d0451c04cb/how-to-get-the-os-version-in-metro-style-app
	return L"unknown";
}

std::wstring GameAnalyticsInterface::GetPlatform() const
{
	// TODO: Get correct platform.
	return L"win8";
}

int GameAnalyticsInterface::GetSessionNumber() const
{
	return this->sessionNumber;
}

std::wstring GameAnalyticsInterface::GetSDKVersion() const
{
	return L"win 2.0";
}

uint64 GameAnalyticsInterface::GetTimeSinceInit() const
{
	LARGE_INTEGER currentTime;

	if (!QueryPerformanceCounter(&currentTime))
	{
		throw ref new Platform::FailureException(L"Unable to get system time.");
	}

	return (currentTime.QuadPart - this->initializationTime.QuadPart) / 10000000;
}

int GameAnalyticsInterface::GetTransactionNumber() const
{
	// TODO: Get correct transaction number.
	return 1;
}

JsonValue^ GameAnalyticsInterface::ToJsonValue(std::wstring s) const
{
	return JsonValue::CreateStringValue(ref new String(s.c_str()));
}

JsonValue^ GameAnalyticsInterface::ToJsonValue(double d) const
{
	return JsonValue::CreateNumberValue(d);
}

task<JsonObject^> GameAnalyticsInterface::SendGameAnalyticsEvent(const std::wstring & route, JsonObject^ eventObject) const
{
	// TODO: Add event queue, send reguarly, cache if offline.
	// Build event JSON.
	JsonArray^ jsonArray = ref new JsonArray();
	jsonArray->Append(eventObject);

	// Generate HMAC SHA256 of event data.
	auto jsonString = jsonArray->Stringify();
	auto secretKeyString = ref new String(this->secretKey.c_str());

	// TODO: Add compression.
	auto alg = MacAlgorithmProvider::OpenAlgorithm(MacAlgorithmNames::HmacSha256);
	auto jsonBuffer = CryptographicBuffer::ConvertStringToBinary(jsonString, BinaryStringEncoding::Utf8);
	auto secretKeyBuffer = CryptographicBuffer::ConvertStringToBinary(secretKeyString, BinaryStringEncoding::Utf8);
	auto hmacKey = alg->CreateKey(secretKeyBuffer);

	auto hashedJsonBuffer = CryptographicEngine::Sign(hmacKey, jsonBuffer);
	auto hashedJsonBase64 = CryptographicBuffer::EncodeToBase64String(hashedJsonBuffer);

	// Build category URL.
	std::wstring relativeUrl = this->gameKey + L"/" + route;
	
	// TODO: Replase by production URL http://api.gameanalytics.com/v2/
	std::wstring absoluteUrl = L"http://sandbox-api.gameanalytics.com/v2/" + relativeUrl;
	auto absoluteUrlString = ref new String(absoluteUrl.c_str());

	// Send event to GameAnalytics.
	auto message = ref new HttpRequestMessage();

	message->RequestUri = ref new Uri(absoluteUrlString);
	message->Method = HttpMethod::Post;
	message->Content = ref new HttpStringContent
		(jsonString,
		Windows::Storage::Streams::UnicodeEncoding::Utf8,
		ref new Platform::String(L"application/json"));
	message->Headers->TryAppendWithoutValidation(L"Authorization", hashedJsonBase64);

	auto response = ref new HttpResponseMessage();

	return create_task(httpClient->SendRequestAsync(message)).then([=](HttpResponseMessage^ response)
	{
		// Validate HTTP status code.
		response->EnsureSuccessStatusCode();
		return create_task(response->Content->ReadAsStringAsync());
	}).then([=](String^ responseBodyAsText){
		JsonObject^ json = JsonObject::Parse(responseBodyAsText);
		return json;
	});
}
