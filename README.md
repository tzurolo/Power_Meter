# Power Meter

Continuous measurement of current and accumulated mAh

## Abstract
For embedded systems that run on batteries, one needs to know how much power the system consumes. The power drain of such systems varies widely and continuously. In order to estimate the power needs of these systems we need a way to continuously measure current and sum up the measurements to get mAh.

This capability was developed in support of the [Water Tank Monitor](https://github.com/tzurolo/Water-Tank-Monitor) project, which will be deployed on a large concrete water tank in the lake Victoria region of Tanzania

## The system
The heart of this system is the INA219A high-side current sensor, which is used to measure current in small intervals (1mS). These samples are averaged into 100ms buckets and reported via the USB serial of an AtMega32U4 on a (now retired) Sparkfun breakout (DEV-11117, which I happened to have laying around).