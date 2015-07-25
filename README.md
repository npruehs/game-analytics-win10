# Game Analytics for Windows 10 & Windows Phone 10 Apps

This is an open-source library for integrating the free GameAnalytics (http://www.gameanalytics.com/) backend in Windows 10 and Windows Phone 10 apps. The library works out-of-the-box with the standard Visual Studio "Visual C++ Windows Universal DirectX App" template.

## Integration

### Enable network access for your app

Open the package.appxmanifest file, and add the _Internet (Client & Server)_ capability for your Windows Phone app, or the _Internet (Client)_ capability for your Windows Store app.

### Import source code

Add the source and header files to your shared app project.

### Enter your game keys

Include the GameAnalyticsInterface.h header file and create a new instance of the GameAnalytics interface, specifying your game key and secret key. You can find your keys on the GameAnalytics dashboard, under your Game Settings.

```
  #include "GameAnalyticsInterface.h"

  auto ga = std::make_shared<GameAnalytics::GameAnalyticsInterface>(L"your_game_key", L"your_secret_key");
```

The interface will automatically generate a unique session id when instantiated.

### Initialize GameAnalytics

Before you can send some events, you need to initialize the library and authenticate yourself with the server.

```
  ga->Init();
```

It is also highly recommended to send a User event before sending any other events, in order to properly set up session tracking. You can pass any user you've got, or just pass an empty User object.

```
	ga->Init().then([this](JsonObject^ response)
	{
		this->ga->SendUserEvent(GameAnalytics::User());
	}
```

### Send some data

Done! GameAnalytics is set up and ready for use. Sending a design event to the backend is as easy as:

```
  ga->SendDesignEvent(L"TestEvent:TestEventType");
```

Note that this method call is asynchronous. Calling this methods won't cause your app to block.

You can send other events by calling the SendBusinessEvent, SendErrorEvent, SendProgressionEvent and SendResourceEvent methods.

### Session handling

You should propagate the [App lifecycle](https://msdn.microsoft.com/en-us/library/windows/apps/hh464925.aspx) Suspending and Resuming events to GameAnalytics by calling SendSessionEndEvent and SendUserEvent, respectively. 

## Error Handling

If any errors occur sending the event, a Platform::COMException will be thrown. This most likely indicates an invalid game or secret key. Double-check the keys in your dashboard, and ensure you're connected to the internet and the app is correctly set up to access the network.

## Specifying User and Build IDs

By default, this plugin will use the app package version as build id, and the Application Specific Hardware Identifier (ASHWID) as user id (see https://msdn.microsoft.com/en-us/library/windows/apps/jj553431 for details).

You can change these at any time by the SetBuild and SetUserId methods.

## Contributors

While he's no direct contributor to this library, Jason Ericson helped me a great deal by providing a [C++ implementation for GameAnalytics](http://jasonericson.blogspot.dk/2013/03/game-analytics-in-c.html).