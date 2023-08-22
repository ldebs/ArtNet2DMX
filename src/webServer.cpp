#include "webServer.h"

#include "main.h"
#include "wifi.h"
#include "dmx.h"
#include "inout.h"
#include "store.h"

#include <Arduino.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

WebServer webServer;
ESP8266WebServer WebServer::ws(80);

#define HIDDEN_PASSWORD "********"

/////////  Web Page & CSS - stored to flash memory ////////////

const char style[] PROGMEM = "body {text-align:center;background:#333;}\n"
    "table {margin-left:auto;margin-right:auto;max-width:500px;width:100%;border-collapse:collapse;border:none;}\n"
    "th {height:40px;background:#666;color:white;font-weight:bold;border:none;}\n"
    ".mini_head {height:20px;background:#999;}\n"
    "td {padding:6px;border:1px solid #ccc;background:#eee;text-align:left;border:none; position: relative;}\n"
    ".left {width:120px;text-align:right;vertical-align:top;}\n"
    ".centre {text-align: center;}\n"
    "input:not(.button) {float: left;}\n"
    "input:not(.number):not(.radio):not(.button):not(.checkbox) {width: 100%;}\n"
    "#viewWifiPass {width:18px, height:18px; position: absolute; top: 7px; right: 10px; }\n"
    ".range {width:150px;}\n"
    ".number {width:50px;}\n"
    ".button {width:150px;margin:10px;}\n"
    ".smallButton {width:80px;}\n"
    ".static_table {border-collapse:collapse;}\n"
    "p {padding:0px;margin:0px;font-size:14px; clear: both;}\n"
    "a {color:#00A;text-decoration:none;}\n"
    "a:hover {color:#00F;text-decoration:underline;}\n"
    ".bigLink {color: white;}\n"
    ".round-button {display:block; float: left; margin-right: 7px; width:14px; height:14px; font-size:14px; line-height:14px; border-radius: 50%; color:#ffffff; text-align:center; text-decoration:none; background: #5555ff; box-shadow: 0 0 2px #9999ff; font-weight:bold; font-family: 'Comic Sans', serif;}";
    
const char page_head[] PROGMEM = "<html><head><title>ArtNet Node Config</title>\n"
    "<link rel='stylesheet' type='text/css' href='/style.css'>\n"
    "<meta name='viewport' content='width=400'>"
    "</head><body>\n"
    "<table id='wrap'>\n";

const char home_top[] PROGMEM = "<tr><th colspan=5><a href='/' class='bigLink'>WiFi ArtNode Config</a></th></tr>"
    "<form method='POST' action='/save' name='artNodeSettings'>\n";

const char save_top[] PROGMEM = "<tr><th><a href='/' class='bigLink'>WiFi ArtNode Config</a></th></tr><tr><td><center>";

const char save_tail[] PROGMEM = "</center></td></tr></table></body></html>";

const char store_top[] PROGMEM = "<tr><th colspan=5><a href='/' class='bigLink'>WiFi ArtNode Config</a></th></tr>"
    "<form method='POST' action='/store/new.save' name='artNodeStore'>\n";

const char store_tail[] PROGMEM =  "\n<tr><th colspan=5 class='mini_head'>Save a New Scene</th></tr>\n"
    "<tr><td colspan=3><input type=text name='sceneName' value='New Scene'></td>\n"
    "<td colspan=2><input type='submit' value='Save' class='smallButton'></td></tr>\n"
    "</form>\n"
    "</table>\n"
    "</body></html>";
    
