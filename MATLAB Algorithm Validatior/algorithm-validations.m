clear all;
clc;

fs = 1200; % sampling frequency
dt = 1/fs; % seconds per sample

f = [49.97, 49.98, 49.97];
cycles = [50, 150, 70];

stopTimeTotal = 0; 
totalSamples = 1;
for i = 1:length(f)
    stopTimes(i) = cycles(i) / f(i);
    samples(i) = stopTimes(i) * fs;
    stopTimeTotal = stopTimeTotal + stopTimes(i);
    totalSamples = totalSamples + samples(i);
end

t = (0:dt:stopTimeTotal);

for i = 1:totalSamples
    if (i <= samples(1) + 1) 
        y(i) = sqrt(2) * sin(2 * pi * f(1) * t(i));
        yTrue(i) = f(1);
    elseif (i <= samples(1) + samples(2) + 1)
        y(i) = sqrt(2) * sin(2 * pi * f(2) * t(i));
        yTrue(i) = f(2);
    elseif (i <= samples(1) + samples(2) + samples(3) + 1) 
        y(i) = sqrt(2) * sin(2 * pi * f(3) * t(i));
        yTrue(i) = f(3);
    end
end

plot(t, y);

hold on

prevVoltage = 0.0;
prevTime = 0.0;
currentFrequency = 0.0;
disp(currentFrequency)

midPoint = 0;
prevPositiveZeroCrossing = -1;

buffer = y(1:24);
disp(buffer);
sumSquareVoltages = 0;
for i = 1:length(buffer)
    sumSquareVoltages = sumSquareVoltages + buffer(i)^2;
    disp(sumSquareVoltages);
    vrmsPlot(i) = 0;
end
vrms = sqrt(sumSquareVoltages / length(buffer));

for i = 1:length(y)
    voltage = y(i);
    currentTime = t(i);

    % VRMS
    if (i > length(buffer)) 

        pos = mod(i-1, length(buffer)) + 1;

        old = buffer(pos);
        new = y(i);

        sumSquareVoltages = (vrms^2 * length(buffer)) - old^2 + new^2;
        vrms = sqrt(sumSquareVoltages/length(buffer));
        buffer(pos) = y(i);

        disp(round(vrms,3));  % 3dp
        vrmsPlot(i) = vrms;
    end

    % Frequency
    if (prevVoltage <= midPoint && voltage > midPoint)
        trueZero = currentTime - ((currentTime - prevTime) * ((voltage - midPoint) / (voltage - prevVoltage)));

        currentFrequency = 1 / (trueZero - prevPositiveZeroCrossing);
        if (i > 100)
            disp(round(currentFrequency, 3));   % 3dp
        end
        prevPositiveZeroCrossing = trueZero;
    end

    frequenciesPlot(i) = currentFrequency;

    prevVoltage = voltage;
    prevTime = currentTime;
end


plot(t,frequenciesPlot)

hold on

plot(t, yTrue)

hold on

plot(t, vrmsPlot)

disp("length(t)");
disp(length(t));
disp("length(yTrue)");
disp(length(yTrue));



