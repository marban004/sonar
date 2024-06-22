import processing.serial.*; // imports library for serial communication
import java.awt.event.KeyEvent; // imports library for reading the data from the serial port
import java.io.IOException;
import java.util.HashMap;

Serial myPort;
String portName = "COM3";
int angle = 0;
int distance = 0;
int connectionStatus = 0;
int screenHeight = 800;
int screenWidth = 1200;
int maxDistance = 40;
boolean portError = false;
float arcDiameter;
/*
float arcDiameter0;
float arcDiameter1;
float arcDiameter2;
float arcDiameter3;
*/
float verticalSeparatorRadius;

void setup() {
  String param = "";
  String [] initParams = loadStrings("sonar.ini");
  HashMap<String, String> paramsList = new HashMap<>();
  paramsList.put("port name", portName);
  paramsList.put("height", Integer.toString(screenHeight));
  paramsList.put("width", Integer.toString(screenWidth));
  paramsList.put("max distance", Integer.toString(maxDistance));
  int index1 = 0;
  if(initParams.length != 0)
  {
    for(String line : initParams)
    {
      line = line.trim();
      index1 = line.indexOf("=");
      param = line.substring(0, index1).trim().toLowerCase();
      
      if(paramsList.containsKey(param))
      {
        paramsList.put(param, line.substring(index1+1, line.indexOf(";")).trim());
      }
    }
    portName = paramsList.get("port name");
    screenHeight = int(paramsList.get("height"));
    screenWidth = int(paramsList.get("width"));
    maxDistance = int(paramsList.get("max distance"));
    if( maxDistance < 40)
    {
      maxDistance = 40;
    }

  }
  windowResize(screenWidth, screenHeight);
  try
  {
  myPort = new Serial(this, portName, 9600);
  myPort.bufferUntil(';');
  }
  catch(Exception e)
  {
    portError = true;
  }
  arcDiameter = width * 0.2;
  /*
  arcDiameter0 = width * 0.2;
  arcDiameter1 = arcDiameter0 * 2;
  arcDiameter2 = arcDiameter0 * 3;
  arcDiameter3 = arcDiameter0 * 4;
  */
  verticalSeparatorRadius = width / 2 * 0.9;
}

void draw() {
  
  noStroke();
  fill(0, 4); 
  rect(0, 0, width, height - height * 0.095); 
  drawRadar();
  drawText();
  drawLineAndObjects();
  if(portError)
  {
    portError= false;
    try
    {
    myPort = new Serial(this, portName, 9600);
    myPort.bufferUntil(';');
    }
    catch(Exception e)
    {
      portError = true;
    }
  }
}

void drawRadar() {
  pushMatrix();
  translate(width / 2, height - height * 0.095); // moves the starting coordinats to new location
  noFill();
  textSize(height * 0.028);
  strokeWeight(2);
  
  stroke(98, 245, 31);
  for(int i = 1; i <= 4; i++)
  {
  arc(0, 0, arcDiameter * i, arcDiameter * i, PI, TWO_PI);
  //fill(98, 245, 31);
  //text(Integer.toString(maxDistance / 4 * i) + " cm", arcDiameter * i / 2, height * 0.03);
  //noFill();
  }

  
  for(int i = 0; i <= 12; i++)
  {
    if(i % 3 == 0)
    {
      strokeWeight(6);
      line(0, 0, verticalSeparatorRadius * cos(radians(15 * i)), - verticalSeparatorRadius * sin(radians(15 * i)));
      strokeWeight(2);
    }
  line(0, 0, verticalSeparatorRadius * cos(radians(15 * i)), - verticalSeparatorRadius * sin(radians(15 * i)));
  fill(98, 245, 31);
  text(Integer.toString(15 * i) + char(176), verticalSeparatorRadius * cos(radians(15 * i)), - verticalSeparatorRadius * sin(radians(15 * i)) * 1.01);
  }
  popMatrix();
}

void drawText() {
  float textHeight = height - height * 0.065;
  String text = "";
  switch(connectionStatus) {
    case 1:
      text = "Data loss";
      break;
    case 2:
      text = "Unstable";
      break;
    case 3:
      text = "Stable";
      break;
    default:
      text = "Disconnected";
  };
  fill(0, 0, 0);
  noStroke();
  rect(0, height - height * 0.095, width, height);
  fill(98, 245, 31);
  textSize(height * 0.028);
  text("Connnection status: " + text, width * 0.005, textHeight);
  text = "Ok";
  if(portError)
  {
    text = "Error";
  }
  text("Port Status: " + text, width * 0.25, textHeight);
  for(int i = 1; i <= 4; i++)
  {
  text(Integer.toString(maxDistance / 4 * i) + " cm", arcDiameter * i / 2 + width / 2, textHeight);
  }
}

void drawLineAndObjects()
{
  pushMatrix();
  translate(width / 2, height - height * 0.095); // moves the starting coordinats to new location
  stroke(30, 250, 60);
  strokeWeight(6);
  line(0, 0, verticalSeparatorRadius * cos(radians(angle)), - verticalSeparatorRadius * sin(radians(angle)));
  stroke(255, 10, 10);
  if(distance > 0 && distance <= maxDistance)
  {
  float distancePix = map(distance, 0, maxDistance, 0, arcDiameter * 2);
  point(distancePix * cos(radians(angle)), - distancePix * sin(radians(angle)));
  }
  popMatrix();
}

void serialEvent (Serial myPort) {
  String data = myPort.readStringUntil(';');
  data = data.substring(0, data.length()-1);

  int index = data.indexOf(",");
  if(index != -1)
  {
  angle = int(data.substring(0, index));
  data = data.substring(index + 1);
  index = data.indexOf(",");
  if(index != -1)
  {
  distance = int(data.substring(0, index)); 
  connectionStatus = int(data.substring(index + 1));
  }
  else
  {
  distance = 0;
  connectionStatus = 0;
  }
  }
}
