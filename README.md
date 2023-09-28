# Semester project in EH5IOT (fall 2023): Power scheduler


## Basic concept
The power scheduler is a proposed device that can cut the power to another device that costs a lot of power to use and does not need to be supplied with power continuously. It will use this ability to ensure that the device it is connected to is supplied specifically when power is at its cheapest.

It will be implemented through a Particle Argon, whose IoT capabilities will be used to acquire the current power prices, and whose computational capabilities will be used to schedule the times of day where the power scheduler will allow the target device to be powered.

## External requirements
**Sensors:** An amperemeter can be used for feedback purposes to see how much power the target device manages to consume. A sensor can also be used to determine the target device's need, like in the case of a refrigerator or freezer, where a temperature sensor can be used to determine if the target device is (and will remain) cold enough for now.

**Actuators:** A relay is expected to be the means by which the power scheduler enables and disables the flow of current into the target deevice.

**IoT elements:** An API will be used to collect power price data. There should be multiple APIs that provide data on Danish power prices free of charge.

## Internal requirements
### Functional requirements
- The device must be able to sever another device's power supply and restore it later.
- The device must be able to acquire power prices from the Internet. 
- The device must be able to use known power prices to find out how to keep another device powered as long and as often as needed at the lowest possible cost.

### Optional requirements
- The device should be able to function with any target device. Failing that, the device should be able to function with any target device within as wide a category as reasonably possible, such as any device that converts electrical energy to thermal energy.

## Intended resources
- **https://www.elprisenligenu.dk/elpris-api** claims to provide the currently known Danish power prices free of charge with no strings attached. This makes it a prime candidate for an API to get power prices from.

## Current goals
- Get the Argon to acquire power price data.
- Get the Argon to sort power price data in ascending order based on price, then mark the hours with the lowest _n_ prices.
