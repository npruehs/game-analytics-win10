# Game Analytics for Windows Store Apps

This is an open-source library for integrating the free GameAnalytics (http://www.gameanalytics.com/) backend in Windows Store apps. The library works out-of-the-box with the standard Visual Studio "Visual C++ Windows Store DirectX App" template.

## Integration

### Enable network access for your app

Open the package.appxmanifest file, and add the Internet (Client) capability for your app.

### Import source code

Add GameAnalyticsInterface.h and GameAnalyticsInterface.cpp to your Windows Store app project.

### Enter your game keys

Include the header file and create a new instance of the GameAnalytics interface, specifying your game key and secret key. You can find your keys on the GameAnalytics dashboard, under your Game Settings.

```
  #include "GameAnalytics\GameAnalyticsInterface.h"

  auto ga = std::make_shared<GameAnalytics::GameAnalyticsInterface>(L"your_game_key", L"your_secret_key");
```

The interface will automatically generate a unique session id when instantiated.

### Send some data

Done! GameAnalytics is set up and ready for use. Sending a design event to the backend is as easy as:

```
  ga->SendGameAnalyticsEvent(L"TestEvent:TestEventType");
```

You can also send other events by specifying the event category explicitly:

```
  ga->SendGameAnalyticsEvent(L"Purchase:RocketLauncher", L"business");
```

## Error Handling

If any errors occur sending the event, a Platform::COMException will be thrown. This most likely indicates an invalid game or secret key. Double-check the keys in your dashboard, and ensure you're connected to the internet and the app is correctly set up to access the network.

If the event could be submitted, but any other errors occur, a Platform::FailureException will be thrown, containing the response JSON sent by the GameAnalytics backend.

## Specifying User and Build IDs

By default, this plugin will use the app package version as build id, and the Application Specific Hardware Identifier (ASHWID) as user id (see https://msdn.microsoft.com/en-us/library/windows/apps/jj553431 for details).

You can change these at any time by the SetBuild and SetUserId methods.
