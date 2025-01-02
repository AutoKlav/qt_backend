-- Globals
CREATE TABLE Globals ( name TEXT NOT NULL UNIQUE, value TEXT NOT NULL );

INSERT INTO Globals VALUES ( "serialDataTime", "3000" );
INSERT INTO Globals VALUES ( "stateMachineTick", "60000" );
INSERT INTO Globals VALUES ( "k", "1" );
INSERT INTO Globals VALUES ( "coolingThreshold", "50" );
INSERT INTO Globals VALUES ( "expansionTemp", "95" );

-- Sensor
create table Sensor
(
    name     TEXT not null
        unique,
    minValue REAL not null,
    maxValue REAL not null
);

-- Analog Input
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('temp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('expansionTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('heaterTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tankTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tempK', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tankWaterLevel', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('pressure', 0, 3);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('steamPressure', 0, 3);

-- Digital Input
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('doorClosed', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('burnerFault', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('waterShortage', 0, 1);

INSERT INTO Sensor (name, minValue, maxValue) VALUES ('waterLevel', 0, 3);

-- Digital Output
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('fillTankWithWater', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('cooling', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tankHeating', 0, 1); 
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('coolingHelper', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('autoklavFill', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('waterDrain', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('heating', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('pump', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('electricHeating', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('increasePressure', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('extensionCooling', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('alarmSignal', 0, 1);

-- Bacteria
drop table Bacteria;
create table Bacteria
(
    id INTEGER primary key autoincrement,
    name TEXT,
    description TEXT,
    d0 REAL, -- secret values, used in top secret formulas below, d0 --> min, z --> celsius
    z REAL, -- D = k * D0 ( 10^(-1/z))^deltaT, F = (10^(1/z))^deltaT / k*D0, r = (10^(1/z))^deltaT / D0
    dateCreated DATETIME not null,
    dateModified DATETIME
);
INSERT INTO Bacteria (id, name, description, d0, z, dateCreated, dateModified) VALUES (1, 'clostridium botulinum', 'G pozitivna, anaerobna bakterija', 0.2, 10, CURRENT_TIMESTAMP,null);

-- Process
drop table Process;
create table Process
(
    id              INTEGER
        primary key autoincrement,
   bacteriaId      INTEGER
        references Bacteria(id) 
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
    processLength   TEXT
);

INSERT INTO Process (id, bacteriaId, name, batchLTO, productName, productQuantity, processStart, processLength, targetF, targetCoolingTime, targetHeatingTime) VALUES (55,1, '2024-11-28T17:28:53', 'LTO324325345', 'Testni podaci', 'sint aliqua do laborum', '2024-11-28T17:28:53', '56363634654', null, null, null);

CREATE INDEX idx_process_start ON Process(processStart);

drop table ProcessType;
create table ProcessType
(
    id               INTEGER
        primary key autoincrement,
    name             TEXT not null,
    type             TEXT,
    customTemp       REAL,
    finishTemp       REAL,    
    maintainTemp     REAL
);

INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainTemp) VALUES (0, 'Sterilizacija', 'sterilizacija', 121.1, 121.1, 5);
INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainTemp) VALUES (1, 'Pasterizacija', 'pasterizacija', 70, 70, 6);
INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainTemp) VALUES (2, 'Prilagođeno', null, null, null, null);

INSERT INTO Sensor (name, minValue, maxValue) VALUES ('temp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('expansionTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('heaterTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tankTemp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tempK', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tankWaterLevel', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('pressure', 0, 3);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('steamPressure', 0, 3);

-- Digital Input
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('doorClosed', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('burnerFault', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('waterShortage', 0, 1);

-- ProcessLog
drop table ProcessLog;
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

-- ProcessLog Graph Sample Data
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:52:53', 11.7276, NULL, NULL, NULL, 12.7364, NULL, -0.0341854, NULL, NULL, NULL, NULL, 108.364, NULL, 68605100000.0, 1.45762e-11, 7.28809e-11, 1.45762e-11, 7.28809e-11);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:53:49', 11.7276, NULL, NULL, NULL, 12.7364, NULL, -0.00335308, NULL, NULL, NULL, NULL, 108.364, NULL, 68605100000.0, 1.45762e-11, 7.28809e-11, 2.91523e-11, 1.45762e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:54:49', 11.7276, NULL, NULL, NULL, 12.9886, NULL, 0.0103502, NULL, NULL, NULL, NULL, 108.111, NULL, 64734500000.0, 1.54477e-11, 7.72386e-11, 4.46001e-11, 2.23e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:55:49', 30.6431, NULL, NULL, NULL, 12.9886, NULL, 0.013776, NULL, NULL, NULL, NULL, 108.111, NULL, 64734500000.0, 1.54477e-11, 7.72386e-11, 6.00478e-11, 3.00239e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:56:49', 34.6784, NULL, NULL, NULL, 13.2408, NULL, 0.97643, NULL, NULL, NULL, NULL, 107.859, NULL, 61082300000.0, 1.63714e-11, 8.18568e-11, 7.64191e-11, 3.82096e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:57:49', 31.9041, NULL, NULL, NULL, 13.4931, NULL, 1.4766, NULL, NULL, NULL, NULL, 107.607, NULL, 57636100000.0, 1.73502e-11, 8.67512e-11, 9.37694e-11, 4.68847e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:58:49', 32.4086, NULL, NULL, NULL, 14.7541, NULL, 1.62048, NULL, NULL, NULL, NULL, 106.346, NULL, 43111300000.0, 2.31958e-11, 1.15979e-10, 1.16965e-10, 5.84826e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 11:59:49', 32.1563, NULL, NULL, NULL, 15.5107, NULL, 1.60335, NULL, NULL, NULL, NULL, 105.589, NULL, 36218400000.0, 2.76103e-11, 1.38051e-10, 1.44575e-10, 7.22877e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:00:49', 32.6608, NULL, NULL, NULL, 16.5195, NULL, 1.59993, NULL, NULL, NULL, NULL, 104.58, NULL, 28710900000.0, 3.483e-11, 1.7415e-10, 1.79405e-10, 8.97027e-10);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:01:49', 33.4174, NULL, NULL, NULL, 17.7806, NULL, 1.60335, NULL, NULL, NULL, NULL, 103.319, NULL, 21475500000.0, 4.65647e-11, 2.32824e-10, 2.2597e-10, 1.12985e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:02:49', 33.6696, NULL, NULL, NULL, 18.5372, NULL, 1.60678, NULL, NULL, NULL, NULL, 102.563, NULL, 18041900000.0, 5.54267e-11, 2.77133e-10, 2.81397e-10, 1.40698e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:03:49', 34.6784, NULL, NULL, NULL, 19.546, NULL, 1.61363, NULL, NULL, NULL, NULL, 101.554, NULL, 14302100000.0, 6.992e-11, 3.496e-10, 3.51317e-10, 1.75658e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:04:49', 35.435, NULL, NULL, NULL, 20.5548, NULL, 1.61363, NULL, NULL, NULL, NULL, 100.545, NULL, 11337500000.0, 8.82032e-11, 4.41016e-10, 4.3952e-10, 2.1976e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:05:49', 35.6872, NULL, NULL, NULL, 21.3115, NULL, 1.61363, NULL, NULL, NULL, NULL, 99.7885, NULL, 9524760000.0, 1.0499e-10, 5.24948e-10, 5.4451e-10, 2.72255e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:06:49', 36.9483, NULL, NULL, NULL, 22.3203, NULL, 1.62048, NULL, NULL, NULL, NULL, 98.7797, NULL, 7550420000.0, 1.32443e-10, 6.62215e-10, 6.76952e-10, 3.38476e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:07:49', 37.7049, NULL, NULL, NULL, 23.3291, NULL, 1.62391, NULL, NULL, NULL, NULL, 97.7709, NULL, 5985340000.0, 1.67075e-10, 8.35375e-10, 8.44027e-10, 4.22014e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:08:49', 38.7137, NULL, NULL, NULL, 24.0857, NULL, 1.62734, NULL, NULL, NULL, NULL, 97.0143, NULL, 5028360000.0, 1.98872e-10, 9.9436e-10, 1.0429e-09, 5.2145e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:09:49', 39.4703, NULL, NULL, NULL, 25.0946, NULL, 1.63076, NULL, NULL, NULL, NULL, 96.0054, NULL, 3986060000.0, 2.50874e-10, 1.25437e-09, 1.29377e-09, 6.46887e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:10:49', 40.7314, NULL, NULL, NULL, 25.8512, NULL, 1.64446, NULL, NULL, NULL, NULL, 95.2488, NULL, 3348740000.0, 2.98619e-10, 1.4931e-09, 1.59239e-09, 7.96197e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:11:49', 41.488, NULL, NULL, NULL, 26.86, NULL, 1.64104, NULL, NULL, NULL, NULL, 94.24, NULL, 2654600000.0, 3.76704e-10, 1.88352e-09, 1.9691e-09, 9.84549e-09);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:12:49', 42.2446, NULL, NULL, NULL, 27.6166, NULL, 1.64104, NULL, NULL, NULL, NULL, 93.4834, NULL, 2230170000.0, 4.48397e-10, 2.24199e-09, 2.41749e-09, 1.20875e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:13:49', 43.5056, NULL, NULL, NULL, 28.6255, NULL, 1.65132, NULL, NULL, NULL, NULL, 92.4745, NULL, 1767890000.0, 5.65647e-10, 2.82823e-09, 2.98314e-09, 1.49157e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:14:49', 44.7667, NULL, NULL, NULL, 29.6343, NULL, 1.65132, NULL, NULL, NULL, NULL, 91.4657, NULL, 1401430000.0, 7.13556e-10, 3.56778e-09, 3.6967e-09, 1.84835e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:15:49', 45.7755, NULL, NULL, NULL, 30.3909, NULL, 1.66845, NULL, NULL, NULL, NULL, 90.7091, NULL, 1177360000.0, 8.49357e-10, 4.24678e-09, 4.54605e-09, 2.27303e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:16:49', 46.5321, NULL, NULL, NULL, 31.3997, NULL, 1.66502, NULL, NULL, NULL, NULL, 89.7003, NULL, 933313000.0, 1.07145e-09, 5.35726e-09, 5.61751e-09, 2.80875e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:17:49', 48.0454, NULL, NULL, NULL, 32.4086, NULL, 1.67187, NULL, NULL, NULL, NULL, 88.6914, NULL, 739852000.0, 1.35162e-09, 6.75811e-09, 6.96913e-09, 3.48456e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:18:49', 49.0542, NULL, NULL, NULL, 33.6696, NULL, 1.67872, NULL, NULL, NULL, NULL, 87.4304, NULL, 553403000.0, 1.807e-09, 9.03501e-09, 8.77613e-09, 4.38807e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:19:49', 50.063, NULL, NULL, NULL, 34.174, NULL, 1.68557, NULL, NULL, NULL, NULL, 86.926, NULL, 492720000.0, 2.02955e-09, 1.01478e-08, 1.08057e-08, 5.40284e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:20:49', 51.5763, NULL, NULL, NULL, 35.1828, NULL, 1.689, NULL, NULL, NULL, NULL, 85.9172, NULL, 390587000.0, 2.56025e-09, 1.28013e-08, 1.33659e-08, 6.68297e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:21:49', 52.3329, NULL, NULL, NULL, 36.4439, NULL, 1.69585, NULL, NULL, NULL, NULL, 84.6561, NULL, 292155000.0, 3.42284e-09, 1.71142e-08, 1.67888e-08, 8.39438e-08);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:22:49', 53.5939, NULL, NULL, NULL, 37.4527, NULL, 1.7027, NULL, NULL, NULL, NULL, 83.6473, NULL, 231596000.0, 4.31786e-09, 2.15893e-08, 2.11066e-08, 1.05533e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:23:49', 54.6027, NULL, NULL, NULL, 38.4615, NULL, 1.70613, NULL, NULL, NULL, NULL, 82.6385, NULL, 183590000.0, 5.44692e-09, 2.72346e-08, 2.65536e-08, 1.32768e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:24:49', 55.6116, NULL, NULL, NULL, 39.2181, NULL, 1.71298, NULL, NULL, NULL, NULL, 81.8819, NULL, 154236000.0, 6.48356e-09, 3.24178e-08, 3.30371e-08, 1.65186e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:25:49', 56.8726, NULL, NULL, NULL, 40.4792, NULL, 1.72326, NULL, NULL, NULL, NULL, 80.6208, NULL, 115367000.0, 8.66795e-09, 4.33398e-08, 4.17051e-08, 2.08525e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:26:49', 57.8814, NULL, NULL, NULL, 41.7402, NULL, 1.73011, NULL, NULL, NULL, NULL, 79.3598, NULL, 86293900.0, 1.15883e-08, 5.79415e-08, 5.32934e-08, 2.66467e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:27:49', 59.1425, NULL, NULL, NULL, 42.4968, NULL, 1.73696, NULL, NULL, NULL, NULL, 78.6032, NULL, 72496700.0, 1.37937e-08, 6.89687e-08, 6.70871e-08, 3.35436e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:28:49', 60.1513, NULL, NULL, NULL, 43.7579, NULL, 1.74724, NULL, NULL, NULL, NULL, 77.3421, NULL, 54226900.0, 1.8441e-08, 9.22052e-08, 8.55281e-08, 4.27641e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:29:49', 60.9079, NULL, NULL, NULL, 45.0189, NULL, 1.74381, NULL, NULL, NULL, NULL, 76.0811, NULL, 40561300.0, 2.46541e-08, 1.2327e-07, 1.10182e-07, 5.50911e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:30:49', 62.1689, NULL, NULL, NULL, 45.7755, NULL, 1.75409, NULL, NULL, NULL, NULL, 75.3245, NULL, 34076100.0, 2.93461e-08, 1.46731e-07, 1.39528e-07, 6.97642e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:31:49', 63.43, NULL, NULL, NULL, 46.7843, NULL, 1.76437, NULL, NULL, NULL, NULL, 74.3157, NULL, 27012600.0, 3.70197e-08, 1.85099e-07, 1.76548e-07, 8.8274e-07);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:32:49', 64.1866, NULL, NULL, NULL, 47.7932, NULL, 1.77122, NULL, NULL, NULL, NULL, 73.3068, NULL, 21413300.0, 4.66999e-08, 2.33499e-07, 2.23248e-07, 1.11624e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:33:49', 65.4476, NULL, NULL, NULL, 49.0542, NULL, 1.77807, NULL, NULL, NULL, NULL, 72.0458, NULL, 16017000.0, 6.24337e-08, 3.12169e-07, 2.85682e-07, 1.42841e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:34:49', 66.4565, NULL, NULL, NULL, 49.8108, NULL, 1.7815, NULL, NULL, NULL, NULL, 71.2892, NULL, 13456100.0, 7.43158e-08, 3.71579e-07, 3.59997e-07, 1.79999e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:35:49', 67.7175, NULL, NULL, NULL, 51.0718, NULL, 1.7952, NULL, NULL, NULL, NULL, 70.0282, NULL, 10065000.0, 9.93538e-08, 4.96769e-07, 4.59351e-07, 2.29676e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:36:49', 68.7263, NULL, NULL, NULL, 52.0807, NULL, 1.80548, NULL, NULL, NULL, NULL, 69.0193, NULL, 7978710.0, 1.25334e-07, 6.26668e-07, 5.84685e-07, 2.92342e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:37:49', 69.7351, NULL, NULL, NULL, 53.0895, NULL, 1.81576, NULL, NULL, NULL, NULL, 68.0105, NULL, 6324850.0, 1.58107e-07, 7.90533e-07, 7.42791e-07, 3.71396e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:38:49', 70.9962, NULL, NULL, NULL, 54.0983, NULL, 1.82603, NULL, NULL, NULL, NULL, 67.0017, NULL, 5013810.0, 1.99449e-07, 9.97247e-07, 9.42241e-07, 4.7112e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:39:49', 72.5094, NULL, NULL, NULL, 55.6116, NULL, 1.83288, NULL, NULL, NULL, NULL, 65.4884, NULL, 3538700.0, 2.8259e-07, 1.41295e-06, 1.22483e-06, 6.12415e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:40:49', 73.7704, NULL, NULL, NULL, 56.6204, NULL, 1.84659, NULL, NULL, NULL, NULL, 64.4796, NULL, 2805180.0, 3.56483e-07, 1.78242e-06, 1.58131e-06, 7.90657e-06);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:41:49', 74.7793, NULL, NULL, NULL, 57.377, NULL, 1.85687, NULL, NULL, NULL, NULL, 63.723, NULL, 2356670.0, 4.24327e-07, 2.12164e-06, 2.00564e-06, 1.00282e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:42:49', 76.2925, NULL, NULL, NULL, 58.3858, NULL, 1.86714, NULL, NULL, NULL, NULL, 62.7142, NULL, 1868170.0, 5.35283e-07, 2.67642e-06, 2.54092e-06, 1.27046e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:43:49', 77.8058, NULL, NULL, NULL, 59.6469, NULL, 1.88427, NULL, NULL, NULL, NULL, 61.4531, NULL, 1397370.0, 7.15628e-07, 3.57814e-06, 3.25655e-06, 1.62828e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:44:49', 79.319, NULL, NULL, NULL, 60.9079, NULL, 1.89455, NULL, NULL, NULL, NULL, 60.1921, NULL, 1045220.0, 9.56732e-07, 4.78366e-06, 4.21328e-06, 2.10664e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:45:49', 80.8322, NULL, NULL, NULL, 61.9167, NULL, 1.91168, NULL, NULL, NULL, NULL, 59.1833, NULL, 828566.0, 1.20691e-06, 6.03453e-06, 5.42019e-06, 2.7101e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:46:49', 82.0933, NULL, NULL, NULL, 63.1778, NULL, 1.92538, NULL, NULL, NULL, NULL, 57.9222, NULL, 619760.0, 1.61353e-06, 8.06764e-06, 7.03372e-06, 3.51686e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:47:49', 83.6065, NULL, NULL, NULL, 64.4388, NULL, 1.94251, NULL, NULL, NULL, NULL, 56.6612, NULL, 463575.0, 2.15715e-06, 1.07857e-05, 9.19087e-06, 4.59543e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:48:49', 84.6153, NULL, NULL, NULL, 65.4476, NULL, 1.95964, NULL, NULL, NULL, NULL, 55.6524, NULL, 367483.0, 2.72121e-06, 1.36061e-05, 1.19121e-05, 5.95604e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:49:49', 85.6242, NULL, NULL, NULL, 66.9609, NULL, 1.98019, NULL, NULL, NULL, NULL, 54.1391, NULL, 259366.0, 3.85555e-06, 1.92778e-05, 1.57676e-05, 7.88381e-05);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:50:49', 87.3896, NULL, NULL, NULL, 68.2219, NULL, 1.98705, NULL, NULL, NULL, NULL, 52.8781, NULL, 194004.0, 5.15454e-06, 2.57727e-05, 2.09222e-05, 0.000104611);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:51:49', 88.3984, NULL, NULL, NULL, 69.4829, NULL, 2.01103, NULL, NULL, NULL, NULL, 51.6171, NULL, 145113.0, 6.89117e-06, 3.44559e-05, 2.78133e-05, 0.000139067);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:52:49', 89.4073, NULL, NULL, NULL, 70.2396, NULL, 1.95964, NULL, NULL, NULL, NULL, 50.8604, NULL, 121912.0, 8.20267e-06, 4.10133e-05, 3.6016e-05, 0.00018008);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:53:49', 90.6683, NULL, NULL, NULL, 71.5006, NULL, 1.9939, NULL, NULL, NULL, NULL, 49.5994, NULL, 91188.8, 1.09663e-05, 5.48313e-05, 4.69823e-05, 0.000234911);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:54:49', 91.4249, NULL, NULL, NULL, 72.5094, NULL, 2.0076, NULL, NULL, NULL, NULL, 48.5906, NULL, 72286.8, 1.38338e-05, 6.91689e-05, 6.0816e-05, 0.00030408);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:55:49', 92.6859, NULL, NULL, NULL, 74.0227, NULL, 2.01445, NULL, NULL, NULL, NULL, 47.0773, NULL, 51019.4, 1.96004e-05, 9.8002e-05, 8.04165e-05, 0.000402082);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:56:49', 93.6948, NULL, NULL, NULL, 75.0315, NULL, 2.02473, NULL, NULL, NULL, NULL, 46.0685, NULL, 40443.8, 2.47256e-05, 0.000123628, 0.000105142, 0.000525711);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:57:49', 94.9558, NULL, NULL, NULL, 76.2925, NULL, 2.03843, NULL, NULL, NULL, NULL, 44.8075, NULL, 30251.6, 3.30561e-05, 0.00016528, 0.000138198, 0.000690991);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:58:49', 95.7124, NULL, NULL, NULL, 77.3013, NULL, 2.03158, NULL, NULL, NULL, NULL, 43.7987, NULL, 23980.9, 4.16998e-05, 0.000208499, 0.000179898, 0.00089949);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 12:59:49', 97.2257, NULL, NULL, NULL, 78.8146, NULL, 2.03158, NULL, NULL, NULL, NULL, 42.2854, NULL, 16925.5, 5.90823e-05, 0.000295412, 0.00023898, 0.0011949);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:00:49', 98.4867, NULL, NULL, NULL, 80.0756, NULL, 2.04186, NULL, NULL, NULL, NULL, 41.0244, NULL, 12660.2, 7.8988e-05, 0.00039494, 0.000317968, 0.00158984);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:01:49', 98.9911, NULL, NULL, NULL, 80.8322, NULL, 2.05214, NULL, NULL, NULL, NULL, 40.2678, NULL, 10636.0, 9.40206e-05, 0.000470103, 0.000411989, 0.00205994);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:02:49', 99.9999, NULL, NULL, NULL, 82.0933, NULL, 2.07954, NULL, NULL, NULL, NULL, 39.0067, NULL, 7955.61, 0.000125697, 0.000628487, 0.000537686, 0.00268843);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:03:49', 101.261, NULL, NULL, NULL, 83.1021, NULL, 2.07954, NULL, NULL, NULL, NULL, 37.9979, NULL, 6306.54, 0.000158566, 0.000792828, 0.000696252, 0.00348126);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:04:49', 102.27, NULL, NULL, NULL, 84.3631, NULL, 2.09667, NULL, NULL, NULL, NULL, 36.7369, NULL, 4717.24, 0.000211988, 0.00105994, 0.00090824, 0.0045412);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:05:49', 103.279, NULL, NULL, NULL, 85.6242, NULL, 2.11038, NULL, NULL, NULL, NULL, 35.4758, NULL, 3528.45, 0.00028341, 0.00141705, 0.00119165, 0.00595825);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:06:49', 104.287, NULL, NULL, NULL, 86.633, NULL, 2.13093, NULL, NULL, NULL, NULL, 34.467, NULL, 2797.06, 0.000357518, 0.00178759, 0.00154917, 0.00774584);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:07:49', 105.548, NULL, NULL, NULL, 87.894, NULL, 2.14463, NULL, NULL, NULL, NULL, 33.206, NULL, 2092.18, 0.000477971, 0.00238986, 0.00202714, 0.0101357);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:08:49', 106.305, NULL, NULL, NULL, 88.9028, NULL, 2.15834, NULL, NULL, NULL, NULL, 32.1972, NULL, 1658.5, 0.000602954, 0.00301477, 0.00263009, 0.0131505);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:10:43', 108.323, NULL, NULL, NULL, 91.4249, NULL, 2.18917, NULL, NULL, NULL, NULL, 29.6751, NULL, 927.917, 0.00107768, 0.00538841, 0.00370778, 0.0185389);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:10:49', 108.575, NULL, NULL, NULL, 91.1727, NULL, 2.18917, NULL, NULL, NULL, NULL, 29.9273, NULL, 983.399, 0.00101688, 0.00508441, 0.00472466, 0.0236233);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:11:49', 108.827, NULL, NULL, NULL, 92.4337, NULL, 2.2063, NULL, NULL, NULL, NULL, 28.6663, NULL, 735.574, 0.00135948, 0.00679741, 0.00608414, 0.0304207);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:12:49', 108.827, NULL, NULL, NULL, 93.6948, NULL, 2.20972, NULL, NULL, NULL, NULL, 27.4052, NULL, 550.203, 0.00181751, 0.00908755, 0.00790165, 0.0395083);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:13:49', 109.332, NULL, NULL, NULL, 94.4514, NULL, 2.21315, NULL, NULL, NULL, NULL, 26.6486, NULL, 462.233, 0.00216341, 0.0108171, 0.0100651, 0.0503254);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:14:46', 110.593, NULL, NULL, NULL, 95.208, NULL, 1.81918, NULL, NULL, NULL, NULL, 25.892, NULL, 388.328, 0.00257514, 0.0128757, 0.0126402, 0.0632011);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:15:46', 108.827, NULL, NULL, NULL, 96.469, NULL, 1.99047, NULL, NULL, NULL, NULL, 24.631, NULL, 290.466, 0.00344274, 0.0172137, 0.0160829, 0.0804148);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:16:46', 109.332, NULL, NULL, NULL, 96.9735, NULL, 1.99732, NULL, NULL, NULL, NULL, 24.1265, NULL, 258.615, 0.00386675, 0.0193337, 0.0199496, 0.0997485);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:17:46', 110.593, NULL, NULL, NULL, 98.2345, NULL, 2.0076, NULL, NULL, NULL, NULL, 22.8655, NULL, 193.442, 0.0051695, 0.0258475, 0.0251191, 0.125596);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:18:46', 110.845, NULL, NULL, NULL, 99.4955, NULL, 2.01103, NULL, NULL, NULL, NULL, 21.6045, NULL, 144.693, 0.00691118, 0.0345559, 0.0320303, 0.160152);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:19:46', 111.097, NULL, NULL, NULL, 99.9999, NULL, 2.01103, NULL, NULL, NULL, NULL, 21.1001, NULL, 128.827, 0.00776236, 0.0388118, 0.0397927, 0.198964);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:20:46', 111.349, NULL, NULL, NULL, 100.757, NULL, 2.01445, NULL, NULL, NULL, NULL, 20.3434, NULL, 108.229, 0.00923965, 0.0461982, 0.0490323, 0.245162);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:21:46', 111.854, NULL, NULL, NULL, 101.513, NULL, 2.01445, NULL, NULL, NULL, NULL, 19.5868, NULL, 90.9248, 0.0109981, 0.0549905, 0.0600304, 0.300152);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:22:46', 112.862, NULL, NULL, NULL, 102.27, NULL, 2.0213, NULL, NULL, NULL, NULL, 18.8302, NULL, 76.3872, 0.0130912, 0.065456, 0.0731216, 0.365608);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:23:46', 112.862, NULL, NULL, NULL, 103.026, NULL, 2.0213, NULL, NULL, NULL, NULL, 18.0736, NULL, 64.1739, 0.0155827, 0.0779133, 0.0887043, 0.443521);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:24:46', 113.115, NULL, NULL, NULL, 104.035, NULL, 2.02473, NULL, NULL, NULL, NULL, 17.0648, NULL, 50.8717, 0.0196573, 0.0982866, 0.108362, 0.541808);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:25:46', 112.862, NULL, NULL, NULL, 104.287, NULL, 2.02473, NULL, NULL, NULL, NULL, 16.8126, NULL, 48.0015, 0.0208327, 0.104163, 0.129195, 0.645971);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:26:46', 113.115, NULL, NULL, NULL, 105.044, NULL, 2.02473, NULL, NULL, NULL, NULL, 16.0559, NULL, 40.3267, 0.0247974, 0.123987, 0.153992, 0.769958);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:27:46', 113.115, NULL, NULL, NULL, 105.801, NULL, 2.01788, NULL, NULL, NULL, NULL, 15.2993, NULL, 33.879, 0.0295168, 0.147584, 0.183509, 0.917542);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:28:46', 113.115, NULL, NULL, NULL, 106.305, NULL, 2.0213, NULL, NULL, NULL, NULL, 14.7949, NULL, 30.1641, 0.033152, 0.16576, 0.216661, 1.0833);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:29:46', 113.115, NULL, NULL, NULL, 106.557, NULL, 2.0213, NULL, NULL, NULL, NULL, 14.5427, NULL, 28.4622, 0.0351343, 0.175671, 0.251795, 1.25897);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:30:46', 113.619, NULL, NULL, NULL, 107.314, NULL, 2.0213, NULL, NULL, NULL, NULL, 13.7861, NULL, 23.9115, 0.0418208, 0.209104, 0.293616, 1.46807);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:31:46', 114.124, NULL, NULL, NULL, 107.566, NULL, 2.0213, NULL, NULL, NULL, NULL, 13.5339, NULL, 22.5625, 0.0443214, 0.221607, 0.337937, 1.68968);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:32:46', 114.376, NULL, NULL, NULL, 108.323, NULL, 2.03158, NULL, NULL, NULL, NULL, 12.7772, NULL, 18.955, 0.0527564, 0.263782, 0.390693, 1.95346);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:33:46', 114.628, NULL, NULL, NULL, 108.827, NULL, 2.02816, NULL, NULL, NULL, NULL, 12.2728, NULL, 16.8765, 0.0592539, 0.296269, 0.449947, 2.24973);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:34:46', 114.376, NULL, NULL, NULL, 109.079, NULL, 2.02816, NULL, NULL, NULL, NULL, 12.0206, NULL, 15.9244, 0.0627968, 0.313984, 0.512744, 2.56371);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:35:46', 114.628, NULL, NULL, NULL, 109.584, NULL, 2.03158, NULL, NULL, NULL, NULL, 11.5162, NULL, 14.1782, 0.0705308, 0.352654, 0.583275, 2.91636);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:36:46', 114.628, NULL, NULL, NULL, 110.088, NULL, 2.02816, NULL, NULL, NULL, NULL, 11.0118, NULL, 12.6235, 0.0792173, 0.396087, 0.662492, 3.31245);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:37:46', 115.385, NULL, NULL, NULL, 110.34, NULL, 2.03843, NULL, NULL, NULL, NULL, 10.7596, NULL, 11.9113, 0.0839539, 0.419769, 0.746446, 3.73222);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:38:46', 115.637, NULL, NULL, NULL, 110.593, NULL, 2.04186, NULL, NULL, NULL, NULL, 10.5074, NULL, 11.2393, 0.0889737, 0.444868, 0.83542, 4.17709);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:39:46', 115.889, NULL, NULL, NULL, 111.097, NULL, 2.04529, NULL, NULL, NULL, NULL, 10.003, NULL, 10.0068, 0.0999316, 0.499658, 0.935352, 4.67675);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:40:46', 116.141, NULL, NULL, NULL, 111.349, NULL, 2.04871, NULL, NULL, NULL, NULL, 9.75077, NULL, 9.44227, 0.105907, 0.529533, 1.04126, 5.20628);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:41:46', 116.141, NULL, NULL, NULL, 111.854, NULL, 2.04871, NULL, NULL, NULL, NULL, 9.24635, NULL, 8.40689, 0.11895, 0.59475, 1.16021, 5.80103);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:42:46', 115.889, NULL, NULL, NULL, 111.854, NULL, 2.04529, NULL, NULL, NULL, NULL, 9.24635, NULL, 8.40689, 0.11895, 0.59475, 1.27916, 6.39578);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:43:46', 116.393, NULL, NULL, NULL, 112.358, NULL, 2.04871, NULL, NULL, NULL, NULL, 8.74194, NULL, 7.48504, 0.1336, 0.667999, 1.41276, 7.06378);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:44:46', 116.646, NULL, NULL, NULL, 112.61, NULL, 2.05214, NULL, NULL, NULL, NULL, 8.48973, NULL, 7.06274, 0.141588, 0.70794, 1.55435, 7.77172);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:45:46', 116.393, NULL, NULL, NULL, 112.862, NULL, 2.04871, NULL, NULL, NULL, NULL, 8.23753, NULL, 6.66427, 0.150054, 0.75027, 1.7044, 8.52199);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:46:46', 116.646, NULL, NULL, NULL, 113.367, NULL, 2.04871, NULL, NULL, NULL, NULL, 7.73311, NULL, 5.9335, 0.168534, 0.842672, 1.87293, 9.36466);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:47:46', 116.898, NULL, NULL, NULL, 113.871, NULL, 2.04871, NULL, NULL, NULL, NULL, 7.2287, NULL, 5.28287, 0.189291, 0.946455, 2.06222, 10.3111);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:48:46', 116.646, NULL, NULL, NULL, 113.619, NULL, 2.05214, NULL, NULL, NULL, NULL, 7.48091, NULL, 5.59874, 0.178611, 0.893057, 2.24083, 11.2042);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:49:46', 116.898, NULL, NULL, NULL, 114.124, NULL, 2.05214, NULL, NULL, NULL, NULL, 6.97649, NULL, 4.98482, 0.200609, 1.00305, 2.44144, 12.2072);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:50:46', 116.646, NULL, NULL, NULL, 114.124, NULL, 2.05214, NULL, NULL, NULL, NULL, 6.97649, NULL, 4.98482, 0.200609, 1.00305, 2.64205, 13.2102);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:51:46', 116.898, NULL, NULL, NULL, 114.628, NULL, 2.04529, NULL, NULL, NULL, NULL, 6.47208, NULL, 4.43821, 0.225316, 1.12658, 2.86737, 14.3368);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:52:46', 116.393, NULL, NULL, NULL, 114.376, NULL, 2.03501, NULL, NULL, NULL, NULL, 6.72429, NULL, 4.70358, 0.212604, 1.06302, 3.07997, 15.3998);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:53:46', 116.646, NULL, NULL, NULL, 114.88, NULL, 2.04871, NULL, NULL, NULL, NULL, 6.21987, NULL, 4.18781, 0.238788, 1.19394, 3.31876, 16.5937);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:54:46', 116.393, NULL, NULL, NULL, 114.88, NULL, 2.04529, NULL, NULL, NULL, NULL, 6.21987, NULL, 4.18781, 0.238788, 1.19394, 3.55755, 17.7876);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:55:46', 116.393, NULL, NULL, NULL, 115.132, NULL, 2.04529, NULL, NULL, NULL, NULL, 5.96767, NULL, 3.95154, 0.253066, 1.26533, 3.81062, 19.0529);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:56:46', 116.141, NULL, NULL, NULL, 115.385, NULL, 2.04186, NULL, NULL, NULL, NULL, 5.71546, NULL, 3.7286, 0.268197, 1.34099, 4.07882, 20.3939);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:57:46', 116.393, NULL, NULL, NULL, 115.637, NULL, 2.04186, NULL, NULL, NULL, NULL, 5.46325, NULL, 3.51824, 0.284233, 1.42117, 4.36305, 21.8151);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:58:46', 116.393, NULL, NULL, NULL, 115.889, NULL, 2.03843, NULL, NULL, NULL, NULL, 5.21105, NULL, 3.31974, 0.301228, 1.50614, 4.66428, 23.3212);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 13:59:46', 107.566, NULL, NULL, NULL, 116.393, NULL, 1.93223, NULL, NULL, NULL, NULL, 4.70663, NULL, 2.95572, 0.338327, 1.69164, 5.00261, 25.0128);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:00:46', 102.018, NULL, NULL, NULL, 115.637, NULL, 1.84659, NULL, NULL, NULL, NULL, 5.46325, NULL, 3.51824, 0.284233, 1.42117, 5.28684, 26.434);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:01:46', 93.4426, NULL, NULL, NULL, 115.889, NULL, 2.08982, NULL, NULL, NULL, NULL, 5.21105, NULL, 3.31974, 0.301228, 1.50614, 5.58807, 27.9401);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:02:46', 87.3896, NULL, NULL, NULL, 115.132, NULL, 1.85001, NULL, NULL, NULL, NULL, 5.96767, NULL, 3.95154, 0.253066, 1.26533, 5.84114, 29.2054);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:03:46', 83.8587, NULL, NULL, NULL, 114.88, NULL, 1.81576, NULL, NULL, NULL, NULL, 6.21987, NULL, 4.18781, 0.238788, 1.19394, 6.07993, 30.3993);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:04:46', 77.5535, NULL, NULL, NULL, 113.871, NULL, 1.79177, NULL, NULL, NULL, NULL, 7.2287, NULL, 5.28287, 0.189291, 0.946455, 6.26922, 31.3458);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:05:46', 74.7793, NULL, NULL, NULL, 112.862, NULL, 1.77465, NULL, NULL, NULL, NULL, 8.23753, NULL, 6.66427, 0.150054, 0.75027, 6.41927, 32.0961);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:06:46', 70.744, NULL, NULL, NULL, 111.097, NULL, 1.7815, NULL, NULL, NULL, NULL, 10.003, NULL, 10.0068, 0.0999316, 0.499658, 6.5192, 32.5958);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:07:46', 67.7175, NULL, NULL, NULL, 109.079, NULL, 1.7815, NULL, NULL, NULL, NULL, 12.0206, NULL, 15.9244, 0.0627968, 0.313984, 6.582, 32.9098);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:08:46', 62.9256, NULL, NULL, NULL, 107.062, NULL, 1.78835, NULL, NULL, NULL, NULL, 14.0383, NULL, 25.3412, 0.0394614, 0.197307, 6.62146, 33.1071);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:09:46', 60.4035, NULL, NULL, NULL, 105.044, NULL, 1.8089, NULL, NULL, NULL, NULL, 16.0559, NULL, 40.3267, 0.0247974, 0.123987, 6.64626, 33.2311);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:10:46', 59.6469, NULL, NULL, NULL, 102.774, NULL, 1.80548, NULL, NULL, NULL, NULL, 18.3258, NULL, 68.011, 0.0147035, 0.0735175, 6.66096, 33.3046);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:11:46', 55.8638, NULL, NULL, NULL, 100.252, NULL, 1.81918, NULL, NULL, NULL, NULL, 20.8479, NULL, 121.559, 0.00822648, 0.0411324, 6.66919, 33.3457);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:12:46', 53.5939, NULL, NULL, NULL, 97.7301, NULL, 1.8089, NULL, NULL, NULL, NULL, 23.3699, NULL, 217.266, 0.00460265, 0.0230132, 6.67379, 33.3687);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:13:46', 51.5763, NULL, NULL, NULL, 95.208, NULL, 1.82261, NULL, NULL, NULL, NULL, 25.892, NULL, 388.328, 0.00257514, 0.0128757, 6.67637, 33.3816);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:14:46', 49.8108, NULL, NULL, NULL, 92.4337, NULL, 1.83288, NULL, NULL, NULL, NULL, 28.6663, NULL, 735.574, 0.00135948, 0.00679741, 6.67773, 33.3884);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:15:46', 48.2976, NULL, NULL, NULL, 89.9117, NULL, 1.82603, NULL, NULL, NULL, NULL, 31.1883, NULL, 1314.72, 0.000760619, 0.00380309, 6.67849, 33.3922);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:16:46', 47.0365, NULL, NULL, NULL, 87.1374, NULL, 1.81918, NULL, NULL, NULL, NULL, 33.9626, NULL, 2490.35, 0.00040155, 0.00200775, 6.67889, 33.3942);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:17:46', 45.5233, NULL, NULL, NULL, 84.3631, NULL, 1.82603, NULL, NULL, NULL, NULL, 36.7369, NULL, 4717.24, 0.000211988, 0.00105994, 6.6791, 33.3953);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:18:46', 43.7579, NULL, NULL, NULL, 81.5888, NULL, 1.81918, NULL, NULL, NULL, NULL, 39.5112, NULL, 8935.42, 0.000111914, 0.000559571, 6.67921, 33.3959);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:19:46', 43.0012, NULL, NULL, NULL, 79.319, NULL, 1.83631, NULL, NULL, NULL, NULL, 41.781, NULL, 15069.6, 6.63589e-05, 0.000331794, 6.67928, 33.3962);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:20:46', 41.7402, NULL, NULL, NULL, 76.5447, NULL, 1.83974, NULL, NULL, NULL, NULL, 44.5553, NULL, 28544.9, 3.50325e-05, 0.000175163, 6.67932, 33.3964);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:21:46', 40.227, NULL, NULL, NULL, 73.7704, NULL, 1.83631, NULL, NULL, NULL, NULL, 47.3296, NULL, 54069.9, 1.84946e-05, 9.24729e-05, 6.67934, 33.3965);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:22:46', 38.7137, NULL, NULL, NULL, 71.2484, NULL, 1.83288, NULL, NULL, NULL, NULL, 49.8516, NULL, 96641.2, 1.03476e-05, 5.17378e-05, 6.67935, 33.3966);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:23:46', 37.9571, NULL, NULL, NULL, 68.7263, NULL, 1.83974, NULL, NULL, NULL, NULL, 52.3737, NULL, 172730.0, 5.78937e-06, 2.89468e-05, 6.67936, 33.3966);
INSERT INTO ProcessLog ( processId, timestamp, temp, expansionTemp, heaterTemp, tankTemp, tempK, tankWaterLevel, pressure, steamPressure, doorClosed, burnerFault, waterShortage, dTemp, state, Dr, Fr, r, sumFr, sumr)
        VALUES ( 55, '2024-11-05 14:24:46', 36.9483, NULL, NULL, NULL, 66.7087, NULL, 1.84316, NULL, NULL, NULL, NULL, 54.3913, NULL, 274874.0, 3.63803e-06, 1.81901e-05, 6.67936, 33.3966);

