#define LPF_MACRO(output, input, accumulator, filterk)  { accumulator += input; output = (int32_t)(accumulator/filterk); accumulator -=output; }

class PressureHandler {
  public:
    PressureHandler(int targetPressure_cmH2O = 35, int minPressure_cmH2O = 15, int maxPressure_cmH2O = 45) {
      setTargetPressure(targetPressure_cmH2O);
      setMinimumPressure(minPressure_cmH2O);
      setMaximumPressure(maxPressure_cmH2O);
    }

    void setTargetPressure(int cmH2O) {
      target_pressure = cmH2O;
    }

    int getTargetPressure() {
      return target_pressure;
    }

    void setMinimumPressure(int cmH2O) {
      minPressure = cmH2O;
    }

    int getMinimumPressure() {
      return minPressure;
    }

    void setMaximumPressure(int cmH2O) {
      maxPressure = cmH2O;
    }

    int getMaximumPressure() {
      return maxPressure;
    }


    int update(int currentPressure_cmH2O) {
      currentPressure = currentPressure_cmH2O;

      LPF_MACRO(currentPressureFiltered, currentPressure, accumulator, 4); // last number is the filter coefficent

    }


    int getPressure() {
      return currentPressure;
    }

    int getFilteredPressure() {
      return currentPressureFiltered;
    }

    boolean checkMinMax() {
      return  (minPressure < currentPressure) && (currentPressure < maxPressure);
    }


  private:
    // Variables for pressure
    int target_pressure;
    int minPressure;
    int maxPressure;
    int currentPressure;
    long currentPressureFiltered;

    // for filter
    long accumulator;
};
