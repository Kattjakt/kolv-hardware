#include <SoftwareSerial.h>
#include <MD5.h>

SoftwareSerial bluetooth(4, 2); // RX, TX

struct Entry {
    String name;
    unsigned pin;
    int(*callback)(unsigned, String);
};

class Handler {
  private:
    Entry **list;
    size_t size;
  
  public: 
    Handler();
    ~Handler();
   
    bool bind(String name, unsigned pin, int(*callback)(unsigned, String));
    bool update(String data);
    void shutdown();
    bool sendStates();
    bool POST();
};

Handler::Handler() {
  this->list = (Entry **)malloc(0);
  this->size = 0;  
};

Handler::~Handler() {
  delete []list;
}

bool Handler::bind(String name, unsigned pin, int(*callback)(unsigned, String)) {
    list = (Entry **)realloc(this->list, (size + 1) * sizeof(Entry *));
    if (this->list != NULL) {
      list[size] = new Entry{name, pin, callback};
      pinMode(pin, OUTPUT);
      
      Serial.println(" * listening for: \"" + list[size]->name + "\"");
      size++;
      
    } else {
      this->shutdown();
      return false;
    }
    return true;
}

bool Handler::update(String data) {
  int argstart = data.indexOf('(');
  int argend   = data.indexOf(')');

  // Send all the current pin states when successfully connected
  if (data == "Connected") {
    this->sendStates();  
    return true;
  }

  // Start checking for commands with arguments
  if (argstart == -1 || argend == -1) {
    Serial.println("Got invalid command: " + data);  
    return false;
  }

  String command = data.substring(0, argstart);
  String args = data.substring(argstart + 1, argend);
  
  for (int i = 0; i < size; i++) {
    if (this->list[i]->name == command) {
      int response = this->list[i]->callback(list[i]->pin, args);  
      Serial.print("Got response from Hardware: ");
      Serial.println(response);
      
      return true;
    } 
  }

  return false;
}

void Handler::shutdown() {
  for (int i = 0; i < size; i++) {
    // If we don't do this, all pins will be left in their current state
    digitalWrite(this->list[i]->pin, LOW);  
  }  
}

bool Handler::POST() {
  
  return true;
}


int setLamp(unsigned pin, String value) {
  if (value == "on") {
    digitalWrite(pin, HIGH);  
  }

  if (value == "off") {
    digitalWrite(pin, LOW);  
  }
  
  return true;
}

bool Handler::sendStates() {
    
  for (int i = 0; i < size; i++) {
    String name = list[i]->name;
    int state = digitalRead(list[i]->pin);
    bluetooth.println("State(" + name + "," + String(state) + ")");
  }

  return true;
}



Handler *handler = new Handler();
String inData = "";

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
 
  handler->bind("headlight", 13, setLamp);
  handler->bind("blinkers_left", 12, setLamp);
  handler->bind("blinkers_right", 11, setLamp);

  handler->POST();
}

void loop() {
  while (bluetooth.available() > 0) {
    char recieved = (char)bluetooth.read();
    
    if (recieved == '\n') {
      Serial.println("Recieved: " + inData); 
      handler->update(inData);
      inData = ""; 
      
    } else {
      inData += recieved;
    }
  }

  if (Serial.available()){
    delay(10); // The delay is necessary to get this working!
    bluetooth.println(Serial.readString());
  }
}
