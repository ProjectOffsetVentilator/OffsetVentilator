import processing.serial.*; //<>// //<>//

Serial myPort;  // Create object from Serial class
String portName;
int timestamp;
float temp1, press1, temp2, press2, temp3, press3;

// To save data to csv
Table table;

//To plot data
Graph graphTemp, graphPressure, graphFlow;
PFont f;

void setup() 
{
  size(1200, 1000);
  smooth();
  
  for (int i = 0; i<Serial.list().length; ++i) {
    println("["+i+"] "+Serial.list()[i]);
  }
  portName = Serial.list()[11]; // insert the correct number from the comport listed in the console
  
  myPort = new Serial(this, portName, 115200);

  table = new Table();
  table.addColumn("timestamp");
  table.addColumn("temp1");
  table.addColumn("press1");
  table.addColumn("temp2");
  table.addColumn("press2");
  table.addColumn("temp3");
  table.addColumn("press3");
  
  
  int xNumValues = 2000;
  graphTemp = new Graph(3, 5, 5, width-10, (height-10)/4, xNumValues, 15, 30, 4, 3);
  graphPressure = new Graph(3, 5, 5+(height-10)/4, width-10, (height-10)/4, xNumValues, 950, 1100, 4, 3);
  graphFlow = new Graph(3, 5, 10+(height-10)/2, width-10, (height-10)/2, xNumValues, -40, 40, 4, 7);
  //String[] fontList = PFont.list();
  //printArray(fontList);
  f = createFont("Courier", 10);
  textFont( f );
}

void draw()
{
  if (receiveData()) {
    addRowToTable();
  }
  
  background(255);
  graphTemp.addValue( temp1, 0 );
  graphTemp.addValue( temp2, 1 );
  graphTemp.addValue( temp3, 2 );
  graphTemp.increaseIndex();
  graphTemp.display();
  
  graphPressure.addValue( press1, 0 );
  graphPressure.addValue( press2, 1 );
  graphPressure.addValue( press3, 2 );
  graphPressure.increaseIndex();
  graphPressure.display();
  
  graphFlow.addValue( press1-press3, 0 );
  graphFlow.addValue( press2-press3, 1 );
  graphFlow.addValue( press1-press2, 2 );
  
      text(press1-press3, height/2-10, 0);
      text(press2-press3, height/2-30, 0);


  graphFlow.increaseIndex();
  graphFlow.display();
}

void exit() {
  saveTable(table, "data/"+year()+"_"+month()+"_"+day()+"-"+hour()+"_"+minute()+".csv");
  super.exit();
}


boolean receiveData() {
  if ( myPort.available() > 0) {  // If data is available,
    String input1 = myPort.readStringUntil('\n');
    if (input1 != null) {
      input1 = trim(input1);
      String[] values1 = split(input1, ",");

      // if the string came from serial port one:
      if (values1.length == 7) {
        timestamp = int(values1[0]); 
        temp1 = float(values1[1]); 
        press1 = float(values1[2]);
        temp2 = float(values1[3]);
        press2 = float(values1[4]);
        temp3 = float(values1[5]);
        press3 = float(values1[6]);

        /*
        print(timestamp); 
         print(","); 
         print(temp1); 
         print(","); 
         print(press1); 
         print(","); 
         print(temp2); 
         print(","); 
         print(press2); 
         print(",");         
         print(temp3); 
         print(","); 
         println(press3); 
         */
         
        return true;
      } else return false;
    }else return false;
  } else return false;
}

void addRowToTable() {
  TableRow newRow = table.addRow();
  newRow.setInt("timestamp", timestamp); 
  newRow.setFloat("temp1", temp1); 
  newRow.setFloat("press1", press1); 
  newRow.setFloat("temp2", temp2); 
  newRow.setFloat("press2", press2); 
  newRow.setFloat("temp3", temp3); 
  newRow.setFloat("press3", press3);
} 


// -----------------------
// The Graph class is based on the work of Limulo
// https://www.openprocessing.org/sketch/581776
// -----------------------

class Graph
{
  // Upper left corner of the entire graph
  PVector pos;
  // size of the graph 
  // (distance between UL and LR corners)
  PVector size;
  // size of the drawing (w/o margins)
  PVector range;
  // actual data values ranges
  PVector realRangeX, realRangeY;
  // position of the graph origin 
  PVector graphOrigin;

  float[][] history;
  int nHistory;

  // this index is pointing to the 
  // last added element in hisotry
  int index;

  // a value to keep track of the color
  float alpha = 255;
  // and weigth of the graph
  float weight;

