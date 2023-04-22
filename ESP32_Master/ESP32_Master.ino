#include <Adafruit_ADS1X15.h>

#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>

#define I2C_SLAVE_ADDR 0x04

Adafruit_ADS1015 ads1;
Adafruit_ADS1015 ads2;

// This is required on ESP32 to put the ISR in IRAM.
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

// Pointer to the timer
hw_timer_t *My_timer = NULL;

volatile bool new_data = false;
void IRAM_ATTR onTimer() {
  new_data = true;
}

void setup(void)
{
  // Set-up the timer interrupt and start it
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 833, true);
  timerAlarmEnable(My_timer);

  Serial.begin(2000000);

  // ADS1015 ADCs
  ads1.setDataRate(RATE_ADS1015_3300SPS);
  ads2.setDataRate(RATE_ADS1015_3300SPS);

  if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS1015 1.");
    ESP.restart();
  }

  if (!ads2.begin(0x49)) {
    Serial.println("Failed to initialize ADS1015 2.");
    ESP.restart();
  }

  // Start differential conversions.
  ads1.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);
  ads2.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/true);

  // GPS serial setup
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // set up GPS module serial communication on 9600 baud
}

void loop(void)
{
  unsigned long startTime;
  unsigned long timeTaken;
  unsigned long prevPositiveZeroCrossing = micros();
  unsigned long prevTime = micros();

  static unsigned long lastWireTransmit = 0;
  static byte x = 0;

  int bufferSize = 400;
  float sumSquareVoltages = 0.0;
  float sumSquareCurrents = 0.0;
  float vrms = 0.0;
  float irms = 0.0;
  float avgScaledVRMS = 0.0;
  float avgScaledIRMS = 0.0;

  float scaledVoltage = 0.0;
  float scaledCurrent = 0.0;
  float scaledVRMS = 0.0;
  float scaledIRMS = 0.0;

  float scaledPower = 0.0;
  float avgScaledPower = 0.0;
  float timeElapsed = 0.0;
  float energyConsumption = 0.0;
  float faultCounter = 0.0;

  float bufferV[bufferSize];
  float bufferC[bufferSize];
  float bufferM[bufferSize];

  float prevVoltage = 0.0;
  float currentFrequency = 0.0;
  float avgFrequency = 0.0;

  int cycleCounter = 1; // used to count the number of full cycles that have occurred for the running average frequency calculations

  float freqMidPoint = 2.5;
  float midPoint = 2.5;
  float midPointAvg = 0.0;

  float primarySideVoltagePeak = 11000.0;
  float primarySideFaultCurrentPeak = 200.0;

  boolean faultOccuring = false;
  boolean prevFaultOccuring = false;



  // --------------- gps syncing ---------------
  // clear the serial
  while (Serial2.available() > 0) {
    Serial2.read();
    delay(1);
  }

  // wait for the start of a new second
  while (Serial2.available() == 0) {

  }

  // warmup - starting at the exact start of a new second
  for (int i = 0; i < 100 ; i++) {
    while (!new_data) {

    }
    ads1.getLastConversionResults();
    ads2.getLastConversionResults();
  }


  startTime = millis();
  // 1000 seconds
  for (int i = 0; i < (1200000 * 36 * 24) ; i++) {

    // Tries to send new data to the slave once per second
    if (!new_data && millis() - lastWireTransmit > 1000 && i > 100) {
      // first create a WirePacker that will assemble a packet
      WirePacker packer;
      char values[250];

      // First row
      dtostrf(scaledVoltage, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(scaledCurrent, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(currentFrequency, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(scaledPower, -10, 3, values);
      packer.write(values);
      packer.write(",");


      // Second row
      dtostrf(avgScaledVRMS * sqrt(2), -10, 3, values); // avgScaledVoltage
      packer.write(values);
      packer.write(",");

      dtostrf(avgScaledIRMS  * sqrt(2), -10, 3, values);  // avgScaledCurrent
      packer.write(values);
      packer.write(",");

      dtostrf(avgFrequency, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(avgScaledPower, -10, 3, values);
      packer.write(values);
      packer.write(",");


      // Third row
      dtostrf(energyConsumption, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(faultCounter, -10, 1, values);
      packer.write(values);
      packer.write(",");

      dtostrf(timeElapsed, -10, 3, values);
      packer.write(values);
      packer.write(",");

      dtostrf(midPoint, -10, 3, values);
      packer.write(values);
      packer.write(",");


      // close the packet
      packer.end();

      // transmit the packed data
      Wire.beginTransmission(I2C_SLAVE_ADDR);
      while (packer.available()) {    // write every packet byte
        int content = packer.read();
        Wire.write(content);
      }
      Wire.endTransmission();         // stop transmitting
      lastWireTransmit = millis();
    }

    // waits until new data is taken from the ADC at each timer interrupt
    while (!new_data) {
    }

    int16_t resultsADS1 = ads1.getLastConversionResults();
    float voltage = ads1.computeVolts(resultsADS1);

    int16_t resultsADS2 = ads2.getLastConversionResults();
    //    float current = ads2.computeVolts(resultsADS2);
    float current = ((ads2.computeVolts(resultsADS2) - midPoint) * 0.098) + midPoint; // remove
    //    float current = ((ads2.computeVolts(resultsADS2) - midPoint) * 0.95) + midPoint; // remove


    // --------------------------- VRMS -----------------------------

    if (i < bufferSize) {
      // midPoint
      midPointAvg = ((midPointAvg * i) + voltage) / (i + 1);
      bufferM[i] = midPoint;

      // vrms
      bufferV[i] = voltage - midPoint;
      sumSquareVoltages += pow(voltage - midPoint, 2);
      vrms = sqrt(sumSquareVoltages / (i + 1));

      // irms
      bufferC[i] = current - midPoint;
      sumSquareCurrents += pow(current - midPoint, 2);
      irms = sqrt(sumSquareCurrents / (i + 1));
    } else {
      int pos = i % bufferSize;
      float oldM = bufferM[pos];
      float oldV = bufferV[pos];
      float oldC = bufferC[pos];

      // midPoint
      midPointAvg = ((midPointAvg * bufferSize) + voltage - (oldV + oldM)) / bufferSize;
      midPoint = midPointAvg;
      bufferM[pos] = midPoint;

      // vrms
      sumSquareVoltages = (pow(vrms, 2) * bufferSize) - pow(oldV, 2) + pow(voltage - midPoint, 2);
      vrms = sqrt(sumSquareVoltages / bufferSize);
      bufferV[pos] = voltage - midPoint;

      // irms
      sumSquareCurrents = (pow(irms, 2) * bufferSize) - pow(oldC, 2) + pow(current - midPoint, 2);
      irms = sqrt(sumSquareCurrents / bufferSize);
      bufferC[pos] = current - midPoint;
    }

    // scale the voltage and current values to the primary side values
    scaledVRMS = (vrms * primarySideVoltagePeak) / sqrt(2);
    scaledVoltage = (scaledVRMS * sqrt(2)); // 11000V == 1.414V

    scaledIRMS = (irms * primarySideFaultCurrentPeak) / sqrt(2);
    scaledCurrent = (scaledIRMS * sqrt(2)); // 200A == 1.414V

    scaledPower = scaledVoltage * scaledCurrent;  // W
    avgScaledPower = ((avgScaledPower * i) + scaledPower) / (i + 1);  // W

    avgScaledVRMS = ((avgScaledVRMS * i) + scaledVRMS) / (i + 1);
    avgScaledIRMS = ((avgScaledIRMS * i) + scaledIRMS) / (i + 1);


    // ------------------------- Frequency ---------------------------

    long currentTime = micros();
    // if at zero crossing (zero crossing from negative to positive specifically)
    if (prevVoltage <= freqMidPoint && voltage > freqMidPoint) {
      long trueZero = currentTime - ((currentTime - prevTime) * ((voltage - freqMidPoint) / (voltage - prevVoltage)));

      currentFrequency = 1000000.0 / (trueZero - prevPositiveZeroCrossing);

      if (i > 100) {
        avgFrequency = ((avgFrequency * cycleCounter) + currentFrequency) / (cycleCounter + 1);
        cycleCounter++;
      } else {
        avgFrequency = currentFrequency;
      }

      prevPositiveZeroCrossing = trueZero;
    }



    // needs 100 samples warmup
    if ( i > 100 ) {
      // energy consumption
      timeElapsed = (millis() - startTime) / 1000;  // s
      energyConsumption = avgScaledPower * timeElapsed / 3600000; // kWh


      // ----------------------- Fault Detection -------------------------
      if (scaledCurrent >= 100.0) { // trip point set at 100A
        faultOccuring = true;
      } else if (scaledCurrent < 40.0) { // fault finished value boundary set at 40 to ensure fault
        faultOccuring = false;            // has truly finished, rather than recounting the same fault twice
      }

      // checks for the very start of a fault
      if (prevFaultOccuring == false && faultOccuring == true) {
        faultCounter++;
      }

      prevFaultOccuring = faultOccuring;


      // -------------------------- Plotting ----------------------------

      //
      //      Serial.print("Frequency:");
      //      Serial.print(currentFrequency);
      //      Serial.print(" ");
      //
      //      Serial.print("ScaledPower:");
      //      Serial.print(scaledPower, 3);
      //      Serial.print(" ");
      //
      //
      //
      //      Serial.print("AvgScaledVoltage:");
      //      Serial.print(avgScaledVRMS * sqrt(2));
      //      Serial.print(" ");
      //
      //      Serial.print("AvgScaledCurrent:");
      //      Serial.print(avgScaledIRMS * sqrt(2));
      //      Serial.print(" ");
      //
      //      Serial.print("AvgFrequency:");
      //      Serial.print(avgFrequency);
      //      Serial.print(" ");
      //
      //      Serial.print("AvgPower:");
      //      Serial.print(avgScaledPower, 3);
      //      Serial.print(" ");
      //
      //
      //
      //
      //      Serial.print("EnergyConsumptionkWh:");
      //      Serial.print(energyConsumption);
      //      Serial.print(" ");
      //
      //      Serial.print("TimeElapsed:");
      //      Serial.print(timeElapsed);
      //      Serial.print(" ");
      //



      //      Serial.print("ScaledVRMS:");
      //      Serial.print(scaledVRMS);
      //      Serial.print(" ");
      //
      //      Serial.print("ScaledIRMS:");
      //      Serial.print(scaledIRMS);
      //      Serial.print(" ");



      Serial.print("ADC1Voltage:");
      Serial.print(voltage);
      Serial.print(" ");

      Serial.print("ADC2Current:");
      Serial.print(current);
      Serial.print(" ");

      Serial.print("ScaledVoltage:");
      Serial.print(scaledVoltage);
      Serial.print(" ");

      Serial.print("ScaledCurrent:");
      Serial.print(scaledCurrent);
      Serial.print(" ");

      Serial.print("FaultCounter:");
      Serial.print(faultCounter);
      Serial.print(" ");

//      Serial.print("AvgFrequency:");
//      Serial.print(avgFrequency);
//      Serial.print(" ");

      Serial.print("Frequency:");
      Serial.print(currentFrequency);
      Serial.print(" ");

      Serial.print("MidPoint:");
      Serial.println(midPoint);
      //      Serial.print(" ");



      //      Serial.print("AvgScaledVRMS:");
      //      Serial.print(avgScaledVRMS);
      //      Serial.print(" ");
      //
      //      Serial.print("AvgScaledIRMS:");
      //      Serial.println(avgScaledIRMS);




    }

    prevVoltage = voltage;
    prevTime = currentTime;

    new_data = false;
  }

  timeTaken = millis() - startTime;




}
