#include <SPI.h>

#define PA_TO_CM_H2O 0.0101972

//Sensor's memory register addresses:
const byte NVM_PAR_T1_7_0 = 0x31;
const byte DATA_0    = 0x04;
const byte DATA_1    = 0x05;
const byte DATA_2    = 0x06;
const byte PWR_CTRL  = 0x1B;
const byte ODR       = 0x1D;
const byte CONFIG    = 0x1F;

const byte READ  = 0b10000000;
const byte WRITE = 0b01111111;

const int CAL_DATA_READ_LEN = 21;
const int SENSOR_DATA_READ_LEN = 6;

typedef enum
{
  SENSOR_1,
  SENSOR_2,
  SENSOR_3,
  SENSOR_4
} sensor_ID_t;

class SensorClusterHandler {
  public:
    SensorClusterHandler(int cs1, int cs2, int cs3) {
      chipSelects[0] = cs1;
      chipSelects[1] = cs2;
      chipSelects[2] = cs3;
    }
    void begin() {
      // start the SPI library:
      SPI.begin();

      // initalize the chip select pins:
      pinMode(chipSelects[SENSOR_1], OUTPUT);
      pinMode(chipSelects[SENSOR_2], OUTPUT);
      pinMode(chipSelects[SENSOR_3], OUTPUT);

      pinMode(6, OUTPUT);

      // briefly assert chip select in order to configure BMP-388 for SPI mode
      digitalWrite(chipSelects[SENSOR_1], LOW);
      digitalWrite(chipSelects[SENSOR_2], LOW);
      digitalWrite(chipSelects[SENSOR_3], LOW);
      delay(10);
      digitalWrite(chipSelects[SENSOR_1], HIGH);
      digitalWrite(chipSelects[SENSOR_2], HIGH);
      digitalWrite(chipSelects[SENSOR_3], HIGH);

      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock

      //put to sleep (soft reset)
      writeRegister(SENSOR_1, PWR_CTRL, 0x00);
      writeRegister(SENSOR_2, PWR_CTRL, 0x00);
      writeRegister(SENSOR_3, PWR_CTRL, 0x00);

      delay(10);

      //get calibration data
      readRegisters(SENSOR_1, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
      parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_1]);

