#include <SoftwareSerial.h>
#include <MD5.h>

SoftwareSerial bluetooth(4, 2); // RX, TX

struct Entry {
    String name;
    unsigned pin;
    int(*callback)(String value);
};

class Handler {
  private:
    Entry* list;
    size_t size;
  
  public: 
    Handler() {
      this->list = NULL;
      this->size = 0;  
    };
    
    ~Handler() {
      delete []list;
    }
    
    bool bind(String name, unsigned pin, int(*callback)(String value));
    bool update(String data);
    void shutdown();
};

bool Handler::bind(String name, unsigned pin, int(*callback)(String value)) {
    this->list = (Entry*)realloc(this->list, ++size * sizeof(Entry));
    if (this->list != NULL) {
      list[size] = Entry{name, pin, callback};
      Serial.println("Appended " + name);
    } else {
      this->shutdown();
      return false;
    }
    return true;
}

bool Handler::update(String data) {
  
}

void Handler::shutdown() {
  for (int i = 0; i < this->size; i++) {
    // turn off outputs  
  }  
}

int testt(String value) {
  return 4;  
}

String inData = "";
Handler *handler = new Handler();

void setup() {
  // Open serial communications:
  Serial.begin(9600);
  Serial.println("Type AT commands!");

  handler->bind("halvljus1", 4, testt);
  handler->bind("halvljus2", 4, testt);
  handler->bind("halvljus3", 4, testt);
  handler->bind("halvljus4", 4, testt);
  
  // The HC-06 defaults to 9600 according to the datasheet.
  bluetooth.begin(9600);
}

void loop() {
  // Read device output if available.

  
  
  
  while (bluetooth.available() > 0) {

    char recieved = (char)bluetooth.read();
    
    if (recieved == '\n') {
      //Serial.println(md5str);
      Serial.println("Recieved: " + inData); 

      handler->update(inData);

      inData = ""; 
      
    } else {
      inData += recieved;
    }
  }
      /*
      if (inData[0] == '!') {
        int str_len = inData.length() + 1;
        char arr[str_len];
  
        inData.toCharArray(arr, str_len);
  
        unsigned char* hash = MD5::make_hash(arr);
        char *md5str = MD5::make_digest(hash, 16);

        
        Serial.println(inData.substring(1, 5));
        Serial.println(md5str);
      }
      */

  
  // Read user input if available.
  if (Serial.available()){
    delay(10); // The delay is necessary to get this working!

    bluetooth.print(Serial.readString() + '\n');
  }
}
