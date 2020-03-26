```
Include
  LiquidCrystal

Define
	Tidal volume (default) (default value needs to be user adjustable)
	Inspiratory period (default) (default value needs to be user adjustable)
	StartDelay (to adjust startup delay(should never be greater than total time between breath cycles))
	Sensor inputs (2 pressure & 1 flow)
	User inputs (Tidal volume control, inspiratory period, start, and alarm reset)
	LCD output
	Stop switch
	Alarm pin
	


Do NOT define
	Pressure sensor alarm level
	Flow sensor alarm level


Startup code (once start button is pressed this would be run)



Breath cycle code (Triggered by timer)



User input (Tidal volume control, inspiratory period, start, and alarm reset)



LCD Output (do we need to support display's that do not use the LiquidCrystal library?)



Alarm code
	Over Pressure
	Over Flow
	Stop switch triggered```
	
Milis () Will not work due to memory limitations -Ammon