  // ticks utilities
  int xTicks, yTicks; 
  float xTicksMargin, yTicksMargin;

  int col[] = {#FF0000, #00FF00, #0000FF, #FF00FF, #00FFFF, #FFFF00};
  int numValues;

  // CONSTRUCTOR ///////////////////////////////////////////////////////////////
  Graph(int _numValues, float _x, float _y, float _w, float _h, int _nHistory, float _yRangeMin, float _yRangeMax, int _xTicks, int _yTicks)
  {
    numValues = _numValues;
    pos = new PVector(_x, _y);
    size = new PVector(_w, _h);
    nHistory = _nHistory;

    xTicks = _xTicks;
    yTicks = _yTicks;

    graphOrigin= new PVector(size.x*0.1, size.y*0.9);
    range = new PVector(size.x*0.8, size.y*0.8);
    realRangeX = new PVector(0, nHistory);
    realRangeY = new PVector(_yRangeMin, _yRangeMax);

    xTicksMargin = yTicksMargin = min( size.x * 0.05, size.y * 0.05);

    history = new float[ nHistory ][numValues];
    for (int i=0; i<nHistory; i++) {
      for (int j=0; j<numValues; j++) {
        history[i][j] = 0.0;
      }
    }
  }

  // UPDATE ////////////////////////////////////////////////////////////////////
  void addValue(float _x, int i)
  {
    history[ index ][i] = _x;
  }
  void increaseIndex() {
    index = (index+1) % nHistory;
  }

  // DISPLAY ///////////////////////////////////////////////////////////////////
  void display()
  {
    pushStyle();
    pushMatrix();

    strokeWeight(1);

    translate(pos.x, pos.y);

    fill(255);
    rect(0, 0, size.x, size.y);

    translate(graphOrigin.x, graphOrigin.y);

    fill(0);
    stroke(120);
    displayAxes();

    stroke(200);
    fill(0);
    displayTicks();

    // we start counting from the last element back to the
    // most recent (the last added to the history buffer)
    for (int i=0; i<nHistory; i++)
    {
      for (int j=0; j<numValues; j++) {
        // x position scaled on range
        float xStart = (range.x/nHistory) * (nHistory-1-i);
        // same for the y position
        float yStart = map((history[(index+i)%nHistory ][j]), realRangeY.x, realRangeY.y, 0, range.y);

        // don't draw the last line
        // (between the first and the last point in the graph)
        if (i != nHistory-1) {
          float xEnd = (nHistory-i-2) * (range.x/nHistory);
          float yEnd = map((history[(index+i+1)%nHistory ][j]), realRangeY.x, realRangeY.y, 0, range.y);
          //alpha = map(i, 0, nHistory-1, 0.0, 255.0);
          //weight= map(i, 0, nHistory-1, 1, 5);
          strokeWeight(1);
          stroke(col[j], 255);
          line(xStart, -yStart, xEnd, -yEnd);
        }

        //ellipse( xStart, -yStart, 5, 5);
      }
    }

    popMatrix();
    popStyle();
  }

  // UTILITY ///////////////////////////////////////////////////////////////////
  void displayAxes()
  {
    textAlign(RIGHT);

    // x axis
    line(0, 0, 0, -range.y);
    text(realRangeX.x, 0.0, yTicksMargin);
    text(realRangeX.y, range.x, yTicksMargin);

    // y axis
    line(0, 0, range.x, 0);
    text(realRangeY.x, -xTicksMargin, 0);
    text(realRangeY.y, -xTicksMargin, -range.y);
  }

  void displayTicks()
  {
    float xSpacing = range.x / (xTicks + 1);
    float ySpacing = range.y / (yTicks + 1);

    float xRealSpacing = (realRangeX.y-realRangeX.x) / (xTicks + 1);
    float yRealSpacing = (realRangeY.y-realRangeY.x) / (yTicks + 1);

    textAlign(RIGHT);

    // x Ticks
    for (int i=0; i<xTicks; i++)
    {
      line(xSpacing*(1+i), 0, xSpacing*(1+i), -range.y);
      float legend = xRealSpacing*(1+i)+realRangeX.x;
      text(legend, xSpacing*(1+i), yTicksMargin);
    }

    // y Ticks
    for (int j=0; j<yTicks; j++)
    {
      line(0, -ySpacing*(1+j), range.x, -ySpacing*(1+j));
      float legend = yRealSpacing*(1+j)+realRangeY.x;
      text(legend, -xTicksMargin, -ySpacing*(1+j));
    }
  }
}
