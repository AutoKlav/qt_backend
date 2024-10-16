# Backend in Qt

## TCP Server

Receives requests from nextjs server
Use QJsonDocument for communication

## Current state

- fatches data from MainManager
- fetches variables from MainManager
- returns info about current temperature, is process running, is door open

## Process state

- fetches data from StateMachine
- returns info abount process if running

## Logs

- fetches data from LogManager
- returns list of stored logs
- returns specified log

## Start/Stop process

- set variables
- start/stop process

---

# Serial Port

- ~~connect to serial port~~
- ~~try reconnecting if disconnected~~
- send data
- ~~receive data, only last sent (track time when last data was received)~~

# Main Manager

- load/save variables
- start SerialPort
- fetch current state (variables, sensor status, state machine status)
- start/stop StateMachine

# State Machine

- states: ready, starting, stage x, stopping
- fetch current sensors and process state machine
- log process with LogManager

# Log Manager

- log program (myb seprate class)
- log process (files)
- manage process logs /w sqlite
- fetch list of logs by preference
- fetch specified log
- (fetch list of distinct names)

# Db Manager

- load variables
- load sensor data & calibrations
- update variable
- update sensor
- insert new log
- fetch logs

# Sensor

- sensor manager
- name, lower, upper

# ~~Global errors~~

- ~~enum~~

# Json Examples

## Requests

### Start/Stop process

```json
{
    "target": "process",
    "action": "start/stop"
}

{
    "code": 0,
    "status": "Success/Already running/Already stopping",
    "errors" []
}
```

### Set variable

```json
{
  "target": "main",
  "action": "setVariable",
  "data": [
    {
      "name": "targetK",
      "value": "5.5"
    },
    {
      "name": "cooldown temperature",
      "value": "70"
    }
  ]
}

{
    "code": 0,
    "status": "Success"
}
```

### Calibrate sensor

```json
{
  "target": "main",
  "action": "setSensor",
  "data": [
    {
      "name": "lower",
      "value": "0"
    },
    {
      "name": "upper",
      "value": "0"
    }
  ]
}
```

### Current state

```json
{
    "target": "main",
    "action": "current"
}

{
    "code": 0,
    "status": "",
    "errors": ["Serial port disconnected", "Serial port data old"],
    "data": {
        "sensors": [
            {
                "name": "temperature",
                "value": 70.3,
                "originalValue": 1023
            }
        ],
        "variables": [
            {
                "name": "targetK",
                "value": 5.5
            }
        ],
        "process": {
            "state": 0,
            "name": "ready",
        }
    }
}
```

### Current State Machine

```json
{
    "target": "state",
    "name": "current"
}

{
    "code": 0,
    "status": "",
    "errors": [""],
    "data": {
        "state": 2,
        "name": "Stage 3",
        "mode": "steril",
        "logName": "Pasteta 3",
        "data": {
            "targetK": 5.5,
            "currentK": 3,
        },
        "history": [
            {
                "time": 0,
                "temp": 27,
                "k": 0.1
            }
        ]
    }
}
```
