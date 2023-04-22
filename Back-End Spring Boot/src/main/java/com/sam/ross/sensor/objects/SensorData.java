package com.sam.ross.sensor.objects;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;

@Getter
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class SensorData {
    private double voltage;
    private double current;
    private double frequency;
    private double power;
    private double avgVoltage;
    private double avgCurrent;
    private double avgFrequency;
    private double avgPower;
    private double energyConsumption;
    private double faultCounter;
    private double timeElapsed;
    private double offset;
}
