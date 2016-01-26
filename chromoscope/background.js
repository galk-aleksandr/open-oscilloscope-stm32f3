chrome.app.runtime.onLaunched.addListener(function() {
    chrome.app.window.create('index.html', {
        innerBounds: {
            width: 1280,
            height: 800,
            minWidth: 640,
            minHeight: 400
        },
        id: "ChromeApps-Oscilloscope"
    });
});
