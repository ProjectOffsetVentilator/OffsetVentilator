/*

   Simple class to calculate volume from incomming flow data

   Functions for integration are taken from the example FlowOnlyCalibration
   from the SFM3X00 lirary by Public Inventions
   https://github.com/PubInv/SFM3x00

*/


#define MINIMUM_FLOW 0.04

class FlowHandler {

  public:
    FlowHandler(float maxTidal_ml_m = 150, float minTidal_ml_m = 800) {
      maxTidal = maxTidal_ml_m;
      minTidal = minTidal_ml_m;
      zero_integration(millis());
      zero_range();
    }

    void update(float raw_flow_slm) {
      unsigned long ms = millis();

      min_recorded_flow = min(min_recorded_flow, raw_flow_slm);
      max_recorded_flow = max(max_recorded_flow, raw_flow_slm);


      // if the flow is less than 0.04 then round to 0
      if (abs(raw_flow_slm) < MINIMUM_FLOW)
      {
        raw_flow_slm = 0;
      }

      float flow_milliliters_per_minute =  (raw_flow_slm * 1000.0);

      totalVolume = add_to_running(totalVolume, ms, flow_milliliters_per_minute);
      if (flow_milliliters_per_minute > 0) {
        inhalationVolume = add_to_running(inhalationVolume, ms, flow_milliliters_per_minute);
      } else {
        exhalationVolume = add_to_running(exhalationVolume, ms, flow_milliliters_per_minute);
      }
    }


    void zero_integration(unsigned long ms) {
      totalVolume = 0;
      inhalationVolume = 0;
      exhalationVolume = 0;
      last_ms = ms;
      last_flow = 0;
    }

    void zero_range() {
      max_recorded_flow = 0.0;
      min_recorded_flow = 0.0;
    }

    float getTidalVolume() {
      return totalVolume;
    }

    float getTotalVolume() {
      return totalVolume;
    }

    float getInhalationVolume() {
      return inhalationVolume;
    }

    float getExhalationVolume() {
      return exhalationVolume;
    }

    float getMinFlow() {
      return min_recorded_flow;
    }
    float getMaxFlow() {
      return max_recorded_flow;
    }

    boolean checkTidalMinMax(){
      return  (minTidal < inhalationVolume) && (inhalationVolume < maxTidal);
    }

  private:
    // Note: v units are milliliters
    float add_to_running(float v, unsigned long ms, float flow_millilters_per_minute) {
      float f = flow_millilters_per_minute;
      // Use a basic quadrilateral integration
      // we'll treat a very small flow as zero...
      float ml_per_ms = f / (60.0 * 1000.0);

      v += (ms - last_ms) * (ml_per_ms + last_flow) / 2.0;

      last_ms = ms;
      last_flow = ml_per_ms;

      return v;
    }


  private:
    float totalVolume = 0.0;
    float inhalationVolume = 0.0;
    float exhalationVolume = 0.0;
    float last_ms = 0.0;
    float last_flow = 0.0;

    float max_recorded_flow = 0.0;
    float min_recorded_flow = 0.0;

    float minTidal = 0.0;
    float maxTidal = 0.0;
};
