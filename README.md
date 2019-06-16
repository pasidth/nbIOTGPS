# nbIOTGPS
GPS using nbIOT from AIS
This project is testing how the nbIOT shield from AIS (Thailand) could be use in my project.
I connected GPS module and DHT11 sensor then send all data in json format to magellan platform provided by AIS.
Wiring:
1. nbIOT connect to Arduino Mega 2560
   pin D8 (nbIOT shield) --> pin D48 (mega)
   pin D6 (nbIOT shield) --> pin D46 (mega)
2. neo GPS module connect to Mega 2560 using software serial
   pin RX (GPS) --> pin D10 (mega)
   pin TX (GPS) --> pin D11 )mega)
3. DHT11 signal connect to pin D6 (mega)
Configuration:
1. goto magellan platform
   https://www.aismagellan.io/authentication/signin
2. Sign up to be a member and then Sign In
3. Creat things and use Token Key into Arduino program
4. Comply the code and upload to MEGA
5. All data will be displayed on the screen after the nbIOT connected to the network.
6. Then create Dashboard and create widget. Choose widget type as you wish.
enjoy with your result.