const char form_tail[] PROGMEM = "<tr><th colspan=5 class='centre'>\n"
    "<input type='submit' value='Save Changes' class='button'>\n"
    "<input type='submit' name='restart' value='Save & Restart Node' class='button'>\n"
    "</th></tr></form>\n"
    "</table><br />"
    "<table id='wrap'>\n"
    "<tr><th colspan=5><a href='/store' class='bigLink'>Stored Scenes</a></th></tr>\n"
    "</table><br />"
    "<table id='wrap'>\n"
    "<form method='POST' action='/update' enctype='multipart/form-data'>"
    "<tr><th colspan=5>Firmware " FIRMWARE_VERSION "</th></tr>\n"
    "<tr><td colspan=1 class='left'>Update</td>\n"
    "<td colspan=4><input type='file' name='update'></td></tr>"
    "<tr><th colspan=5 class='centre'>\n"
    "<input type='submit' value='Update Firmware' class='button'>"
    "</th></tr></form>"
    "</table></body>\n"
    "<script>\n"
    "var rad = document.artNodeSettings.dhcp;\n"
    "var prev = null;\n"
    "for(var i = 0; i < rad.length; i++) {\n"
    "  rad[i].onclick = function() {\n"
    "    if(this.value != prev)\n"
    "      prev = this.value;\n"
    "      var tab = document.getElementsByClassName('static_table');\n"
    "      if (prev == 'false') {\n"
    "        for(var x = 0; x < tab.length; x++)\n"
    "          tab[x].style.display = 'table-row';\n"
    "      } else {\n"
    "        for(var x = 0; x < tab.length; x++)\n"
    "          tab[x].style.display = 'none';\n"
    "      }\n"
    "      var tab = document.getElementsByClassName('dhcp_table');\n"
    "      if (prev != 'false') {\n"
    "        for(var x = 0; x < tab.length; x++)\n"
    "          tab[x].style.display = 'table-row';\n"
    "      } else {\n"
    "        for(var x = 0; x < tab.length; x++)\n"
    "          tab[x].style.display = 'none';\n"
    "      }\n"
    "   };\n"
    "}\n"
    "</script>\n"
    "</html>";

const char firmware_success[] PROGMEM = "<html><body>"
    "Update successful.  Rebooting...\n"
    "<script>\n"
    "setTimeout(function () {\n"
    "   window.location.href = '/';\n"
    "}, 15000);\n"
    "</script>\n"
    "</body></html>";

const char firmware_fail[] PROGMEM = "<html><body>"
    "Update failed.  Rebooting...\n"
    "<script>\n"
    "setTimeout(function () {\n"
    "   window.location.href = '/';\n"
    "}, 15000);\n"
    "</script>\n"
    "</body></html>";

const char file_fail[] PROGMEM = "<tr><td colspan=5>Failed to read scenes list from file system.</td>";

/* getFlashString()
 *  Get our strings stored in flash memory
 */
String WebServer::getFlashString(const char *fStr) {
  int len = strlen_P(fStr);
  char buffer[len+1];
  int k;
  
  for (k = 0; k < len; k++)
    buffer[k] =  pgm_read_byte_near(fStr + k);
  
  buffer[k] = 0;
  
  return String(buffer);
}



/* startWebServer()
 *  Very self explanitory - it starts our webserver
 *  Sets the handlers for the various pages we will serve
 */
void WebServer::start() {
  ws.on("/", std::bind(&WebServer::webHome, &webServer));
  ws.on("/save", std::bind(&WebServer::webSave, &webServer));
  ws.on("/style.css", std::bind(&WebServer::webCSS, &webServer));
  ws.on("/update", HTTP_POST, std::bind(&WebServer::webFirmwareUpdate, &webServer), std::bind(&WebServer::webFirmwareUpload, &webServer));
  ws.onNotFound(std::bind(&WebServer::webNotFound, &webServer));

  //MDNS.begin(wifiSSID);
  ws.begin();
  //MDNS.addService("http", "tcp", 80);
  
}

/* webHome()
 *  Our main web page.
 */
