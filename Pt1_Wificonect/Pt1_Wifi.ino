#include <WiFi.h>
#include <WebServer.h>
const char* ssid = "Trai Đẹp Cocacola";    
const char* password = "depzaikhoaito";
WebServer server(80);
String doorStatus = "ĐANG KHÓA";
#define RXD2 16
#define TXD2 17
void handleRoot() {
  String html = "<html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;margin-top:50px;}";
  html += "button{width:200px;height:80px;font-size:25px;margin:20px;border:none;border-radius:10px;cursor:pointer;color:white;}";
  html += ".btn-open{background-color:#28a745;} .btn-close{background-color:#dc3545;}";
  html += "h1{color:#333;} h2{font-size:24px;}</style>";
  html += "<meta http-equiv='refresh' content='3'></head>";
  html += "<body>";
  html += "<h1>HỆ THỐNG KHÓA CỬA</h1>";
  html += "<h2>Trạng thái: <span style='color:" + String(doorStatus == "ĐÃ MỞ" ? "green" : "red") + "'>" + doorStatus + "</span></h2>";
  html += "<a href='/open'><button class='btn-open'>MỞ CỬA</button></a>";
  html += "<br>";
  html += "<a href='/'><button class='btn-close' style='background-color:#6c757d'>LÀM MỚI</button></a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleOpen() {
  doorStatus = "ĐÃ MỞ";
  for(int i=0; i<10; i++) {
    Serial2.write('O'); 
    delay(10); // Nghỉ cực ngắn rồi gửi tiếp
  }
  Serial.println("Da gui spam lenh MO CUA xuong Arduino");
  server.sendHeader("Location", "/");
  server.send(303);
}
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); // RX=16, TX=17

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());
  
  server.on("/", handleRoot);
  server.on("/open", handleOpen);
  server.begin();
}
void loop() {
  server.handleClient();
  if (Serial2.available()) {
    String res = Serial2.readStringUntil('\n');
    res.trim();
    if (res == "OPEN_DOOR") doorStatus = "ĐÃ MỞ";
    else if (res == "LOCKED") doorStatus = "ĐANG KHÓA";
  }
}
