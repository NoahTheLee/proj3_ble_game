#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <M5Core2.h>

// ====================== CONFIG ======================
#define SERVICE_UUID "a7fb2851-526f-4fcb-b1a7-a2e580184f70"
#define CHARACTERISTIC_UUID "11745b7b-865c-44aa-8642-28e9dc0a21db"

const String serverHostname = "GameServer";
const String clientName = "GameClient";

// ====================== INTERNAL (you can ignore these) ======================
static bool isClientRole = false;
static bool deviceConnected = false;
static bool hasNewDataFlag = false;
static String lastReceived = "";

BLEServer *bleServer = nullptr;
BLEService *bleService = nullptr;
BLECharacteristic *bleCharacteristic = nullptr;
static BLERemoteCharacteristic *bleRemoteCharacteristic = nullptr;
static BLEAdvertisedDevice *bleRemoteServer = nullptr;
static boolean doConnect = false;
static boolean doScan = false;

// ====================== CALLBACKS ======================
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        deviceConnected = true;
        Serial.println("Server: Device connected");
    }
    void onDisconnect(BLEServer *pServer) override
    {
        deviceConnected = false;
        Serial.println("Server: Device disconnected");
    }
};

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient) override
    {
        deviceConnected = true;
        Serial.println("Client: Connected");
    }
    void onDisconnect(BLEClient *pclient) override
    {
        deviceConnected = false;
        Serial.println("Client: Disconnected");
    }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice) override
    {
        if (advertisedDevice.haveServiceUUID() &&
            advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID)) &&
            advertisedDevice.getName() == serverHostname.c_str())
        {
            BLEDevice::getScan()->stop();
            bleRemoteServer = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        }
    }
};

static void notifyCallback(BLERemoteCharacteristic *, uint8_t *pData, size_t length, bool)
{
    lastReceived = String((char *)pData, length);
    hasNewDataFlag = true;
    Serial.println("Client received: " + lastReceived);
}

// ====================== CORE FUNCTIONS ======================
void BLE_Begin(bool asClient)
{
    isClientRole = asClient;
    if (asClient)
    {
        BLEDevice::init(clientName.c_str());
        BLEScan *pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setInterval(1349);
        pBLEScan->setWindow(449);
        pBLEScan->setActiveScan(true);
        pBLEScan->start(5, false);
        Serial.println("Client scan started");
    }
    else
    {
        BLEDevice::init(serverHostname.c_str());
        bleServer = BLEDevice::createServer();
        bleServer->setCallbacks(new MyServerCallbacks());
        bleService = bleServer->createService(SERVICE_UUID);
        bleCharacteristic = bleService->createCharacteristic(CHARACTERISTIC_UUID,
                                                             BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE |
                                                                 BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
        bleCharacteristic->setValue("Hello from Server!");
        bleService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        BLEDevice::startAdvertising();
        Serial.println("Server broadcasting...");
    }
}

void BLE_Send(String message)
{
    if (!deviceConnected)
    {
        Serial.println("BLE not connected yet");
        return;
    }

    if (!isClientRole)
    {
        // SERVER sending to client(s)
        bleCharacteristic->setValue(message.c_str());
        bleCharacteristic->notify();
        Serial.println("Server sent: " + message);
    }
    else
    {
        // CLIENT sending to server
        if (bleRemoteCharacteristic)
        {
            bleRemoteCharacteristic->writeValue(message.c_str(), message.length());
            Serial.println("Client sent: " + message);
        }
    }
}

void BLE_ServerPollReceive()
{ // call every loop if you are the server
    if (!isClientRole && deviceConnected)
    {
        size_t len = bleCharacteristic->getLength();
        if (len > 0)
        {
            uint8_t *data = bleCharacteristic->getData();
            lastReceived = String((char *)data, len);
            hasNewDataFlag = true;
        }
    }
}

bool BLE_HasNewData() { return hasNewDataFlag; }
String BLE_Read()
{
    hasNewDataFlag = false;
    return lastReceived;
}
bool BLE_IsConnected() { return deviceConnected; }

// ====================== CLIENT CONNECTION (called automatically) ======================
bool connectToServer()
{
    if (!bleRemoteServer)
        return false;
    BLEClient *bleClient = BLEDevice::createClient();
    bleClient->setClientCallbacks(new MyClientCallback());

    if (!bleClient->connect(bleRemoteServer))
        return false;

    BLERemoteService *pService = bleClient->getService(SERVICE_UUID);
    if (!pService)
    {
        bleClient->disconnect();
        return false;
    }

    bleRemoteCharacteristic = pService->getCharacteristic(CHARACTERISTIC_UUID);
    if (!bleRemoteCharacteristic)
    {
        bleClient->disconnect();
        return false;
    }

    if (bleRemoteCharacteristic->canNotify())
    {
        bleRemoteCharacteristic->registerForNotify(notifyCallback); // ← this handles all incoming data
    }
    return true;
}

void BLE_ClientHandle()
{ // call every loop if you are the client
    if (doConnect)
    {
        if (connectToServer())
            Serial.println("Client now connected");
        else
            Serial.println("Client connect failed");
        doConnect = false;
    }
    if (!deviceConnected && doScan)
    {
        BLEDevice::getScan()->start(0);
    }
}

/*
Periodically update the client/server data
*/
void doHeartbeat(boolean isClient)
{
    if (isClient)
    {
        BLE_ClientHandle(); // keep client scanning & connected
    }
    else
    {
        BLE_ServerPollReceive(); // server must poll for incoming data
    }
}