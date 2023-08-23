#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include "settings.h"

#ifdef USE_WEBSERVER

#include <ESP8266WebServer.h>

class WebServer
{
  friend class Store;
private:
  static ESP8266WebServer ws;
  bool outputScene = false;
  uint16_t outputSceneNum = 0;

  String getFlashString(const char *fStr);

  void webHome();
  void webSave();
  void webCSS();
  void webFirmwareUpdate();
  void webFirmwareUpload();
  void webNotFound();
  void webStore();

  bool checkIp(const String & id, IPAddress & tmpAddr);

public:
  void start();
  inline void handleClient() { ws.handleClient(); }
};
extern WebServer webServer;

#endif

#endif
