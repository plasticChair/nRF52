#ifndef MAIN_H
#define MAIN_H

#define screenSize  200
#define statusBarSize  30
#define numRows  4
#define numCols  2

#define lineWidth 2  // Pixels
#define yScreenSize (screenSize-statusBarSize)
#define rowHeight (yScreenSize/numRows)
#define rowWidth (screenSize/numCols)
#define xIconSpace (yScreenSize/numRows)
#define iconSize 25
#define iconSizeSmall 15
#define iconEdgeSpace 5

#define COLOR_WHITE 0
#define COLOR_BLACK 1
#define COLOR_RED 2
#define COLOR_REDINV 3

#define STATUS_ACQ 0
#define STATUS_TRK 1
#define STATUS_GPS 2
#define STATUS_BT  3
#define STATUS_SD  4

struct screenPos{
  uint8_t X;
  uint8_t Y;
};

struct screenField{
  struct screenPos scDate;
  struct screenPos scTime;
  struct screenPos humid; 
  struct screenPos curAlt;
  struct screenPos distance;
  struct screenPos curTemp;
  struct screenPos tempH;
  struct screenPos tempL;
  struct screenPos altGain;
  struct screenPos altLow;
  struct screenPos tripTime;
  struct screenPos comp;
  struct screenPos avgSpd;
  struct screenPos battV;

  struct screenPos iconDT;
  struct screenPos iconAlt;
  struct screenPos iconHum;
  struct screenPos iconDis;
  struct screenPos iconTemp;
  struct screenPos iconTempH;
  struct screenPos iconTempL;
  struct screenPos iconAltH;
  struct screenPos iconAltL;
  struct screenPos iconComp;
  struct screenPos iconSpd;
  struct screenPos iconBatt;
  
  struct screenPos iconStatusBar1;
  struct screenPos iconStatusBar2;
  struct screenPos iconStatusBar3;
  struct screenPos iconStatusBar4;
  struct screenPos iconStatusBar5;

} ;


#endif /* MAIN_H */