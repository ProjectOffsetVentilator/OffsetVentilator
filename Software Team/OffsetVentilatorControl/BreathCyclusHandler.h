class BreathCyclusHandler {
public:
    BreathCyclusHandler(int bpm = 20, int iEval = 2) {
      setTargetBPM(bpm);
      setIEval(iEval);
    }

    // Breath preiod & timing

    setTargetBPM(int bpm) {
      target_bpm = bpm;
      calc_breathPeriod = (60 / target_bpm) * 1000;
      calc_inhalePeriod = calc_breathPeriod / (target_ieVal + 1);
    }

    setIEval(int ieval) {
      // inhale exhale proportion
      target_ieVal = ieval;  // 1:IEval
      setTargetBPM(target_bpm); // recalculate breath- & inhalePeriod
    }

    int getTargetBPM() {
      return target_bpm;
    }

    int getTargetBreathPeriod() {
      return calc_breathPeriod;
    }

    int getTargetInhalePeriod() {
      return calc_inhalePeriod;
    }

    void startCycle(unsigned long ms){
      startTimeStamp = ms;
    }

    void update(unsigned long ms){
      currentTime = ms;
    }

    boolean isInhale(){
      return currentTime-startTimeStamp < calc_inhalePeriod;
    }

    boolean isExhale(){
      return currentTime-startTimeStamp > calc_inhalePeriod;
    }

    boolean isFinished(){
      return currentTime-startTimeStamp > calc_breathPeriod;
    }

    /*  TODO
     *  Actual readings of BPM
     *  Actual readings of IEVal
     */
    

  private:
    // Variables for breath preiod & timing
    int target_bpm;
    int target_ieVal = 2;                

    int calc_breathPeriod;
    int calc_inhalePeriod;


    unsigned long startTimeStamp;
    unsigned long currentTime;
    
    //unsigned long endInhaleTimeStamp;
    //unsigned long endExhaleTimeStamp;

};