      readRegisters(SENSOR_2, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
      parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_2]);

      readRegisters(SENSOR_3, NVM_PAR_T1_7_0, CAL_DATA_READ_LEN, rxBuffer);
      parse_calib_data(&rxBuffer[2], &cal_data[SENSOR_3]);

      //Configure BMP-388:
      writeRegister(SENSOR_1, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode
      writeRegister(SENSOR_2, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode
      writeRegister(SENSOR_3, PWR_CTRL, 0x33);  // enable temp, enable press, normal mode

      SPI.endTransaction();

      // give the sensor time to set up:
      delay(500);
    }

    void readSensor(int repeat = 1, bool doCalcOffset = false) {
      double averagePressure[] = {0,0,0,0};
      double averageTemperature[] = {0,0,0,0};
      for(int i = 0; i<repeat; ++i){
        for(int s = 0; s < 3; ++s){ // !!! having 4 sensors this needs to be changed to s <= SENSOR_4
          getPressureSensorData(s, &temperature[s], &pressure[s]);
          averagePressure[s] += pressure[s];
          averageTemperature[s] += temperature[s];
        }
      }
      for(int s = 0; s < 3; ++s){
        averagePressure[s] /= repeat;
        averageTemperature[s] /= repeat;
      }

      // pressure 4 should also be averagePressure...
      if (doCalcOffset) {
        pressure[SENSOR_4] = averagePressure[SENSOR_3]; // FAKE 4th Sensor.
        offset_1 = averagePressure[SENSOR_1] - pressure[SENSOR_4];
        offset_2 = averagePressure[SENSOR_2] - pressure[SENSOR_4];
        offset_3 = averagePressure[SENSOR_3] - pressure[SENSOR_4];
      }

      relativePressureA = averagePressure[SENSOR_1] - pressure[SENSOR_4] - offset_1;
      relativePressureA *= PA_TO_CM_H2O; // convert from Pa to cmH2o
      relativePressureB = averagePressure[SENSOR_2] - pressure[SENSOR_4] - offset_2;
      relativePressureB *= PA_TO_CM_H2O; // convert from Pa to cmH2o
      relativePressureC = averagePressure[SENSOR_3] - pressure[SENSOR_4] - offset_3;
      relativePressureC *= PA_TO_CM_H2O; // convert from Pa to cmH2o
      differentialPressure = relativePressureA - relativePressureB;

      flow = calcFlow(differentialPressure);
      //flow = (differentialPressure / 100) * (10 + abs(differentialPressure / 200));


/*
       // Darrens Sensor
      const float c1 = 19.069981;
      const float c2 = 0.001515;
      const float c3 = -0.00000422945829804185;
      const float b1 = 0.969567;
      const float b2 = 0.999946;
      const float E1 = 0.517553;
      const float f = 1; // Factor orifice

      //float differentialPressure = 10; // ΔP Pa  Pressure differenctial across orifice plate
      float t_amb = 20; // T_amb degC  Ambient temperature (ideally measured upstream of orifice plate
      float p3 = 10; // P3  Pa  Downstream (of orifice plate)  gauge pressure

      //float flow = (c1*pow(diffP, E1))*(c2*t_amb+b1)*(c3*p3+b2)*f;
       flow = (c1 * pow(differentialPressure, E1)) * f/10*0.6;

*/
    }

    double calcFlow( float differentialPressure_cmH2O){
      double a = 0.73;
      double b = 7.14;
      double c = 5.02;
      double d = 0.6;
      double f = 0;
      double absDiffPress = abs(differentialPressure_cmH2O);
      if(absDiffPress<d){
        f = sqrt(absDiffPress/d)*(a*d*d+b*d+c);
      } else {
        f = a*absDiffPress*absDiffPress + b*absDiffPress + c;
      }
      if(differentialPressure_cmH2O<0) f*=-1;

      return f;
    }

    double getRelativePressureA() {
      return relativePressureA;
    }

    double getRelativePressureB() {
      return relativePressureB;
    }

    double getRelativePressureC() {
      return relativePressureC;
    }

    double getDifferentialPressure() {
      return differentialPressure;
    }

    double getPressure(sensor_ID_t sensor) {
      return pressure[sensor];
    }

    double getTemperature(sensor_ID_t sensor) {
      return temperature[sensor];
    }

    double getFlow() {
      return flow;
    }


  private:

    double temperature[3] = {0};
    double pressure[4] = {0};
    double relativePressureA;
    double relativePressureB;
    double relativePressureC;
    double differentialPressure;
    double flow;

    double offset_1;
    double offset_2;
    double offset_3;



    // ------ HELPER FUNCTIONS ------

    // pins used for the connection with the sensor
    // the other you need are controlled by the SPI library):
    int chipSelects[3] = {2, 3, 4}; //{8, 9, 10};

    byte rxBuffer[CAL_DATA_READ_LEN + 2] = {0};

    typedef struct
    {
      double par_t1;
      double par_t2;
      double par_t3;
      double par_p1;
      double par_p2;
      double par_p3;
      double par_p4;
      double par_p5;
      double par_p6;
      double par_p7;
      double par_p8;
      double par_p9;
      double par_p10;
      double par_p11;
      double t_lin;
    } bmp3_quantized_calib_data;

    bmp3_quantized_calib_data cal_data[3] = {0};



    unsigned int readRegisters(sensor_ID_t sensor, byte thisRegister, int bytesToRead, byte* rxbuff)
    {
      unsigned int result = 0;   // result to return
      int i = 0;

      // now combine the address and the command into one byte
      rxbuff[0] = thisRegister | READ;

      // take the chip select low to select the device:
      digitalWrite(chipSelects[sensor], LOW);

      // send the device the register you want to read:
      SPI.transfer(rxbuff, bytesToRead + 2);

      // take the chip select high to de-select:
      digitalWrite(chipSelects[sensor], HIGH);

      // return the result:
      return (result);
    }


    void writeRegister(sensor_ID_t sensor, byte thisRegister, byte thisValue)
    {
      // now combine the register address and the command into one byte:
      byte dataToSend = thisRegister & WRITE;

      // take the chip select low to select the device:
      digitalWrite(chipSelects[sensor], LOW);

      SPI.transfer(dataToSend); //Send register location
      SPI.transfer(thisValue);  //Send value to record into register

      // take the chip select high to de-select:
      digitalWrite(chipSelects[sensor], HIGH);
    }

    unsigned int getPressureSensorData(sensor_ID_t sensor, double* temperature, double* pressure)
    {
      unsigned long temperatureData = 0;
      unsigned long pressureData = 0;

      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // 1 MHz clock

      readRegisters(sensor, DATA_0, SENSOR_DATA_READ_LEN, rxBuffer);

      SPI.endTransaction();

      pressureData = (unsigned long)rxBuffer[2];
      pressureData |= (unsigned long)rxBuffer[3] << 8;
      pressureData |= (unsigned long)rxBuffer[4] << 16;

      temperatureData = (unsigned long)rxBuffer[5];
      temperatureData |= (unsigned long)rxBuffer[6] << 8;
      temperatureData |= (unsigned long)rxBuffer[7] << 16;

      *temperature = compensate_temperature(temperatureData, &cal_data[sensor]);
      *pressure = compensate_pressure(pressureData, &cal_data[sensor]);
    }

    static void parse_calib_data(const uint8_t *raw_cal, bmp3_quantized_calib_data *cal)
    {
      /* Temporary variable */
      double temp_var;

      uint16_t nvm_par_t1;
      uint16_t nvm_par_t2;
      int8_t nvm_par_t3;
      int16_t nvm_par_p1;
      int16_t nvm_par_p2;
      int8_t nvm_par_p3;
      int8_t nvm_par_p4;
      uint16_t nvm_par_p5;
      uint16_t nvm_par_p6;
      int8_t nvm_par_p7;
      int8_t nvm_par_p8;
      int16_t nvm_par_p9;
      int8_t nvm_par_p10;
      int8_t nvm_par_p11;

      /* 1 / 2^8 */
      temp_var = 0.00390625f;
      nvm_par_t1 = (((uint16_t)raw_cal[1] << 8) | (uint16_t)raw_cal[0]);
      cal->par_t1 = ((double)nvm_par_t1 / temp_var);

      nvm_par_t2 = (((uint16_t)raw_cal[3] << 8) | (uint16_t)raw_cal[2]);
      temp_var = 1073741824.0f;
      cal->par_t2 = ((double)nvm_par_t2 / temp_var);

      nvm_par_t3 = (int8_t)raw_cal[4];
      temp_var = 281474976710656.0f;
      cal->par_t3 = ((double)nvm_par_t3 / temp_var);

      nvm_par_p1 = (int16_t)(((uint16_t)raw_cal[6] << 8) | (uint16_t)raw_cal[5]);
      temp_var = 1048576.0f;
      cal->par_p1 = ((double)(nvm_par_p1 - (16384)) / temp_var);

      nvm_par_p2 = (int16_t)(((uint16_t)raw_cal[8] << 8) | (uint16_t)raw_cal[7]);
      temp_var = 536870912.0f;
      cal->par_p2 = ((double)(nvm_par_p2 - (16384)) / temp_var);

      nvm_par_p3 = (int8_t)raw_cal[9];
      temp_var = 4294967296.0f;
      cal->par_p3 = ((double)nvm_par_p3 / temp_var);

      nvm_par_p4 = (int8_t)raw_cal[10];
      temp_var = 137438953472.0f;
      cal->par_p4 = ((double)nvm_par_p4 / temp_var);

      nvm_par_p5 = (((uint16_t)raw_cal[12] << 8) | (uint16_t)raw_cal[11]);
      /* 1 / 2^3 */
      temp_var = 0.125f;
      cal->par_p5 = ((double)nvm_par_p5 / temp_var);

      nvm_par_p6 = (((uint16_t)raw_cal[14] << 8) | (uint16_t)raw_cal[13]);
      temp_var = 64.0f;
      cal->par_p6 = ((double)nvm_par_p6 / temp_var);

      nvm_par_p7 = (int8_t)raw_cal[15];
      temp_var = 256.0f;
      cal->par_p7 = ((double)nvm_par_p7 / temp_var);

      nvm_par_p8 = (int8_t)raw_cal[16];
      temp_var = 32768.0f;
      cal->par_p8 = ((double)nvm_par_p8 / temp_var);

      nvm_par_p9 = (int16_t)(((uint16_t)raw_cal[18] << 8) | (uint16_t)raw_cal[17]);
      temp_var = 281474976710656.0f;
      cal->par_p9 = ((double)nvm_par_p9 / temp_var);

      nvm_par_p10 = (int8_t)raw_cal[19];
      temp_var = 281474976710656.0f;
      cal->par_p10 = ((double)nvm_par_p10 / temp_var);

      nvm_par_p11 = (int8_t)raw_cal[20];
      temp_var = 36893488147419103232.0f;
      cal->par_p11 = ((double)nvm_par_p11 / temp_var);
    }

    static double compensate_temperature(unsigned long uncomp_temperature, bmp3_quantized_calib_data *cal)
    {
      double partial_data1;
      double partial_data2;

      partial_data1 = (double)(uncomp_temperature - cal->par_t1);
      partial_data2 = (double)(partial_data1 * cal->par_t2);

      /* Update the compensated temperature in calib structure since this is
         needed for pressure calculation */
      cal->t_lin = partial_data2 + (partial_data1 * partial_data1) * cal->par_t3;

      /* Returns compensated temperature */
      return cal->t_lin;
    }

    static double compensate_pressure(unsigned long uncomp_pressure_in, const bmp3_quantized_calib_data *cal)
    {
      /* Variable to store the compensated pressure */
      double comp_pressure;
      double uncomp_pressure = uncomp_pressure_in;

      /* Temporary variables used for compensation */
      double partial_data1;
      double partial_data2;
      double partial_data3;
      double partial_data4;
      double partial_out1;
      double partial_out2;

      partial_data1 = cal->par_p6 * cal->t_lin;
      partial_data2 = cal->par_p7 * (cal->t_lin * cal->t_lin);
      partial_data3 = cal->par_p8 * (cal->t_lin * cal->t_lin * cal->t_lin);
      partial_out1 = cal->par_p5 + partial_data1 + partial_data2 + partial_data3;
      partial_data1 = cal->par_p2 * cal->t_lin;
      partial_data2 = cal->par_p3 * (cal->t_lin * cal->t_lin);
      partial_data3 = cal->par_p4 * (cal->t_lin * cal->t_lin * cal->t_lin);
      partial_out2 = uncomp_pressure * (cal->par_p1 + partial_data1 + partial_data2 + partial_data3);
      partial_data1 = (uncomp_pressure * uncomp_pressure);
      partial_data2 = cal->par_p9 + cal->par_p10 * cal->t_lin;
      partial_data3 = partial_data1 * partial_data2;
      partial_data4 = partial_data3 + (uncomp_pressure * uncomp_pressure * uncomp_pressure) * cal->par_p11;
      comp_pressure = partial_out1 + partial_out2 + partial_data4;

      return comp_pressure;
    }
};
