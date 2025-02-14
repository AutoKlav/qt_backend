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

## Add endpoint guidelines

1. Modify proto\autoklav.proto
2. Build to verfiy .proto file
3. Modify grpc server
4. Import new .proto file in postman
