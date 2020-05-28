

class MotorControl {
  public:
    MotorControl(int _pinA, int _pinB) {
      pinA = _pinA;
      pinB = _pinB;
    }

    void begin() {
      pinMode(pinA, OUTPUT);
      pinMode(pinB, OUTPUT);
      enabled = true;
      setSpeed(0);
    }

    void disableMotor() {
      enabled = false;
    }

    void setSpeed(int speed) {
      if (!enabled) {
        speed = 0;
      }
      // Constrain motor
      constrain(speed, -400, 400); // Constrain to Max PWM dutycycle

      currentSpeed = speed;

      if (speed < 0) {
        speed = -speed;  // Make speed a positive quantity
        motorUp(speed * 51 / 80);
      }
      else {
        motorDown(speed * 51 / 80);
      }
    }

    int getSpeed() {
      return currentSpeed;
    }

    void home(){
//    while (getCurrent() < 600)
  //    {
  //      setMotor1Speed(150);             // reset motor position
  //      Serial.println(getCurrent());
  //    }
  //  setSpeed(0);
  }

  private:

    void motorDown(int speed) {
      analogWrite(pinA, abs(speed));
      analogWrite(pinB, 0);
    }

    void motorUp(int speed) {
      analogWrite(pinA, 0);
      analogWrite(pinB, abs(speed));
    }

    int currentSpeed;
    int pinA, pinB;
    bool enabled = false;
};
