# Globals
CREATE TABLE Globals ( name TEXT NOT NULL UNIQUE, value TEXT NOT NULL );

INSERT INTO Globals VALUES ( "targetK", "3.5" );
INSERT INTO Globals VALUES ( "serialDataTime", "3000" );
INSERT INTO Globals VALUES ( "stateMachineTick", "60000" );
INSERT INTO Globals VALUES ( "sterilizationTemp", "121.1" );
INSERT INTO Globals VALUES ( "pasterizationTemp", "70.0" );


# Sensor
CREATE TABLE Sensor ( name TEXT NOT NULL UNIQUE, pinName TEXT NOT NULL UNIQUE, minValue REAL NOT NULL, maxValue REAL NOT NULL );

INSERT INTO Sensor VALUES ( "temp", "A0", 0, 150 );
INSERT INTO Sensor VALUES ( "tempK", "A1", 0, 150 );
INSERT INTO Sensor VALUES ( "pressure", "A2", 0, 3 );

INSERT INTO Sensor VALUES ( "waterFill", "D0", 0, 1 );
INSERT INTO Sensor VALUES ( "heating", "D1", 0, 1 );
INSERT INTO Sensor VALUES ( "bypass", "D2", 0, 1 );
INSERT INTO Sensor VALUES ( "pump", "D3", 0, 1 );
INSERT INTO Sensor VALUES ( "inPressure", "D4", 0, 1 );
INSERT INTO Sensor VALUES ( "cooling", "D5", 0, 1 );


# ProcessLog
CREATE TABLE ProcessLog (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE,
    productName TEXT,
    productQuantity TEXT,
    bacteria TEXT,
    description TEXT,
    processStart DATETIME,
    processLength TEXT
);
CREATE INDEX idx_process_start ON ProcessLog(processStart);
