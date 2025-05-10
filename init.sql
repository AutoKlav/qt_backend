-- Globals
drop table if exists Globals;
CREATE TABLE Globals ( name TEXT NOT NULL UNIQUE, value TEXT NOT NULL );

INSERT INTO Globals VALUES ( "stateMachineTick", "5000" );
INSERT INTO Globals VALUES ( "dbTick", "60000" );
INSERT INTO Globals VALUES ( "serialDataOldTime", "5000" );
INSERT INTO Globals VALUES ( "k", "5" );
INSERT INTO Globals VALUES ( "coolingThreshold", "50" );
INSERT INTO Globals VALUES ( "expansionUpperTemp", "95" );
INSERT INTO Globals VALUES ( "expansionLowerTemp", "90" );
INSERT INTO Globals VALUES ( "heaterWaterLevel", "40" );
INSERT INTO Globals VALUES ( "maintainWaterTankTemp", "95" );

-- InputPin, used for reading and displaying sensor data through Modbus network from server PLC 
DROP TABLE IF EXISTS InputPin;

CREATE TABLE InputPin (
    id INTEGER PRIMARY KEY,
    alias TEXT,
    minValue REAL NOT NULL,
    maxValue REAL NOT NULL
);

-- Input pins
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (0, 'temp', −48.35, 220.12);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (1, 'tempK', -51.23, 217.24);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (2, 'expansionTemp', −48.35, 220.12);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (3, 'heaterTemp', −48.35, 220.12);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (4, 'tankTemp', −48.35, 220.12);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (5, 'tankWaterLevel', -43.731249999999875, 167.46874999999994);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (7, 'pressure', -1.08, 5.064);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (6, 'steamPressure', -0.00598, 16.16966);

-- Digital Pins read through Analog Inputs, converted from [analogMin, analogMax] to [0, 1]
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (8, 'doorClosed', 0, 1);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (9, 'burnerFault', 0, 1);
INSERT INTO InputPin (id, alias, minValue, maxValue) VALUES (10, 'waterShortage', 0, 1);

-- OutputPin, used for sending commands to the PLC through Modbus network, QT acts as clients that sends commands to the server PLC
DROP TABLE IF EXISTS OutputPin;

CREATE TABLE OutputPin (
    id INTEGER PRIMARY KEY,
    alias TEXT
);

-- Digital Outputs
INSERT INTO OutputPin (id, alias) VALUES (0, 'fillTankWithWater');
INSERT INTO OutputPin (id, alias) VALUES (1, 'cooling');
INSERT INTO OutputPin (id, alias) VALUES (2, 'tankHeating');
INSERT INTO OutputPin (id, alias) VALUES (3, 'coolingHelper');
INSERT INTO OutputPin (id, alias) VALUES (4, 'autoklavFill');
INSERT INTO OutputPin (id, alias) VALUES (5, 'waterDrain');
INSERT INTO OutputPin (id, alias) VALUES (6, 'heating');
INSERT INTO OutputPin (id, alias) VALUES (7, 'pump');
INSERT INTO OutputPin (id, alias) VALUES (11, 'electricHeating');
INSERT INTO OutputPin (id, alias) VALUES (8, 'increasePressure');
INSERT INTO OutputPin (id, alias) VALUES (10, 'extensionCooling');
INSERT INTO OutputPin (id, alias) VALUES (9, 'alarmSignal');

-- Bacteria
drop table if exists Bacteria;
create table Bacteria
(
    id INTEGER primary key autoincrement,
    name TEXT,
    description TEXT,
    d0 REAL,
    z REAL,
    dateCreated DATETIME not null,
    dateModified DATETIME
);

INSERT INTO Bacteria (id, name, description, d0, z, dateCreated, dateModified) VALUES 
(1, 'Clostridium botulinum', 'G pozitivna, anaerobna bakterija', 0.2, 10, CURRENT_TIMESTAMP, NULL),
(2, 'Clostridium thermosaccharolyticum', 'G pozitivna, anaerobna bakterija', 4.0, 9, CURRENT_TIMESTAMP, NULL),
(3, 'Clostridium sporogenes', 'G pozitivna, anaerobna bakterija', 1.0, 10, CURRENT_TIMESTAMP, NULL),
(4, 'Bacillus stearothermophilus', 'G pozitivna, termofilna bakterija', 5.0, 12.2, CURRENT_TIMESTAMP, NULL);

-- Process
drop table if exists ProcessLog;
drop table if exists Process;
drop table if exists ProcessType;

create table ProcessType
(
    id               INTEGER
        primary key autoincrement,
    name             TEXT not null,
    type             TEXT,
    customTemp       REAL,
    maintainTemp     REAL
);

INSERT INTO ProcessType (id, name, type, customTemp, maintainTemp) VALUES (0, 'Sterilizacija', 'sterilizacija', 121.11, 116 );
INSERT INTO ProcessType (id, name, type, customTemp, maintainTemp) VALUES (1, 'Pasterizacija', 'pasterizacija', 78, 78);

create table Process
(
    id              INTEGER
        primary key autoincrement,
   bacteriaId      INTEGER
        references Bacteria(id)
        on delete set null,
   processTypeId INTEGER
        references ProcessType(id)
        on delete set null,
    name            TEXT
        unique,
    batchLTO        TEXT,
    productName     TEXT,
    productQuantity TEXT,
    processStart    DATETIME,
    targetF         TEXT,
    targetHeatingTime TEXT,
    targetCoolingTime TEXT,
    processLength   TEXT,
    finishTemp   TEXT
);

CREATE INDEX idx_process_start ON Process(processStart);

-- ProcessLog
drop table if exists ProcessLog;
create table ProcessLog
(
    processId INTEGER  not null
        references Process,
    temp      REAL,
    expansionTemp REAL,
    heaterTemp REAL,
    tankTemp  REAL,
    tempK     REAL,
    tankWaterLevel REAL,
    pressure  REAL,
    steamPressure REAL,
    doorClosed REAL,
    burnerFault REAL,
    waterShortage REAL,
    dTemp     REAL,
    state     REAL,
    Dr        REAL,
    Fr        REAL,
    r         REAL,
    sumFr     REAL,
    sumr      REAL,
    timestamp DATETIME not null
);