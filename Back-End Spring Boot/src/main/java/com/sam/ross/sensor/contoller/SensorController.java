package com.sam.ross.sensor.contoller;

import com.sam.ross.sensor.objects.SensorData;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;

@RestController
// specifies that should only accept HTTP requests received from the React JS web URL
@CrossOrigin(origins = {"https://smart-wireless-sensor.nw.r.appspot.com/"})
//@CrossOrigin  // uncomment to enable HTTP requests from any origin
@Slf4j
public class SensorController {
    double voltage = 0.0;
    double current = 0.0;
    double frequency = 0.0;
    double avgVoltage = 0.0;
    double avgCurrent = 0.0;
    double avgFrequency = 0.0;
    double timeElapsed = 0.0;
    double power = 0.0;
    double avgPower = 0.0;
    double energyConsumption = 0.0;
    double offset = 0.0;
    double faultCounter = 0.0;

    @GetMapping("/setReadings/{data}")
    public ResponseEntity<String> setValues(@PathVariable String data) {
        log.info("setValues endpoint has received a request (controller)");

        String[] values = data.split(",");

        for (String value: values) {
            if (value.equals("nan")) {
                return ResponseEntity.ok("\"nan value received\"");
            }
        }

        voltage = Double.parseDouble(values[0]);
        current = Double.parseDouble(values[1]);
        frequency = Double.parseDouble(values[2]);
        power = Double.parseDouble(values[3]);
        avgVoltage = Double.parseDouble(values[4]);
        avgCurrent = Double.parseDouble(values[5]);
        avgFrequency = Double.parseDouble(values[6]);
        avgPower = Double.parseDouble(values[7]);
        energyConsumption = Double.parseDouble(values[8]);
        faultCounter = Double.parseDouble(values[9]);
        timeElapsed = Double.parseDouble(values[10]);
        offset = Double.parseDouble(values[11]);

        return ResponseEntity.ok("Success: " + data);
    }

    @GetMapping("/getReadings")
    public ResponseEntity<SensorData> getValues() {
        log.info("getValues endpoint has received a request (controller)");

        SensorData sensorData = SensorData.builder()
                .voltage(voltage)
                .current(current)
                .frequency(frequency)
                .avgVoltage(avgVoltage)
                .avgCurrent(avgCurrent)
                .avgFrequency(avgFrequency)
                .timeElapsed(timeElapsed)
                .power(power)
                .avgPower(avgPower)
                .energyConsumption(energyConsumption)
                .offset(offset)
                .faultCounter(faultCounter)
                .build();

        return ResponseEntity.ok(sensorData);
    }

    @CrossOrigin    // ping requests accepted from any origin
    @GetMapping("/ping")
    public ResponseEntity<String> ping() {
        log.info("ping endpoint has received a request (controller)");

        return ResponseEntity.ok("pong");
    }

}
