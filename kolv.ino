#include <SoftwareSerial.h>
#include <MD5.h>

struct Entry {
    String name;
    unsigned pin;
    int(*callback)(String);
};

class Handler {
  private:
    Entry **list;
    size_t size;
  
  public: 
    Handler();
    ~Handler();
   
    bool bind(String name, unsigned pin, int(*callback)(String));
    bool update(String data);
    void shutdown();
    bool POST();
};

Handler::Handler() {
  this->list = (Entry **)malloc(0);
  this->size = 0;  
};

Handler::~Handler() {
  delete []list;
}

bool Handler::bind(String name, unsigned pin, int(*callback)(String)) {
    list = (Entry **)realloc(this->list, (size + 1) * sizeof(Entry *));
    if (this->list != NULL) {
      list[size] = new Entry{name, pin, callback};
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
  
  if (argstart == -1 || argend == -1) {
    Serial.println("Got invalid command: " + data);  
    return false;
  }

  String name = data.substring(0, argstart);
  String args = data.substring(argstart + 1, argend);
  
  for (int i = 0; i < size; i++) {
    if (this->list[i]->name == name) {
      int response = this->list[i]->callback(args);  
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
    digitalWrite(this->list[size]->pin, LOW);  
  }  
}

bool Handler::POST() {
  
  return true;
}


int testt(String value) {
  return value.length();
}

SoftwareSerial bluetooth(4, 2); // RX, TX
Handler *handler = new Handler();
String inData = "";

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
 
  handler->bind("backlyktah", 1, testt);
  handler->bind("backlyktav", 2, testt);
  handler->bind("halvljus", 3, testt);
  handler->bind("helljus", 4, testt);

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
    bluetooth.print(Serial.readString() + '\n');
  }
}
