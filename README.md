# Game Analytics for Windows Phone 8.1 & Windows Store Apps

This is an open-source library for integrating the free GameAnalytics (http://www.gameanalytics.com/) backend in Windows Phone 8.1 and Windows Store apps. The library works out-of-the-box with the standard Visual Studio "Visual C++ Windows Phone DirectX App" and "Visual C++ Windows Store DirectX App" template.

## Integration

### Enable network access for your app

Open the package.appxmanifest file, and add the _Internet (Client & Server)_ capability for your Windows Phone 8.1 app, or the _Internet (Client)_ capability for your Windows Store app.

### Import source code

Add the source and header files to your app project.

### Enter your game keys

Include the GameAnalyticsInterface.h header file and create a new instance of the GameAnalytics interface, specifying your game key and secret key. You can find your keys on the GameAnalytics dashboard, under your Game Settings.

```
  #include "GameAnalyticsInterface.h"

  auto ga = std::make_shared<GameAnalytics::GameAnalyticsInterface>(L"your_game_key", L"your_secret_key");
```

The interface will automatically generate a unique session id when instantiated.

### Send some data

Done! GameAnalytics is set up and ready for use. Sending a design event to the backend is as easy as:

```
  ga->SendDesignEvent(L"TestEvent:TestEventType");
```

Note that this method call is asynchronous. Calling this methods won't cause your app to block.

For sending business, error or user events, call SendBusinessEvent, SendErrorEvent or SendUserEvent, respectively.

## Error Handling

If any errors occur sending the event, a Platform::COMException will be thrown. This most likely indicates an invalid game or secret key. Double-check the keys in your dashboard, and ensure you're connected to the internet and the app is correctly set up to access the network.

If the event could be submitted, but any other errors occur, a Platform::FailureException will be thrown, containing the response JSON sent by the GameAnalytics backend.

## Specifying User and Build IDs

By default, this plugin will use the app package version as build id, and the Application Specific Hardware Identifier (ASHWID) as user id (see https://msdn.microsoft.com/en-us/library/windows/apps/jj553431 for details).

You can change these at any time by the SetBuild and SetUserId methods.

## Specifying The Current Area

Design, business and error events allow you to indicates the area or game level where the event occurred. You can set the area for all subsequent events by calling the SetArea method.

## Contributors

While he's no direct contributor to this library, Jason Ericson helped me a great deal by providing a [C++ implementation for GameAnalytics](http://jasonericson.blogspot.dk/2013/03/game-analytics-in-c.html).