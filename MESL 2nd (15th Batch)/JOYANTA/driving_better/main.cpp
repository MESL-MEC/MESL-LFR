#include <Arduino.h>

#define SENSOR_COUNT (uint8_t)10
#define LeftMotorID (uint8_t)0
#define RightMotorID (uint8_t)1
#define THRESHOLD (uint16_t)550

// --- constants ---
const uint8_t MotorPins[2][2] = {
    {9, 8},  // left motor pins
    {10, 11} // right motor pins
};

const uint8_t SensorPins[SENSOR_COUNT] = {
    A0, A1, A2, A3, A4, A5, A6, A7, 2, 3};

// --- variables ---
bool sensorData[SENSOR_COUNT] = {false};
uint8_t activeSensorCount = 0;
float error = 0.0;

// --- function declarations ---
void readSensorArray();
void showSensorArray();
float processSensorData();
void controlMotor(uint8_t motorID, float driveValue);

// --- init point ---
void setup()
{
    Serial.begin(9600);
}

void loop()
{
    readSensorArray();
    delay(100);
}

// --- function defination
void readSensorArray()
{
    uint8_t sensorID = 0;
    while (sensorID < 8)
    {
        sensorData[sensorID] = analogRead(SensorPins[sensorID]) > THRESHOLD ? true : false;
        sensorID++;
    }
    while (sensorID < SENSOR_COUNT)
    {
        sensorData[sensorID] = digitalRead(SensorPins[sensorID]) ? true : false;
        sensorID++;
    }
    showSensorArray();
}

void showSensorArray()
{
    for (uint8_t sensorID = 0; sensorID < SENSOR_COUNT; sensorID++)
    {
        Serial.print(sensorData[sensorID]);
        Serial.print("\t");
    }
    Serial.println();
}

// ---
float processSensorData()
{
    // if sensor count is 10,
    // max sensorTotalWeight = 9 + 8 + 7 + ... + 3 + 2 + 1 = 50 -> that means uint8_t is fine
    activeSensorCount = 0;
    uint8_t sensorTotalWeight = 0;
    for (uint8_t sensorDataIndex = 0; sensorDataIndex < SENSOR_COUNT; sensorDataIndex++)
    {
        activeSensorCount += sensorData[sensorDataIndex];
        sensorTotalWeight += sensorData[sensorDataIndex] * sensorDataIndex;
    }

    if (activeSensorCount > 0)
    {
        // This is a clever mathematics, it's better not to try reverse engineering.
        // just make you own, it's not that hard anyway.
        error = (float)sensorTotalWeight / (float)activeSensorCount / 2.0F - 1.0F;
    }
    return error;
}

// ---

void controlMotor(uint8_t motorID, float controlValue)
{
    // This function is specially designed for our specific lfr
    // it's motors' all pins are not pwm supported.
    // right motor's both are pwm supported.
    // left motor pins [1] = 8 is not pwm-ed
    // That's why, I had to use common sense a little.
    if (controlValue == 0.0)
    {
        // power cut
        digitalWrite(MotorPins[motorID][0], LOW);
        digitalWrite(MotorPins[motorID][1], LOW);
    }

    else if (controlValue == +1.0)
    {
        // forward max speed
        digitalWrite(MotorPins[motorID][0], LOW);
        digitalWrite(MotorPins[motorID][1], HIGH);
    }
    else if (controlValue == -1.0)
    {
        // backward max speed
        digitalWrite(MotorPins[motorID][0], HIGH);
        digitalWrite(MotorPins[motorID][1], LOW);
    }
    else if (abs(controlValue) <= 1.0)
    {
        // variable seed
        uint8_t speed = (uint8_t)(abs(controlValue) * UINT8_MAX);
        if (controlValue > 0.0)
        {
            // forward variable speed
            // This is where I needed to think more.
            analogWrite(MotorPins[motorID][0], UINT8_MAX - speed);
            digitalWrite(MotorPins[motorID][1], LOW);
        }
        else
        {
            // backward variable speed
            analogWrite(MotorPins[motorID][0], speed);
            digitalWrite(MotorPins[motorID][1], LOW);
        }
    }
    else
    {
        // invalid value !
        // cutting power
        controlMotor(motorID, 0.0);
    }
}