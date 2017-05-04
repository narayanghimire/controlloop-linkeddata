#include <SPI.h>
#include <Ethernet.h>
#include <avr/pgmspace.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xBB, 0xB7 };

const char server[] PROGMEM= "scc-cnor-129py5.scc.kit.edu";  

IPAddress ip(192, 168, 137, 10);

EthernetClient client;

void setup() {
  // setup code to run once:
  // Open serial communications and wait for port to open:
  Serial.begin(38400);
  while (!Serial) {
    ; // wait for serial port to connect
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println(F("connecting..."));

  char buf[25];
  strcpy_P(buf, server); //copy server from flash to ram

  // if you get a connection, report back via serial:
  if (client.connect(buf, 8080)) {
    Serial.println("connected");
    // Make a HTTP request:
    String prefix = "@prefix ssn: <http://purl.oclc.org/NET/ssnx/ssn#> . @prefix adr: <http://www.w3.org/2006/vcard/ns#> . ";
    String triples = "<> a ssn:SensingDevice ; adr:hasAddress <http://dbpedia.org/page/Karlsruhe> ."; 
    String content = prefix + triples;
    client.println(F("POST /marmotta/ldp/ldbc/SensorPart HTTP/1.1")); 
    client.println(F("Host: scc-cnor-129py5.scc.kit.edu:8080 ")); 
    client.println(F("Authorization: Basic YW5vbnltb3VzOlBhc3N3b3JkMQ==")); 
    client.println(F("Content-Type: text/turtle"));
    client.println(F("Link: <w3.org/ns/ldp/BasicContainer>; rel=”type”"));
    client.println(F("Slug: 90-A2-DA-10-BB-B7"));
    client.print(F("Content-length: ")); 
    client.println(content.length()); //sizeof(content)
    client.println();
    client.println(content);
    } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }
}

void loop() {
  // main code to run repeatedly:
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println(F("disconnecting."));
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}

