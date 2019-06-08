#ifndef GLOBALTYPE_H
#define GLOBALTYPE_H




struct NMEA_GPS_Struct {
  float lat;
  float lng;
  float alt_m;
  float spd_knts;
  uint8_t fix;
  struct minmea_time time;
  bool GPS_Done;
  char line[255];
  char *linePtr;
  uint8_t lineSize;
};


#endif /* MINMEA_H */