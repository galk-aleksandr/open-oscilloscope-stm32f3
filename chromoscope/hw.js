/* Constants */
var encoder = new TextEncoder();
var FRAME_COMMAND = new Uint8Array(encoder.encode("FRAME")).buffer;
var CONFIG_COMMAND = new Uint8Array(encoder.encode("CONFIG")).buffer;

/* Variables */
var usbConnection = null;
var openDevice = null;
var cmdQueue = [];
var frameParam = {w: 1535, h: 4096, c: 3};

var data = {keyFrame: null, frame: null};

/**
 @type {CanvasRenderingContext2D}
 */

var deviceInfo = document.getElementById("device-info");

function registerDevice(device) {
    if (openDevice != null) return;
    chrome.usb.openDevice(device, function (connection) {
        if (connection) {
            openDevice = device;
            usbConnection = connection;
            chrome.usb.claimInterface(usbConnection, 0, function () {
                chrome.usb.bulkTransfer(usbConnection, {
                    "direction": "out",
                    "endpoint": 2,
                    "data": CONFIG_COMMAND
                }, function (transferResult) {//todo check transfer result for errors.
                    chrome.usb.bulkTransfer(usbConnection, {
                        "direction": "in",
                        "endpoint": 129,
                        "length": 4096
                    }, function (transferResult) {
                        var config = String.fromCharCode.apply(null, new Uint8Array(transferResult.data));
                        releaseUsb();
                        showStatus("Device opened: " + openDevice.productName + "(" + openDevice.manufacturerName + ") #" + openDevice.serialNumber + "/" + config);
                        scanControls();
                        runFrame();
                    })
                });
            });


        } else {
            showStatus("Device failed to open.");
        }
    });
}
function onDeviceFound(devices) {
    if (devices) {
        if (devices.length > 0) {
            registerDevice(devices[0]);
        } else {
            showStatus("Device not connected");
        }
    } else {
        showStatus("Permission denied.");
    }
}

function releaseUsb() {
    chrome.usb.releaseInterface(usbConnection, 0, function () {
        if (chrome.runtime.lastError)
            console.warn(chrome.runtime.lastError);
    });
}
function runFrame() {

    function startAllFrameTransfer() {
        chrome.usb.bulkTransfer(usbConnection, {
            "direction": "out",
            "endpoint": 2,
            "data": FRAME_COMMAND
        }, function (transferResult) {//todo check transfer result for errors.
            chrome.usb.bulkTransfer(usbConnection, {
                "direction": "in",
                "endpoint": 129,
                "length": (frameParam.w * frameParam.c + 1) * 2
            }, function (transferResult) {
                var array = new Uint16Array(transferResult.data);
                //noinspection JSBitwiseOperatorUsage
                if (array[0] & 0x8000 !=0) {
                    data.frame = null;
                    data.keyFrame = array;
                } else {
                    data.frame = array;
                }
                releaseUsb();
                drawData(data);

                frameHandler = window.setTimeout(runFrame, 15);
            });
        });
    }

    function commandTransfer(transferResult) {
        if (cmdQueue.length > 0) {
            chrome.usb.bulkTransfer(usbConnection, {
                "direction": "out",
                "endpoint": 2,
                "data": encoder.encode(cmdQueue.pop()).buffer
            }, commandTransfer);
            data.frame = null;
            data.keyFrame = null;
        } else {
            startAllFrameTransfer();
        }
    }

    chrome.usb.claimInterface(usbConnection, 0, function () {
        if (chrome.runtime.lastError) {
            console.warn(chrome.runtime.lastError);
            return;
        }

        commandTransfer();
    });


}

function setParamFromInput(event) {
    var elm = event.srcElement;
    setParam(elm.name, elm.value)
}
function setParam(name, value) {
    cmdQueue.push(name + "=" + value)
}

chrome.usb.onDeviceRemoved.addListener(function (device) {
    if (device.device == openDevice.device) {
        if (frameHandler != null) window.clearTimeout(frameHandler);
        openDevice = null;
        chrome.usb.closeDevice(usbConnection);
        showStatus("Device disconnected");
    }
});

chrome.usb.onDeviceAdded.addListener(registerDevice);

function initialDeviceDetect() {
    chrome.usb.getDevices({"vendorId": 4617, "productId": 57622}, onDeviceFound);
}



