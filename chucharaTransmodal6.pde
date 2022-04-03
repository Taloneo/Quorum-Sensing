
import themidibus.*; //Import the library
import mqtt.*;

MQTTClient client;
MidiBus myBus; // The MidiBus

int minMidi = 5;
int maxMidi = 115;//limitamos el valor maximo para no pasar el fader de 0 db.
float msjLerp;
int lastMsjInt;
int midiCh = 1;

class Adapter implements MQTTListener {
  void clientConnected() {
    println("client connected");

    client.subscribe("acceTotal1"); 
    client.subscribe("acceTotal2");
    client.subscribe("acceTotal3");
  }

  void messageReceived(String topic, byte[] payload) {
    if (topic.equals("acceTotal1") == true) {
     // println("LLego al cliente 1" + "-" +new String(payload));
      dataToMidi(payload, 1);
    }
    if (topic.equals("acceTotal2") == true) {
      //println("LLego al cliente 2"  + "-" + new String(payload));
      dataToMidi(payload, 2);
    } 
    if (topic.equals("acceTotal3") == true) {
    //  println("LLego al cliente 3" + "-"  + new String(payload));
      dataToMidi(payload, 3);
    }

    //  println("new message: " + topic + " - " + new String(payload));
  }

  void connectionLost() {
    println("connection lost");
  }
}

Adapter adapter;

void setup() {
  adapter = new Adapter();
  client = new MQTTClient(this, adapter);
  client.connect("mqtt://localhost@192.168.1.108", "processing");
  MidiBus.list(); // List all available Midi devices on STDOUT. This will show each device's index and name.
  myBus = new MidiBus(this, -1, "loopMIDI Port"); //Crear un nuevo bus midi sin entrada y con salida el nombre del puerto, puede ser tambien su index.
}


void draw() {
}

void keyPressed() {
  client.publish("inTopic", "12");
}

void dataToMidi(byte[] payload, int cliente) {
  
  
  byte [] msj= payload;
  String msjString = new String(msj);
  Float msjFloat = Float.valueOf(msjString);
  float thisMsjLerp = msjLerp;
  

  if (msjFloat -0.5 <= minMidi) { //interpolacion de datos, una para subida otra para bajada
    thisMsjLerp = lerp(thisMsjLerp, msjFloat, 0.15);
  } else {
    thisMsjLerp = lerp(thisMsjLerp, msjFloat, 0.60);
  }
  int msjInt = constrain(int(thisMsjLerp), 0, maxMidi);
  myBus.sendControllerChange(midiCh, 15 + cliente, msjInt);//unico midi ch multiples msjs cc
  //myBus.sendControllerChange(cliente, 15 + cliente, msjInt);// multiples midi ch y msjs cc
  
  
}
