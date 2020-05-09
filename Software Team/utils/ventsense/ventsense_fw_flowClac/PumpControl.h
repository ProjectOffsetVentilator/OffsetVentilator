class PumpControl {
  public:
    PumpControl(int pwmPin) {
      mPwmPin = pwmPin;
    }

    void init() {
      //TCCR0B = TCCR0B & B11111000 | B00000001; // for PWM frequency of 62500.00 Hz
      //TCCR0B = TCCR0B & B11111000 | B00000010; // for PWM frequency of 7812.50 Hz
      //TCCR0B = TCCR0B & B11111000 | B00000011; // for PWM frequency of 976.56 Hz (The DEFAULT)
      TCCR0B = TCCR0B & B11111000 | B00000100; // for PWM frequency of 244.14 Hz

      pinMode(mPwmPin, OUTPUT);  // sets the pin as output
      analogWrite(mPwmPin, 0 );
    }

    void setPwm(int v) {
      analogWrite(mPwmPin, v);
    }

    void toggleProcedure() {
      reset();
      doAirProcedure = !doAirProcedure;
    }

    void reset() {
      cntSpeedUpMeasures = 0;
      calibState = 0;
      analogWrite(mPwmPin, 0 );
    }

    void update() {
      if (doAirProcedure) {
        if (calibState < calibStates) {
          int outputStrength = calibState * calibStateJump + minStrength;
          analogWrite(mPwmPin, outputStrength);

          if (cntSpeedUpMeasures < speedUpMeasures) {
            cntSpeedUpMeasures++;
          } else {
            cntSpeedUpMeasures = 0;
            calibState++;
          }
        } else {
          doAirProcedure = false;
          analogWrite(mPwmPin, 0);
        }
      }
    }

  private:
    int mPwmPin;
    // Calibration procedure
    boolean doAirProcedure;
    const int calibStates = 100;
    const int speedUpMeasures = 5;
    int cntSpeedUpMeasures = 0;
    int calibState = 0;
    unsigned long calibStartTime;

    const int minStrength = 30;
    const int maxStrength = 255;
    const int calibStateJump = (maxStrength - minStrength) / calibStates;
};
