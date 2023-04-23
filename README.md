# Smart-Wireless-Sensor-Project

Final year project completed over the course of November 2022 - April 2023



Report: [Smart-Wireless-Sensor-Final-Report.pdf](https://github.com/sam-ross/Smart-Wireless-Sensor-Project/files/11301789/Smart-Wireless-Sensor-Final-Report.pdf)

Website: https://smart-wireless-sensor.nw.r.appspot.com/


## Abstract

This project aims to research, design, and develop an embedded smart wireless sensor solution that
can augment individual three-phase system Digital Fault Recorders (DFRs) to solve their main two
problems – the DFR recordings and other useful near-real-time measurements/metrics cannot be
accessed/retrieved remotely and secondly, it may be electrically impossible to install DFRs in certain
substation scenarios due to their hardwiring requirements and size constraints.

The ESP32-DevKitC-32UE MCU was concluded as the optimal prototyping platform for the project. A
range of other components were acquired, including two ADS1015 ADCs for taking voltage readings,
a MicroSD card breakout for storing the useful live measurements, a GPS breakout for interrupt
timer disciplining and a secondary ESP32 for handling external communications. Two signal
conditioning circuits were then designed and engineered to attenuate the secondary-side voltage
(VT) and current (CT) signals down to 1VRMS, which were then fed into the two ADCs. 

The 1200sps readings taken by the ADCs are then run through various validated algorithms to calculate the 12
final useful live measurements. These 12 near-real-time measurements are then displayed on the
secondary ESP32 Wi-Fi Access Point & Station Web Servers, along with being sent through a backend
pipeline to a cloud-hosted React JS website and stored in cloud data storage. The true frequency and
VRMS algorithms were validated through a simulation in MATLAB and found to be over 33 times
more accurate than originally required (+-0.3mHz and +-0.3mV, respectively).

<br/>


## Smart Wireless Sensor Circuit Diagram
![Full ESP32 Circuit Diagram](https://user-images.githubusercontent.com/67061245/233792437-f0c9193b-4c64-401d-b262-2def480e407d.jpg)

<br/><br/>


## ESP32 External Communications Diagram
![ESP32 External Communications](https://user-images.githubusercontent.com/67061245/233795433-3cacb573-c1a4-4925-8e77-56079dd9c734.png)
