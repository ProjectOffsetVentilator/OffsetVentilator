Ok, so I am not very knowledgeable when it comes to small electric motors, Though I think based on the points below that using a servo motor is most suitable for this project.

# Types of Motors

## 1. Brushed and Brushless DC Motors
DC motors are electromagnetic devices that use the interaction of magnetic fields and conductors to convert electrical energy to mechanical energy for rotation. There are many types of DC motors out in the market. The brushed and brushless motors are the most common DC motors.

Brushed DC Motors
The brushed DC motor have been around for a long time, and its use can be traced back to the 1830s. They can be found just about anywhere. In toys, household appliances, computer cooling fans, you name it. As one of the simplest motors to construct and control, it is no wonder that the brushed DC motor still remains as a favorite among professionals and hobbyists alike.
Why are they called brushed motors? The current is provided via two stationary metallic brushes that make contact with the different segments on the ring. As the commutator rotates, the brushes make contact with the next segment and therefore continue the rotation of the motor. As you can imagine, this generates friction and so heat and even sparks are generated.
How does a DC motor move? DC motors consist of coils connected to segments of a ring, or commutator. The coils are surrounded by a pair of magnets, or a stator, that envelopes the coils in an electric field. If you remember from your physics classes, when current is passed through a wire in a magnetic field, the wire experiences a force, and so the coils in the motor experience a force that pushes the coil and begins the rotation. The GIF illustrates the working principle of the brushed motor. The coil experiences a downward force when it reaches the area on the right, and an upward force when it reaches the area on the left. By adding multiple coils attached to different segments on the commutator, steady rotation can be maintained. The direction of rotation can be reversed simply by reversing the polarity on the motor’s contacts.

### Advantages & Limitations:

### Advantages

* Simple to control Controlling a brushed DC motor is as simple as a switch. Simply apply a voltage to start driving them. They slow down when the voltage is lowered, and spin in the other direction when the voltage is reversed.
* Excellent torque at low speeds
* High torque is achieved at low speeds.
* Reasonably Efficient
* Brushed DC motors are about 75-80% efficient.
* Inexpensive, A typical brushed DC motor at the Seeed Bazaar costs only $2.55.

### Limitations

* Noise
* Aside from the audible noise from the rubbing parts, electromagnetic noise is also generated as a result of the strong sparks that occur at areas where the brushes pass over the gaps in the commutator. This can potentially cause interference in other parts of the system.
* Constant Maintenance
* Brushes could get easily worn out as a result of continuous moving contact and require constant maintenance. Speed could be limited due to brush heating.

### Brushless DC Motors

Brushless DC motors are mechanically simpler than brushed ones. As commutation is achieved electrically, the sparks and noise of brushed DC motors is eliminated, enabling the current flow to switch silently and therefore allowing the motor to be driven quietly. These quiet motors find applications in computer fans, disk drives, drones, electric vehicles and high-precision servomechanisms. The brushless DC motor only has one moving component – the rotor, which eliminates the complications caused by brushes in brushed motors. And also unlike brushed motors, the rotor consists of a ring of permanent magnets, whereas the coils are stationary. This set-up eliminates the need for brushes. The difficult part comes in controlling the polarity of the current flowing through the coils and keeping this in sync with the speed of the rotor. This can be achieved by measuring back EMF or using Hall effect sensors to directly measure the position of the magnets. Due to this, brushless DC motors are typically more expensive and complex, in spite of the numerous advantages it has over brushed DC motors.

### Advantages & Limitations:
### Advantages

* Quiet, They generate less electrical noise compared to brushed motors as no brush is used. Hence, brushless DC motors are often preferred in applications where it is important to avoid electrical noise.
* Efficient, Brushless DC motors are more efficient than brushed motor, as they are able to continuously achieve maximum rotational force/torque. Brushed motors in contrast, will reach maximum torque only at certain points during the rotation. For a brushed motor to achieve the same torque as a brushless motor, it would require a larger magnet.
* Require less maintenance Brushless DC motors offer high durability as there are no brushes to be replaced.

### Limitations

* Controller, Some brushless motors are difficult to control and require a specialized regulator

## 2. Stepper Motors
Stepper motors are motors that move in slow, precise and discrete steps. Valued for their precise position control, they find a myriad of applications such as desktop printers, security cameras, and CNC milling machines. Stepper motors have a controller system that sends electrical pulses to a driver, which interprets these pulses and sends a proportional voltage to the motor. The motor then moves in accurate and fixed angle increments, hence the name “stepper”. The stepper motor works similarly to brushless DC motors, except that it moves in much smaller steps. Its only moving part is also the rotor, which contains the magnets. The polarity of each coil is controlled by an alternating current. As the polarity changes, each coil is given a push or a pull effect, thus moving the motor.
They can be controlled with commonly available and cheap microcontrollers. However, the stepper motor is a power-hungry device that constantly draws maximum current. The small steps it takes also means that it has a low top speed, and steps can potentially be skipped when high loads are used.

2. Stepper Motors
Stepper motors are motors that move in slow, precise and discrete steps. Valued for their precise position control, they find a myriad of applications such as desktop printers, security cameras, and CNC milling machines.