void WebServer::webHome() {
  // Stop DMX interupts
  dmx.pause();
  
  // Initialize our page from our flash strings
  String message = getFlashString(page_head);
  message += getFlashString(home_top);

  // Our MAC Address
  message += "<tr><td class='left'>Mac Address</td><td colspan=4>"
        + wifi.getMac()
        + "</td></tr>";

  // ****************** Create our settings form *************
  
  message += "<tr><td class='left'>Node Name</td><td colspan=4>\n"
      "<input type='text' name='nodeName' value='"
      + String(settings.nodeName)
      + "'></td></tr>\n";

  
  // ******* WiFi Settings *********
  
  message += "\n<tr><th colspan=5 class='mini_head'>Wifi</th>\n";

  message += "<tr><td class='left'>SSID</td><td colspan=4>\n"
      "<input type='text' name='wifiSSID' value='"
      + String(settings.wifiSSID)
      + "'></td></tr>\n";

  message += "<tr><td class='left'>Password</td><td colspan=4>\n"
      "<img id='viewWifiPass' src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABIAAAASCAYAAABWzo5XAAAAAXNSR0IArs4c6QAAAiJJREFUOBHNkzFoU1EUhvNeQl4HQYdECLSgg05aOhTUKWQQCi1iBgkiOrjkJbxmKGpa6JBJkECG2IREcdAhuLiIg1NEsinoIIpEcLLEyUAoVR9J9DuX3HBTIwQnL5x37/nPf/577nn3BgL/w6jVavlKpbIgtVj/WhAiRcuybpD/sdPpnP6rULlcng+HwxcgniDhF6Pt+/6zXC73xRDxB4PBpWw2+/QPISk1GAyWSE4iEjxQ8QDBt8SWwcciwpkQYqfLYPcgHpIgSV2mV9gPsDjzEUwNYndd181p39YLRLYhN7QIeIOjHIO8gl3Ef6S5MsPzyMlrTFUEsEXgtgaZ39DAM5FI5DDH3LBt+xSY9MvHdrHjmB630ul00UIkhchjjco8HA7XM5nMTr1eb+ImBOMowMMkwkdx7wumB3hKjjYuTwcQ/jxan9MY8778HZK+Gphawt+0+/3+VXbrmUECi1RaBJsz8IeyJrZkYFJpD7umelStVuOU/ByCSiTwkwQHX3pyE3tHH15wt6KO47zHj2Ii8h1bow1N9dco+SX4eeybEEYisiwhUKbxLfqV4IK2wLRIF96qiAhRVSQLGZAfMF1XzujDjnskhHDNY74Gv8K1+KS5qiJxRj0RER/SHWY5glQnl1Mf+QPNdqnwrCmiePIRERLkAU5ce3B5ZydJtnhTbc/z2sKfNqxCoRCKxWJPCK7oBziNOBOGWJjdzTszU55J+g2ntvCk6kL2lQAAAABJRU5ErkJggg=='"
      " onClick=\"javascript: var inp = document.getElementById('wifiPass'); var pic = document.getElementById('viewWifiPass');"
      " if (inp.type == 'text') { inp.type = 'password'; pic.src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABIAAAASCAYAAABWzo5XAAAAAXNSR0IArs4c6QAAAdxJREFUOBHdUjtIQmEYvV5fBUEFOggFLdkQFERkS0O0NEQQDRFBY15RHGqQrMGptgJR1JYgyIbAofYamipoK0inKFBIiB5Qvjvn+l+5We3RD/d+33fO+c7/lKR/Owy/7SwcDndZLJZp8L1CkykWi0d+v//hp55vRtFotNtoNG4ZDIYZNBibmiqoU+VyecXr9d7ruS9G8Xh8HuQOTNooqtVqTwgXzDFGgHcyAf6GsKQoygFrDrkeJAkm6xAmNRPgSWylB+JJfsyJUU8NtbFYbI01h7oimKyC2KhD6v8qm826bDZbO7a5LMtyrVQqbefz+WeHw3EOxZCmxeqCmGhThslck4lUrVZ3Q6FQ2Ww2H8IkiKY15sTIaSaM7KUHtxbQE4K8E9iwjlNzNGqcjpICMpa2CORVjwIbYI24p+G6fFDDhOaFHtoZTQA8xmytQvRYKBT68WYeE4nEODG3232Kt2W3Wq3XKO3EYPCOb8rj8ZyoRgRFQwppB2sIbmHswaGfscYhjwGLAesT/BPyWU7AumHEIhKJOE0m0z4E+rP5IIfRUg/q/xKmC7itjIY13hEBn8+XzuVyLtyMAuGNENFANSFGDqsc1ZtQ92VForERuEK8IyeBSqWS5kQN8s8nn9HVzS14VqbqAAAAAElFTkSuQmCC';"
      " } else { inp.type = 'text'; pic.src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABIAAAASCAYAAABWzo5XAAAAAXNSR0IArs4c6QAAAiJJREFUOBHNkzFoU1EUhvNeQl4HQYdECLSgg05aOhTUKWQQCi1iBgkiOrjkJbxmKGpa6JBJkECG2IREcdAhuLiIg1NEsinoIIpEcLLEyUAoVR9J9DuX3HBTIwQnL5x37/nPf/577nn3BgL/w6jVavlKpbIgtVj/WhAiRcuybpD/sdPpnP6rULlcng+HwxcgniDhF6Pt+/6zXC73xRDxB4PBpWw2+/QPISk1GAyWSE4iEjxQ8QDBt8SWwcciwpkQYqfLYPcgHpIgSV2mV9gPsDjzEUwNYndd181p39YLRLYhN7QIeIOjHIO8gl3Ef6S5MsPzyMlrTFUEsEXgtgaZ39DAM5FI5DDH3LBt+xSY9MvHdrHjmB630ul00UIkhchjjco8HA7XM5nMTr1eb+ImBOMowMMkwkdx7wumB3hKjjYuTwcQ/jxan9MY8778HZK+Gphawt+0+/3+VXbrmUECi1RaBJsz8IeyJrZkYFJpD7umelStVuOU/ByCSiTwkwQHX3pyE3tHH15wt6KO47zHj2Ii8h1bow1N9dco+SX4eeybEEYisiwhUKbxLfqV4IK2wLRIF96qiAhRVSQLGZAfMF1XzujDjnskhHDNY74Gv8K1+KS5qiJxRj0RER/SHWY5glQnl1Mf+QPNdqnwrCmiePIRERLkAU5ce3B5ZydJtnhTbc/z2sKfNqxCoRCKxWJPCK7oBziNOBOGWJjdzTszU55J+g2ntvCk6kL2lQAAAABJRU5ErkJggg=='; }"
      "\" alt='View/Hide Password' />\n"
      "<input type='password' name='wifiPass' id='wifiPass' value='" HIDDEN_PASSWORD "'>\n"
      "</td></tr>\n";

  // ******* Hotspot Settings *********
  
  message += "\n<tr><th colspan=5 class='mini_head'>Wifi Hotspot</th>\n";

  message += "<tr><td class='left'>Start Delay</td><td colspan=4>\n"
      "<a href='#' onClick=\"javascript: var help = document.getElementById('helpHotSpotTimeout').style; if (help.display == 'none') {help.display='block';} else { help.display='none';}\" class='round-button'>?</a>\n"
      "<input type='number' name='hotSpotDelay' value='"
      + String(settings.hotSpotDelay)
      + "' min='10' max='65535' class='number'>\n"
      "<p id='helpHotSpotTimeout' style='display:none;'>The hotspot will start after <i>x</i> seconds if it can't connect to the WiFi specified above.  This will only occur on power up and not after a dropped WiFi connection.</p>\n"
      "</td></tr>\n";

  message += "<tr><td class='left'>Stand Alone</td><td colspan = 4>\n"
      "<a href='#' onClick=\"javascript: var help = document.getElementById('helpStandAlone').style; if (help.display == 'none') {help.display='block';} else { help.display='none';}\" class='round-button'>?</a>\n"
      "<input type='checkbox' name='standAlone' value='true' class='checkbox'";
  message += (settings.standAlone) ? " checked>\n" : ">\n";
  message += "<p id='helpStandAlone' style='display:none;'>Disable connecting to WiFi, have hotpot always enabled and enable receiving ArtNet through the hotspot.<br />By default, the device will connect to a WiFi network to receive ArtNet.  HotSpot is only for accessing this settings page.</p>\n"
      "</td></tr>\n";

      
  // ********* ArtNet Settings ********
  
  message += "\n<tr><th colspan=5 class='mini_head'>Artnet</th></tr>\n";

  message += "<tr><td class='left'>Subnet</td><td colspan=4>\n"
      "<input type='number' name='artNetSub' value='"
      + String(settings.artNetSub)
      + "' min=0 max=15 class='number'></td></tr>\n";

  message += "<tr><td class='left'>Universe A</td><td colspan=4>\n"
      "<input type='number' name='artNetUniA' value='"
      + String(settings.artNetUniA)
      + "' min=0 max=15 class='number'></td></tr>\n";

  message += "<tr><td class='left'>Universe B</td><td colspan=4>\n"
      "<input type='number' name='artNetUniB' value='"
      + String(settings.artNetUniB)
      + "' min=0 max=15 class='number'></td></tr>\n";



  // ********** IP Settings ***********

  message += "\n<tr><th colspan=5 class='mini_head'>IP Settings</th></tr>\n";

  message += "<tr><td class='left'></td><td colspan=2>\n"
      "<input type='radio' name='dhcp' value='true'";
  if (settings.dhcp)
    message += " checked";
  message += " class='radio'> &nbsp; "
    "<a href='javascript: document.getElementsByName(\"dhcp\")[0].click();'>DHCP</a>"
    "</td><td colspan=2><input type='radio' name='dhcp' value='false'";
  if (!settings.dhcp)
    message += " checked";
  message += " class='radio'> &nbsp; "
    "<a href='javascript: document.getElementsByName(\"dhcp\")[1].click();'>Static</a></td></tr>";

  IPAddress addr;
  // IP
  addr=IPAddress(settings.ip);
  message += "<tr class='static_table'";
  if (settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>IP</td>\n";
  for (int x = 0; x < 4; x++) {
    message += "<td><input type='number' name='ip_";
    message += char(x+48);
    message += "' value='"
        + String(addr[x])
        + "' min=0 max=255 class='number'></td>\n";
  }
  message += "</tr>\n";
  
  message += "<tr class='dhcp_table'";
  if (!settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>IP</td>\n";
  message += "<td colspan=4>";
  for (int x = 0; x < 4; x++) {
    if (x > 0)
      message += " . ";
    message += String(addr[x]);
  }
  message += "</td></tr>\n";

  
  
  // Subnet
  addr=IPAddress(settings.subnet);
  message += "<tr class='static_table'";
  if (settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>Subnet</td>\n";
  for (int x = 0; x < 4; x++) {
    message += "<td><input type='number' name='subnet_";
    message += char(x+48);
    message += "' value='"
        + String(addr[x])
        + "' min=0 max=255 class='number'></td>\n";
  }
  message += "</tr>\n";

  message += "<tr class='dhcp_table'";
  if (!settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>Subnet</td>\n";
  message += "<td colspan=4>";
  for (int x = 0; x < 4; x++) {
    if (x > 0)
      message += " . ";
    message += String(addr[x]);
  }
  message += "</td></tr>\n";
  
  // Broadcast
  addr=IPAddress(settings.broadcast_ip);
  message += "<tr class='static_table'";
  if (settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>Broadcast IP</td>\n";
  for (int x = 0; x < 4; x++) {
    message += "<td><input type='number' name='broadcast_ip_";
    message += char(x+48);
    message += "' value='"
        + String(addr[x])
        + "' min=0 max=255 class='number'></td>\n";
  }
  message += "</tr>\n";

  message += "<tr class='dhcp_table'";
  if (!settings.dhcp)
    message += " style='display:none;'";
  message += "><td class='left'>Broadcast IP</td>\n";
  message += "<td colspan=4>";
  for (int x = 0; x < 4; x++) {
    if (x > 0)
      message += " . ";
    message += String(addr[x]);
  }
  message += "</td></tr>\n";

  // ********** Misc Settings ***********

  message += "\n<tr><th colspan=5 class='mini_head'>Misc Settings</th></tr>\n";

  message += "<tr><td class='left'>LED Amplitude</td><td>0</td><td colspan=2>\n"
      "<input type='range' name='ledAmplitude' value='"
      + String((int)INTENSITY_UNCONVERT(settings.ledIntensity))
      + "' min=0 max=255 class='range'></td><td>255</td></tr>\n";
  
  message += "<tr><td class='left'>LED Blink BPM</td><td>40</td><td colspan=2>\n"
      "<input type='range' name='blinkBpm' value='"
      + String((int)BLINK_CONVERT(settings.blinkTimeoutEighth))
      + "' min=40 max=180 class='range'></td><td>180</td></tr>\n";
      
  message += "<tr><td class='left'>Status</td><td colspan=4>\n"
      + BIN16(statusLed.get())
      + "</td></tr>\n";

  message += "<tr><td colspan=5>\n"
      + globalinfo
      + "</td></tr>\n";

  // Add the end of the form & page
  message += getFlashString(form_tail);

  // Send to the client
  ws.sendHeader("Connection", "close");
  ws.send(200, "text/html", message);
  
  // Restart DMX interupts
  dmx.unPause();
}

bool WebServer::checkIp(const String & id, IPAddress & tmpAddr){
  bool ok = true;
  char tmpArg[32];
  int tmp;
  for (int x = 0; x < 4; x++) {
    ws.arg(id+x).toCharArray(tmpArg, 32);
    tmp = atoi(tmpArg);
    if (tmp < 0 || tmp > 255)
      ok = false;
    else
      tmpAddr[x]=tmp;
  }
  return ok;
}

#define __WS_ARG_CONV(tmp,wsarg,op,val,conv) globalinfo += "<br/>" #wsarg ":" + ws.arg(#wsarg); tmp = ws.arg(#wsarg).toDouble(); tmp = conv(tmp); tmp = tmp op val ? (message += String("- " #wsarg " set to ") + val + " (was " #op " " + val + ")<br/>",val) : tmp
#define __STORE(tmp,set) settings.set = tmp; globalinfo += String(" => " #set ":") + tmp
#define __STORE_WS_ARG_CONV(tmp,wsarg,set,op,val,conv) __WS_ARG_CONV(tmp,wsarg,op,val,conv); __STORE(tmp,set)
#define __STORE_WS_ARG(tmp,wsarg,set,op,val) __STORE_WS_ARG_CONV(tmp,wsarg,set,op,val,)
#define __STORE_WS_VAR(tmp,var,op,val) __STORE_WS_ARG(tmp,var,var,op,val)

#define __WS_ARG2_CONV(tmp,wsarg,op,val,op2,val2,conv) __WS_ARG_CONV(tmp,wsarg,op,val,conv) op2 val2 ? (message += String("- " #wsarg " set to ") + val2 + " (was " #op2 " " + val2 + ")<br/>",val2) : tmp
#define __STORE_WS_ARG2_CONV(tmp,wsarg,set,op,val,op2,val2,conv) __WS_ARG2_CONV(tmp,wsarg,op,val,op2,val2,conv); __STORE(tmp,set)
#define __STORE_WS_VAR2_CONV(tmp,var,op,val,op2,val2) __STORE_WS_ARG2_CONV(tmp,var,var,op,val,op2,val2,conv)
#define __STORE_WS_VAR2(tmp,var,op,val,op2,val2) __STORE_WS_ARG2_CONV(tmp,var,var,op,val,op2,val2,)

/* webSave()
 *  Handle the Save buttons being pressed on web page.
 *  Copy data into our global variables.
 *  Verifies data then calls our saveSettings function.
 *  Resets node if the save and reset button clicked
 */
void WebServer::webSave() {
  bool ok = true;
  String message = "";
  globalinfo="";

  // Stop DMX interupts
  dmx.pause();
  

  // Copy data into our variables
  ws.arg("nodeName").toCharArray(settings.nodeName, 32);

  // Checkbox for stand alone mode
  settings.standAlone = ws.hasArg("standAlone");

  // Get numbers
  double d;
  __STORE_WS_VAR(d,artNetSub,>,15);
  __STORE_WS_VAR(d,artNetUniA,>,15);
  __STORE_WS_VAR(d,artNetUniB,>,15);
  __STORE_WS_VAR2(d,hotSpotDelay,<,10,>,65535);
  __STORE_WS_ARG2_CONV(d,ledAmplitude,ledIntensity,<,0,>,255,INTENSITY_CONVERT);
  __STORE_WS_ARG2_CONV(d,blinkBpm,blinkTimeoutEighth,<,BLINK_CONVERT(180),>,BLINK_CONVERT(40),BLINK_CONVERT);
  
  // Copy more data into our variables
  ws.arg("wifiSSID").toCharArray(settings.wifiSSID, 32);
  if (ws.arg("wifiPass") != HIDDEN_PASSWORD)
    ws.arg("wifiPass").toCharArray(settings.wifiPass, 32);

  // If DHCP - set variables accordingly
  if (ws.arg("dhcp") == "true") {
    settings.dhcp = true;

    if (wifi.isHotSpot) {
      settings.ip = WiFi.softAPIP();
      wifi.ap_ip = WiFi.softAPIP();
    } else {
      settings.ip = WiFi.localIP();
      wifi.ap_ip = WiFi.localIP();
    }
    settings.subnet = WiFi.subnetMask();
    wifi.setBroadcastAddr();
  } 
  // If Static IP - check IPs first, then set if valid
  else {
    IPAddress ip(0,0,0,0);
    IPAddress broadcast_ip(0,0,0,0);
    IPAddress subnet(0,0,0,0);

    // Check IP
    if (!checkIp("ip_",ip))
      ok = false, message += "- Invalid IP Address<br/>";

    // Check broadcast
    if (!checkIp("broadcast_ip_",broadcast_ip))
      ok = false, message += "- Invalid Broadcast Address<br/>";

    // Check subnet
    if (!checkIp("subnet_",subnet))
      ok = false, message += "- Invalid IP Subnet<br/>";

    // If we dont have error message, set the IP variables
    if (ok) {
      // All IPs are valid, store them
      settings.ip = ip;
      settings.broadcast_ip = broadcast_ip;
      settings.subnet = subnet;
      settings.dhcp = false;
    }
  }

  // Save settings to EEPROM
  if (settings.save()){
    // IP issues from above - the IPs didn't get saved but the rest did
    if (!ok)
      message = "Some changes saved. There were the following issues:<br/><br/>" + message;
    else
      message += "Changes Saved.";
  }
  // Error saving our settings to EEPROM
  else
    message = "Error saving settings. Please try again.<br/><br/>" + message;

  // If save and reset pressed, notify user that device will reset
  if ( ws.arg("restart")[0] == 'S')
    message += "<br/><br/>Device will now restart with it's new settings. Please wait until it's done.<br/>";
  
  message += "<br/><a href='/'>Back to settings page</a><br/>";
  message += globalinfo;

  //Generate final page using flash strings
  String tmp = getFlashString(page_head);
  tmp += getFlashString(save_top);
  tmp += message;
  tmp += getFlashString(save_tail);

  // Send page
  ws.sendHeader("Connection", "close");
  ws.send(200, "text/html", tmp);
  
  // Restart DMX interupts
  dmx.unPause();
  
  // If 'save & reset' was pressed, check for any remaining requests then reset the node
  if ( ws.arg("restart")[0] == 'S') {
    uint32_t startTime = millis();

    // handle any pending requests
    while (true) {
      handleClient();
      delay(10);

      // After 5 seconds, exit the while loop
      if ((millis() - startTime) > 5000)
        break;
    }
    
    // reset the node    
    restart();
  }
}



/* webCSS()
 *  Send our style sheet to the web client
 */
void WebServer::webCSS() {
  // Stop DMX interupts
  dmx.pause();
  ws.sendHeader("Connection", "close");
  ws.send(200, "text/html", getFlashString(style));
  // Restart DMX interupts
  dmx.unPause();
}



/* webFirmwareUpdate()
 *  display update status after firmware upload and restart
 */
void WebServer::webFirmwareUpdate() {
  // Stop DMX so webserver is more responsive
  dmx.end();
  
  // Generate the webpage from the variables above
  String fail = getFlashString(page_head) + getFlashString(firmware_fail);
  String ok = getFlashString(firmware_success);

  // Send to the client
  ws.sendHeader("Connection", "close");
  ws.sendHeader("Access-Control-Allow-Origin", "*");
  ws.send(200, "text/html", (Update.hasError()) ? fail : ok);

  // Restart device
  restart();
}



/* webFirmwareUpload()
 *  handle firmware upload and update
 */
void WebServer::webFirmwareUpload() {
  // Stop DMX so webserver is more responsive
  dmx.end();
  
  HTTPUpload& upload = ws.upload();
  
  if(upload.status == UPLOAD_FILE_START){
    WiFiUDP::stopAll();
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if(!Update.begin(maxSketchSpace)){//start with max available size
      statusLed.set(ERROR_OTA_NO_SPACE);
    }
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
      statusLed.set(ERROR_OTA_FILE_WRITE);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(Update.end(true)){ //true to set the size to the current progress
      statusLed.set(SUCCESS_OTA);
      
      // Send success page to the client
      ws.sendHeader("Connection", "close");
      ws.sendHeader("Access-Control-Allow-Origin", "*");
      ws.send(200, "text/html", getFlashString(firmware_success));
    } else {
      statusLed.set(ERROR_OTA_FAIL);
      
      // Send fail page to the client
      ws.sendHeader("Connection", "close");
      ws.sendHeader("Access-Control-Allow-Origin", "*");
      ws.send(200, "text/html", getFlashString(firmware_fail));
    }
  }
  yield();
}



/* webNotFound()
 *  display a 404 page
 */
void WebServer::webNotFound() {
  // Stop DMX interupts
  dmx.pause();
  // Check if we're recalling a stored DMX state
  char charBuf[ws.uri().length() + 1];
  ws.uri().toCharArray(charBuf, ws.uri().length() + 1);
  char* tmp = strtok(charBuf, "/.");
  
  if (strcmp(tmp, "store") == 0) {
    webStore();
    return;
  }

  // Generate page from flash strings
  String message = getFlashString(page_head);
  message += getFlashString(save_top);
  message += "404: File Not Found\n<br />\n<br />";
  message += "URI: ";
  message += ws.uri();
  message += "<br />\n<br />\n<a href='/'>Go to settings page</a>";
  message += getFlashString(save_tail);

  // Send page
  ws.sendHeader("Connection", "close");
  ws.send(200, "text/html", message);

  // Restart DMX interupts
  dmx.unPause();
}



/* webStore()
 *  main page for storing/recalling Artnet data
 */
void WebServer::webStore() {
  // Stop DMX interupts
  dmx.pause();
  
  // Generate page from flash strings
  String message = getFlashString(page_head);
  message += getFlashString(store_top);

  // Get URI Parameteres
  char charBuf[ws.uri().length() + 1];
  ws.uri().toCharArray(charBuf, ws.uri().length() + 1);
  char* tmp = strtok(charBuf, "/.");    // tmp points to store
  tmp = strtok(NULL, "/.");             // tmp points to scene number or new or NULL

  if (tmp != NULL) {
    // Restore Artnet Output
    if (strcmp(tmp, "artnet") == 0) {
      store.scenesClear();
    
    // Save a new scene
    } else if (strcmp(tmp, "new") == 0) {
      char sceneName[32];
      ws.arg("sceneName").toCharArray(sceneName, 32);

      bool r = store.newScene(sceneName);

      if (!r) {
        message += "\n<tr><th colspan=5 class='mini_head'>Error</th></tr>\n";
        message += "<tr><td colspan=5><center>Failed to save new scene correctly.</center></td></tr>\n";
      } else {
        message += "\n<tr><th colspan=5 class='mini_head'>Success</th></tr>\n";
        message += "<tr><td colspan=5><center>New scene has been saved.</center></td></tr>\n";
      }
    } else {
      uint8_t sceneNum = atoi(tmp);
      tmp = strtok(NULL, "/.");
      bool r = 0;

      // Call appropriate fuctions
      if (strcmp(tmp, "save") == 0)
        r = store.sceneSave(sceneNum);
      else if (strcmp(tmp, "delete") == 0)
        r = store.sceneDelete(sceneNum);
      else if (strcmp(tmp, "load") == 0)
        r = store.sceneLoad(sceneNum);

      // Handle success and error
      if (!r) {
        message += "<tr><td colspan=5><center>ERROR - Scene " + String(sceneNum) + " failed to " + tmp + ".</center></td></tr>\n";
      } else if (strcmp(tmp, "load") != 0) {
        message += "<tr><td colspan=5><center>Success - Scene " + String(sceneNum) + " has been " + tmp + "d.</center></td></tr>\n";
      }
    }
  }

  // Display current output source (Artnet or scene name)
  message += "<tr><td colspan=2>Output Source</td>";
  if (!outputScene)
    message += "<td colspan=3>Artnet</td></tr>";
  else {
    if (outputSceneNum == 0)
      message += "<td colspan=3>All Lights Off</td></tr>";
    else {
      File f = LittleFS.open("/scenes.txt", "r");
      if (f) {
        while(f.available()) {
          uint16_t num = f.readStringUntil(':').toInt();
          String sceneName = f.readStringUntil('\n');
          if (num == outputSceneNum) {
            message += "<td colspan=3>" + sceneName + "</td></tr>\n";
            break;
          }
        }
      }
      f.close();
    }
    message += "<tr><td colspan=2></td><td colspan=3><a href='/store/artnet.restore'>Stop Scene & Resume Artnet</a></td></tr>\n";       
  } 

  // Saved scenes heading and list
  message += "\n<tr><th colspan=5 class='mini_head'>Saved Scenes</th></tr>\n";
  message += "<tr><td colspan=2>All Lights Off</td>\n";
  message += "<td><a href='/store/0.load'>Load</a></td>";
  message += "<td colspan=2><a href='/store/0.save'>Save</a></td></tr>";

  File f = LittleFS.open("/scenes.txt", "r");
  
  if (!f)
    message += getFlashString(file_fail);
  else {
    // read line by line from the file.  Links for load, save, delete
    while(f.available()) {
      String sceneNum = f.readStringUntil(':');
      String sceneDesc = f.readStringUntil('\n');
      
      message += "<tr><td colspan=2>" + sceneDesc + "</td>";
      message += "<td><a href='/store/" + sceneNum + ".load'>Load</a></td>";
      message += "<td><a href='/store/" + sceneNum + ".save'>Save</a></td>";
      message += "<td><a href='/store/" + sceneNum + ".delete'>Delete</a></td></tr>";
    }
  }
  
  f.close();

  // Display used space
  FSInfo fs_info;
  LittleFS.info(fs_info);
  message += "<tr><td colspan=5><center><br />";
  message += fs_info.usedBytes;
  message += " of ";
  message += fs_info.totalBytes;
  message += " bytes used.</td></tr>";
  
  message += getFlashString(store_tail);

  // Send page
  ws.sendHeader("Connection", "close");
  ws.send(200, "text/html", message);
  
  // Restart DMX interupts
  dmx.unPause();
}
