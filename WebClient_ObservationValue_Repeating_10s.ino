#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>

int sensorPin = 0;

byte mac[] = {  0x90, 0xA2, 0xDA, 0x10, 0xBB, 0xB7 };

const char server[] PROGMEM = "scc-cnor-129py5.scc.kit.edu";

IPAddress ip(192, 168, 137, 10);
IPAddress mydns( 129, 13, 64, 5 ); // 192, 168, 10, 2

EthernetClient client;

unsigned long lastConnectionTime = 0;             // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

char c;
String bufetag = "";
String etag = "";

void setup() {
  // setup code to run once:
  // start serial port:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  Ethernet.begin(mac, ip); //, mydns
  // print the Ethernet board/shield's IP address:
  delay(2000); //time for Ethernet shield to initalize
  Serial.print(F("My IP address: "));
  Serial.println(Ethernet.localIP());

  //configures reference voltage for temperature sensor to 1.1 instead of 5V
  analogReference(INTERNAL); 

  char buf[25];
  strcpy_P(buf, server); // copy server from flash to ram

   ////first GET request to receive the appropriate ETag in order to send a PUT request
  // if there's a successful connection:
  if (client.connect(buf, 8080)) {
    Serial.println(F("connecting GET...")); 
    // send the HTTP GET request:
    client.println(F("GET /marmotta/ldp/ldbc/SensorPart/TempValue/ObservationValue HTTP/1.1"));
    client.println(F("Host: scc-cnor-129py5.scc.kit.edu:8080 "));
    client.println(F("Authorization: Basic YW5vbnltb3VzOlBhc3N3b3JkMQ==")); 
    client.println(F("Accept: text/turtle"));
    client.println(F("Connection: close"));
    client.println();
  } else {
    // if you couldn't make a connection:
    Serial.println(F("connection failed"));
  }

}

void loop() {
  // main code to run repeatedly:
  // if there's incoming data from the net connection.
  // send it out the serial port. 
  if (client.available()) {
    bufetag = "";
    c = client.read();
    Serial.print(c);
    if (c == 'W') {
      c = client.read();
      Serial.print(c);
      if ( c == '/') {
        for (int i = 1; i < 16; i++) {
          c = client.read();
          Serial.print(c);
          bufetag.concat(c);
          etag = bufetag;
        }
      }
    }
  }
  //if ten seconds have passed since your last connection,
  //then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpPUTRequest();
  }
}

void httpPUTRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  if (c = -1) {
    Serial.println("ETag: W/" + etag);
  }
  client.stop();

  char buf[25];
  strcpy_P(buf, server); // copy server from flash to ram

  // if there's a successful connection:
  if (client.connect(buf, 8080)) {
    Serial.println(F("connecting PUT..."));

    int reading = analogRead(sensorPin);
    int temp = reading / 9.31; //10mV equals to 1 degree -> 10mV/ (1.1V /1024)=9.31

    //int temp = random(30);

    String prefix = "@prefix qudt: <http://qudt.org/schema/qudt#> . @prefix ssn: <hhtp://purl.oclc.org/NET/ssnx/ssn#> . @prefix unit: <http://data.nasa.gov/qudt/owl/unit#> .";
    String triple1 = "<> qudt:QuantityValue ";
    String triple2 = " ; qudt:unit unit:DegreeCelsius; a ssn:ObservationValue . "; 
    String content = prefix + triple1 + temp + triple2;

    client.println(F("PUT /marmotta/ldp/ldbc/SensorPart/TempValue/ObservationValue HTTP/1.1"));
    client.println(F("Host: scc-cnor-129py5.scc.kit.edu:8080 "));
    client.println(F("Authorization: Basic YW5vbnltb3VzOlBhc3N3b3JkMQ==")); //YW5vbnltb3VzOlBhc3N3b3JkMQ== , YWRtaW46cGFzczEyMw==
    client.print(F("If-Match: W/"));
    client.println(etag);
    client.println(F("Content-Type: text/turtle"));
    client.print(F("Content-length: "));
    client.println(content.length());
    client.println();
    client.println(content);
    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println(F("connection failed"));
  }
}
