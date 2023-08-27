SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

void setup() {
    Particle.connect();
    
    Serial1.begin(9600);
    
    Particle.subscribe("hook-response/<WebhookPost>", responseHandler, MY_DEVICES);
}

void loop() {
    if (Serial1.available()) {
        String inStr = Serial1.readStringUntil('\n');
        inStr.trim();
        
        String command = inStr;
        String arg;
        for(int i = 0; i < inStr.length() - 1; i++) {
            if (inStr.substring(i,i+1) == " ") {
                command = inStr.substring(0,i);
                arg = inStr.substring(i+1,inStr.length());
                i = inStr.length();
            }
        }
        
        if (command == "PING") {
            if (Particle.connected())
                Serial1.println("PONG CLOUD");
            else {
                if (WiFi.ready())
                    Serial1.println("PONG WIFI");
                else
                    Serial1.println("PONG DISC");
            }
        }
            
        else if (command == "LOGUID")
            Particle.publish("<WebhookPost>", "{ \"type\": \"loguid\", \"uid\": \"" + arg + "\", \"token\": \"<token>\" }", PRIVATE);
            
        else if (command == "LOGSID")
            Particle.publish("<WebhookPost>", "{ \"type\": \"logsid\", \"sid\": \"" + arg + "\", \"token\": \"<token>\" }", PRIVATE);
            
        else Serial1.println("ECHO " + inStr);
    }
}

void responseHandler(const char *event, const char *data) {
    // Handle the integration response
    String name;
    String status;
    String error;
    
    JSONValue json = JSONValue::parseCopy(data);
    
    JSONObjectIterator iter(json);
    while(iter.next()) {
        if (iter.name() == "name") {
            name = (const char *) iter.value().toString();
        } else if (iter.name() == "status") {
            status = (const char *) iter.value().toString();
        } else if (iter.name() == "error") {
            error = (const char *) iter.value().toString();
        }
    }
    
    if (name != "" && status != "") {
        if (status == "in") {
            Serial1.println("IN " + name);
        } else {
            Serial1.println("OUT " + name);
        }
    } else {
        Serial1.println("ERROR " + error);
    }
}