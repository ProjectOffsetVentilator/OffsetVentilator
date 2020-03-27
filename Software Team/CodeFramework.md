``` 
Include
  LiquidCrystal
  Relevent servo library (if needed)

Define
	StartDelay (to adjust startup delay(should never be greater than total time between breath cycles)) (Is this needed?)
	Sensor inputs (2 pressure & 1 flow)
	User inputs (Tidal volume control, inspiratory period, start, and alarm reset)
	LCD output
	Stop switch
	Alarm pin
	
	CurrentMilis = Milis (unsigned long)
	BreathStartTime = 3974400000 (46 days in Miliseconds) (unsigned long)
	


These definitions will be used for alarms and should NOT be edited by a user
	Tidal volume (= minimum volume - 10%) (default value needs to be user adjustable though the UI ONLY)
	Breaths per minute (= 10) (default value needs to be user adjustable though the UI ONLY)
	Pressure sensor alarm level
	Flow sensor alarm level

	
Sensors 
	Map pressure sensor to usable value (cmH2O?)
	Map flow sensor to usable value 


Startup code (once start button is pressed this would be run)



Breath cycle code (Triggered by timer)



User input (Tidal volume control, inspiratory period, start, and alarm reset)



LCD Output (do we need to support display's that do not use the LiquidCrystal library?)



Alarms (all alarms set off the buzzer)
Fatal alarms (Stops operation)
	Over Pressure
	Over Flow
	Stop switch triggered (set BreathStartTime to 3974400000 (46 days))
Non fatal alarms (does not stop operation)
	Expected breath not seen
	Breath timing incorrect
	45 day's on (prevent milis reset) (Do we want to use the reset pin or just set off a alarm and display a onscreen please reset message?)
```
