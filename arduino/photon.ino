SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

const String kToken = "<token>";

void setup() {
    Particle.connect();
    
    Serial1.begin(9600);
    
    Particle.subscribe("hook-response/<WebhookPost>", responseHandler, MY_DEVICES);
    Particle.subscribe("hook-response/<WebhookGet>", responseHandler, MY_DEVICES);
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
            Particle.publish("<WebhookPost>", "{ \"type\": \"loguid\", \"uid\": \"" + arg + "\", \"token\": \"" + kToken + "\" }", PRIVATE);
            
        else if (command == "LOGSID")
            Particle.publish("<WebhookPost>", "{ \"type\": \"logsid\", \"sid\": \"" + arg + "\", \"token\": \"" + kToken + "\" }", PRIVATE);
            
        else if (command == "PERMUID")
            Particle.publish("<WebhookGet>", "{ \"type\": \"permissions\", \"uid\": \"" + arg + "\", \"token\": \"" + kToken + "\" }", PRIVATE);
            
        else if (command == "ADDUID") {
            String sid;
            String uid;
            
            for(int i = 0; i < arg.length() - 1; i++) {
                if (arg.substring(i,i+1) == " ") {
                    sid = arg.substring(0,i);
                    uid = arg.substring(i+1,arg.length());
                    i = arg.length();
                }
            }
            
            Particle.publish("<WebhookPost>", "{ \"type\": \"adduid\", \"uid\": \"" + uid + "\", \"sid\": \"" + sid + "\", \"token\": \"" + kToken + "\" }", PRIVATE);
        }
            
        else Serial1.println("ECHO " + inStr);
    }
}

void responseHandler(const char *event, const char *data) {
    // Handle the integration response
    String name;
    String status;
    String error;
    String tier;
    
    JSONValue json = JSONValue::parseCopy(data);
    
    JSONObjectIterator iter(json);
    while(iter.next()) {
        if (iter.name() == "name") {
            name = (const char *) iter.value().toString();
        } else if (iter.name() == "status") {
            status = (const char *) iter.value().toString();
        } else if (iter.name() == "error") {
            error = (const char *) iter.value().toString();
        } else if (iter.name() == "tier") {
            tier = (const char *) iter.value().toString();
        }
    }
    
    if (error != "") {
        Serial1.println("ERROR " + error);
    } else if (name != "" && status != "") {
        if (status == "in") {
            Serial1.println("IN " + name);
        } else if (status == "out") {
            Serial1.println("OUT " + name);
        } else if (status == "success") {
            Serial1.println("SUCCESS " + name);
        } else {
            Serial1.println("RESP: " + String(data));
        }
    } else {
        if (status == "permissions") {
            if (tier == "admin") {
                Serial1.println("ADMIN");
            }
        }
        else {
            Serial1.println("RESP: " + String(data));
        }
    }
}