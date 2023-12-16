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

## Web resources
- **The API of choice** is provided by Energi Data Service at https://api.energidataservice.dk/dataset/Elspotprices?start=now-PT1H&limit=35&filter=%7b%22PriceArea%22:%22DK1%22%7d&sort=HourDK%20asc. It provides data about Danish electricity prices with no strings attached, and it does so in a manner that allows different data to be accessed with the same URL depending on when it is accessed (which is very important, because as far as everyone involved in this project is concerned, it is impossible to get a Particle program to vary any part of the URL that the webhook uses).
- **The datasheet for the Particle Argon** can be accessed at https://docs.particle.io/reference/datasheets/wi-fi/argon-datasheet/.
- **The datasheet for the LM35** can be accessed at https://www.ti.com/lit/ds/symlink/lm35.pdf.
- **The program that this project uses is based on one from the Particle website.** It can be accessed at https://web.archive.org/web/20171206220525/https://docs.particle.io/tutorials/integrations/webhooks/#what-39-s-the-weather-like-.