Working Principle:

An operating Stepper motor
(Source: emmeshop)
Stepper motors have a controller system that sends electrical pulses to a driver, which interprets these pulses and sends a proportional voltage to the motor. The motor then moves in accurate and fixed angle increments, hence the name “stepper”. The stepper motor works similarly to brushless DC motors, except that it moves in much smaller steps. Its only moving part is also the rotor, which contains the magnets. The polarity of each coil is controlled by an alternating current. As the polarity changes, each coil is given a push or a pull effect, thus moving the motor.

They can be controlled with commonly available and cheap microcontrollers. However, the stepper motor is a power-hungry device that constantly draws maximum current. The small steps it takes also means that it has a low top speed, and steps can potentially be skipped when high loads are used.

### Advantages & Limitations:
### Advantages

* Precise positioning, Stepper motors have a high pole count, usually from 50 to 100, and can accurately move between their many poles without the aid of a position encoder. As they move in precise steps, they excel in applications requiring precise positioning such as 3D printers, CNC, camera platforms and X, Y plotters.
* Precise speed control, Precise increments in movement enables excellent speed control, making them a good choice in process automation and robotics.
* Excellent torque characteristics at low speeds, Stepper motors have maximum torque at low speeds (less than 2000 rpm), making them suitable for applications that need low speed with high precision. Normal DC motors and servo motors do not have much torque at low speeds.
* Excellent torque to maintain position, Suitable for applications with high holding torque.
* Easy to control, Stepper motors can be easily controlled with microcontrollers such as the ATmega chips that are readily available on Arduino development boards.

### Limitations

* Noise, Stepper motors are known to generate some noise during operation. Thus, if your device needs to be quiet, accommodate a high range of speeds and torques and maintain a reasonable efficiency, then consider using a DC motor. But if your motion control application needs to be built quickly, does not need to be efficient, and a little noise is acceptable, then a stepper motor might be more suitable.
* Limited high-speed torque, Generally, stepper motors have less torque at high speeds than at low speeds. Some steppers can be optimized for better torque at high speeds, but a driver would have to be paired with it to achieve that performance.
* Low Efficiency, Unlike DC motors, the current consumption of stepper motors is independent of load and they constantly draw maximum current. As such, they tend to become hot.
* Might skip steps, As stepper motors have a low top-speeds, they might skip steps at high loads.

## 3. Servo Motors

Servo motors are motors capable of providing very precise motion control. The feedback in a servo motor system senses the difference between the actual and desired speed or position so that the controller can adjust the output to correct any drift from the target position. The positional rotation and continuous rotation are two basic types of servo motors
Working Principle:
The servo motor consists of a DC motor. DC motors spin at high rpm and very low torque. However, inside a servo motor, there is an arrangement of gears that will take the high speed of the internal DC motor and slow it down, while at the same time increasing the torque. In other words, the gear design rotation speed of the servo is much slower but with more torque. The amount of work is the same, but just more useful. Gears in a cheap servo are typically made of plastic to keep it light, and to keep the costs down. But for servo motors designed to provide more torque for heavier work, the gears are made of metal instead.
A servo contains a positional sensor, or encoder, on the final gear. Based on closed-loop control, the microcontroller compares the actual position of the rotor to the desired position and generates an error signal. This error signal is then used to generate the appropriate control signal to move the rotor to the final position. More sophisticated servos also measure speed to provide more precise and smoother movement.
Positional rotation servos – Widely used for small-scale projects where moderate precise positioning is required, this is the most common and inexpensive type of servo motor. This servo motor rotates within a 180 degrees range. They do not provide speed control or continuous rotation. It has physical stops built into the gear mechanism to prevent turning beyond these limits to protect the rotational sensor.
Continuous rotation servos – Unlike the positional rotation servos, the continuous rotation servo can turn clockwise or anti-clockwise continuously, at varying speeds depending on the command signal.

### Advantages & Limitations:
### Advantages

* Excellent torque characteristics at high speeds, At speeds greater than 2000 rpm, servo motors have high torque and are best suited for applications with high speeds and high torque applications that involve dynamic load changes. Servo motors can generate a higher peak torque as they are able to operate at higher speeds. This is because servo motors operate under a constant closed-loop feedback mechanism as opposed to the open-loop system of a stepper motor, which allows it to reach higher speeds and generate higher peak torque.
* Variety, They come in many different sizes and torque ratings.
* Inexpensive, Small sized servos only costs a few dollars. Many servo motors have gears that are made of plastic to keep them light and at the same time, cheap.

### Limitations

* Limited range of motion
* Positional rotation servos are limited to 180 degrees of motion.
* Jitter, The feedback mechanism in the servo will constantly try to correct any drift from the desired position. This constant adjustment results in twitching while trying to hold a steady position. As such, a stepper motor could be considered instead if this is a problem for your application.
* Servo motors find uses in many places, from simple toys to everyday objects and automobiles. Servos are used in remote-controlled cars where they are used to control the steering, and in DVD disc players to extend and retract disc trays. They are also extensively used in robotics for moving joints. With a variety of shapes and sizes available, you can be assured that you will find the right servo for your application.
