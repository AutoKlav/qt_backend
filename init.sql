-- Globals
CREATE TABLE Globals ( name TEXT NOT NULL UNIQUE, value TEXT NOT NULL );

INSERT INTO Globals VALUES ( "serialDataTime", "3000" );
INSERT INTO Globals VALUES ( "stateMachineTick", "60000" );
INSERT INTO Globals VALUES ( "sterilizationTemp", "121.1" );
INSERT INTO Globals VALUES ( "pasterizationTemp", "70.0" );


-- Sensor
create table Sensor
(
    name     TEXT not null
        unique,
    minValue REAL not null,
    maxValue REAL not null
);

INSERT INTO Sensor (name, minValue, maxValue) VALUES ('temp', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('tempK', 0, 150);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('pressure', 0, 3);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('waterFill', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('heating', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('bypass', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('pump', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('inPressure', 0, 1);
INSERT INTO Sensor (name, minValue, maxValue) VALUES ('cooling', 0, 1);

-- Process
create table Process
(
    id              INTEGER
        primary key autoincrement,
    name            TEXT
        unique,
    productName     TEXT,
    productQuantity TEXT,
    bacteria        TEXT,
    description     TEXT,
    processStart    DATETIME,
    targetF         TEXT,
    processLength   TEXT
);

INSERT INTO Process (id, name, productName, productQuantity, bacteria, description, processStart, processLength, targetF) VALUES (55, '2024-11-28T17:28:53', 'Testni podaci', 'sint aliqua do laborum', 'nulla do laborum laboris labore', 'reprehenderit magna eiusmod et', '2024-11-28T17:28:53', '56363634654', null);

CREATE INDEX idx_process_start ON Process(processStart);

 CREATE TABLE ProcessType
(
    id              INTEGER
        primary key autoincrement,
    name TEXT NOT NULL,
    type TEXT NOT NULL,
    customTemp REAL NOT NULL,
    finishTemp REAL NOT NULL,
    maintainPressure REAL NOT NULL,
    pressure REAL NOT NULL
);

INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainPressure, maintainTemp) VALUES (0, 'Sterilizacija', 'sterilizacija', 121.1, 121.1, 5, 5);
INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainPressure, maintainTemp) VALUES (1, 'Pasterizacija', 'pasterizacija', 70, 70, 6, 6);
INSERT INTO ProcessType (id, name, type, customTemp, finishTemp, maintainPressure, maintainTemp) VALUES (2, 'Prilagođeno', null, null, null, null, null);

-- ProcessLog
create table ProcessLog
(
    processId INTEGER  not null
        references Process,
    temp      REAL     not null,
    tempK     REAL     not null,
    dTemp     REAL     not null,
    pressure  REAL     not null,
    state     REAL     not null,
    Dr        REAL     not null,
    Fr        REAL     not null,
    r         REAL     not null,
    sumFr     REAL     not null,
    sumr      REAL     not null,
    timestamp DATETIME not null
);

-- ProcessLog Graph Sample Data
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:52:53', -0.0341854, 11.7276, 12.7364, 108.364, 0, 68605100000.0, 1.45762e-11, 7.28809e-11, 1.45762e-11, 7.28809e-11);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:53:49', -0.00335308, 11.7276, 12.7364, 108.364, 0, 68605100000.0, 1.45762e-11, 7.28809e-11, 2.91523e-11, 1.45762e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:54:49', 0.0103502, 11.7276, 12.9886, 108.111, 0, 64734500000.0, 1.54477e-11, 7.72386e-11, 4.46001e-11, 2.23e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:55:49', 0.013776, 30.6431, 12.9886, 108.111, 0, 64734500000.0, 1.54477e-11, 7.72386e-11, 6.00478e-11, 3.00239e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:56:49', 0.97643, 34.6784, 13.2408, 107.859, 0, 61082300000.0, 1.63714e-11, 8.18568e-11, 7.64191e-11, 3.82096e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:57:49', 1.4766, 31.9041, 13.4931, 107.607, 0, 57636100000.0, 1.73502e-11, 8.67512e-11, 9.37694e-11, 4.68847e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:58:49', 1.62048, 32.4086, 14.7541, 106.346, 0, 43111300000.0, 2.31958e-11, 1.15979e-10, 1.16965e-10, 5.84826e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 11:59:49', 1.60335, 32.1563, 15.5107, 105.589, 0, 36218400000.0, 2.76103e-11, 1.38051e-10, 1.44575e-10, 7.22877e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:00:49', 1.59993, 32.6608, 16.5195, 104.58, 0, 28710900000.0, 3.483e-11, 1.7415e-10, 1.79405e-10, 8.97027e-10);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:01:49', 1.60335, 33.4174, 17.7806, 103.319, 0, 21475500000.0, 4.65647e-11, 2.32824e-10, 2.2597e-10, 1.12985e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:02:49', 1.60678, 33.6696, 18.5372, 102.563, 0, 18041900000.0, 5.54267e-11, 2.77133e-10, 2.81397e-10, 1.40698e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:03:49', 1.61363, 34.6784, 19.546, 101.554, 0, 14302100000.0, 6.992e-11, 3.496e-10, 3.51317e-10, 1.75658e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:04:49', 1.61363, 35.435, 20.5548, 100.545, 0, 11337500000.0, 8.82032e-11, 4.41016e-10, 4.3952e-10, 2.1976e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:05:49', 1.61363, 35.6872, 21.3115, 99.7885, 0, 9524760000.0, 1.0499e-10, 5.24948e-10, 5.4451e-10, 2.72255e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:06:49', 1.62048, 36.9483, 22.3203, 98.7797, 0, 7550420000.0, 1.32443e-10, 6.62215e-10, 6.76952e-10, 3.38476e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:07:49', 1.62391, 37.7049, 23.3291, 97.7709, 0, 5985340000.0, 1.67075e-10, 8.35375e-10, 8.44027e-10, 4.22014e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:08:49', 1.62734, 38.7137, 24.0857, 97.0143, 0, 5028360000.0, 1.98872e-10, 9.9436e-10, 1.0429e-09, 5.2145e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:09:49', 1.63076, 39.4703, 25.0946, 96.0054, 0, 3986060000.0, 2.50874e-10, 1.25437e-09, 1.29377e-09, 6.46887e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:10:49', 1.64446, 40.7314, 25.8512, 95.2488, 0, 3348740000.0, 2.98619e-10, 1.4931e-09, 1.59239e-09, 7.96197e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:11:49', 1.64104, 41.488, 26.86, 94.24, 0, 2654600000.0, 3.76704e-10, 1.88352e-09, 1.9691e-09, 9.84549e-09);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:12:49', 1.64104, 42.2446, 27.6166, 93.4834, 0, 2230170000.0, 4.48397e-10, 2.24199e-09, 2.41749e-09, 1.20875e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:13:49', 1.65132, 43.5056, 28.6255, 92.4745, 0, 1767890000.0, 5.65647e-10, 2.82823e-09, 2.98314e-09, 1.49157e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:14:49', 1.65132, 44.7667, 29.6343, 91.4657, 0, 1401430000.0, 7.13556e-10, 3.56778e-09, 3.6967e-09, 1.84835e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:15:49', 1.66845, 45.7755, 30.3909, 90.7091, 0, 1177360000.0, 8.49357e-10, 4.24678e-09, 4.54605e-09, 2.27303e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:16:49', 1.66502, 46.5321, 31.3997, 89.7003, 0, 933313000.0, 1.07145e-09, 5.35726e-09, 5.61751e-09, 2.80875e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:17:49', 1.67187, 48.0454, 32.4086, 88.6914, 0, 739852000.0, 1.35162e-09, 6.75811e-09, 6.96913e-09, 3.48456e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:18:49', 1.67872, 49.0542, 33.6696, 87.4304, 0, 553403000.0, 1.807e-09, 9.03501e-09, 8.77613e-09, 4.38807e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:19:49', 1.68557, 50.063, 34.174, 86.926, 0, 492720000.0, 2.02955e-09, 1.01478e-08, 1.08057e-08, 5.40284e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:20:49', 1.689, 51.5763, 35.1828, 85.9172, 0, 390587000.0, 2.56025e-09, 1.28013e-08, 1.33659e-08, 6.68297e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:21:49', 1.69585, 52.3329, 36.4439, 84.6561, 0, 292155000.0, 3.42284e-09, 1.71142e-08, 1.67888e-08, 8.39438e-08);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:22:49', 1.7027, 53.5939, 37.4527, 83.6473, 0, 231596000.0, 4.31786e-09, 2.15893e-08, 2.11066e-08, 1.05533e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:23:49', 1.70613, 54.6027, 38.4615, 82.6385, 0, 183590000.0, 5.44692e-09, 2.72346e-08, 2.65536e-08, 1.32768e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:24:49', 1.71298, 55.6116, 39.2181, 81.8819, 0, 154236000.0, 6.48356e-09, 3.24178e-08, 3.30371e-08, 1.65186e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:25:49', 1.72326, 56.8726, 40.4792, 80.6208, 0, 115367000.0, 8.66795e-09, 4.33398e-08, 4.17051e-08, 2.08525e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:26:49', 1.73011, 57.8814, 41.7402, 79.3598, 0, 86293900.0, 1.15883e-08, 5.79415e-08, 5.32934e-08, 2.66467e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:27:49', 1.73696, 59.1425, 42.4968, 78.6032, 0, 72496700.0, 1.37937e-08, 6.89687e-08, 6.70871e-08, 3.35436e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:28:49', 1.74724, 60.1513, 43.7579, 77.3421, 0, 54226900.0, 1.8441e-08, 9.22052e-08, 8.55281e-08, 4.27641e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:29:49', 1.74381, 60.9079, 45.0189, 76.0811, 0, 40561300.0, 2.46541e-08, 1.2327e-07, 1.10182e-07, 5.50911e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:30:49', 1.75409, 62.1689, 45.7755, 75.3245, 0, 34076100.0, 2.93461e-08, 1.46731e-07, 1.39528e-07, 6.97642e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:31:49', 1.76437, 63.43, 46.7843, 74.3157, 0, 27012600.0, 3.70197e-08, 1.85099e-07, 1.76548e-07, 8.8274e-07);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:32:49', 1.77122, 64.1866, 47.7932, 73.3068, 0, 21413300.0, 4.66999e-08, 2.33499e-07, 2.23248e-07, 1.11624e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:33:49', 1.77807, 65.4476, 49.0542, 72.0458, 0, 16017000.0, 6.24337e-08, 3.12169e-07, 2.85682e-07, 1.42841e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:34:49', 1.7815, 66.4565, 49.8108, 71.2892, 0, 13456100.0, 7.43158e-08, 3.71579e-07, 3.59997e-07, 1.79999e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:35:49', 1.7952, 67.7175, 51.0718, 70.0282, 0, 10065000.0, 9.93538e-08, 4.96769e-07, 4.59351e-07, 2.29676e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:36:49', 1.80548, 68.7263, 52.0807, 69.0193, 0, 7978710.0, 1.25334e-07, 6.26668e-07, 5.84685e-07, 2.92342e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:37:49', 1.81576, 69.7351, 53.0895, 68.0105, 0, 6324850.0, 1.58107e-07, 7.90533e-07, 7.42791e-07, 3.71396e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:38:49', 1.82603, 70.9962, 54.0983, 67.0017, 0, 5013810.0, 1.99449e-07, 9.97247e-07, 9.42241e-07, 4.7112e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:39:49', 1.83288, 72.5094, 55.6116, 65.4884, 0, 3538700.0, 2.8259e-07, 1.41295e-06, 1.22483e-06, 6.12415e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:40:49', 1.84659, 73.7704, 56.6204, 64.4796, 0, 2805180.0, 3.56483e-07, 1.78242e-06, 1.58131e-06, 7.90657e-06);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:41:49', 1.85687, 74.7793, 57.377, 63.723, 0, 2356670.0, 4.24327e-07, 2.12164e-06, 2.00564e-06, 1.00282e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:42:49', 1.86714, 76.2925, 58.3858, 62.7142, 0, 1868170.0, 5.35283e-07, 2.67642e-06, 2.54092e-06, 1.27046e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:43:49', 1.88427, 77.8058, 59.6469, 61.4531, 0, 1397370.0, 7.15628e-07, 3.57814e-06, 3.25655e-06, 1.62828e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:44:49', 1.89455, 79.319, 60.9079, 60.1921, 0, 1045220.0, 9.56732e-07, 4.78366e-06, 4.21328e-06, 2.10664e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:45:49', 1.91168, 80.8322, 61.9167, 59.1833, 0, 828566.0, 1.20691e-06, 6.03453e-06, 5.42019e-06, 2.7101e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:46:49', 1.92538, 82.0933, 63.1778, 57.9222, 0, 619760.0, 1.61353e-06, 8.06764e-06, 7.03372e-06, 3.51686e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:47:49', 1.94251, 83.6065, 64.4388, 56.6612, 0, 463575.0, 2.15715e-06, 1.07857e-05, 9.19087e-06, 4.59543e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:48:49', 1.95964, 84.6153, 65.4476, 55.6524, 0, 367483.0, 2.72121e-06, 1.36061e-05, 1.19121e-05, 5.95604e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:49:49', 1.98019, 85.6242, 66.9609, 54.1391, 0, 259366.0, 3.85555e-06, 1.92778e-05, 1.57676e-05, 7.88381e-05);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:50:49', 1.98705, 87.3896, 68.2219, 52.8781, 0, 194004.0, 5.15454e-06, 2.57727e-05, 2.09222e-05, 0.000104611);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:51:49', 2.01103, 88.3984, 69.4829, 51.6171, 0, 145113.0, 6.89117e-06, 3.44559e-05, 2.78133e-05, 0.000139067);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:52:49', 1.95964, 89.4073, 70.2396, 50.8604, 0, 121912.0, 8.20267e-06, 4.10133e-05, 3.6016e-05, 0.00018008);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:53:49', 1.9939, 90.6683, 71.5006, 49.5994, 0, 91188.8, 1.09663e-05, 5.48313e-05, 4.69823e-05, 0.000234911);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:54:49', 2.0076, 91.4249, 72.5094, 48.5906, 0, 72286.8, 1.38338e-05, 6.91689e-05, 6.0816e-05, 0.00030408);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:55:49', 2.01445, 92.6859, 74.0227, 47.0773, 0, 51019.4, 1.96004e-05, 9.8002e-05, 8.04165e-05, 0.000402082);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:56:49', 2.02473, 93.6948, 75.0315, 46.0685, 0, 40443.8, 2.47256e-05, 0.000123628, 0.000105142, 0.000525711);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:57:49', 2.03843, 94.9558, 76.2925, 44.8075, 0, 30251.6, 3.30561e-05, 0.00016528, 0.000138198, 0.000690991);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:58:49', 2.03158, 95.7124, 77.3013, 43.7987, 0, 23980.9, 4.16998e-05, 0.000208499, 0.000179898, 0.00089949);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 12:59:49', 2.03158, 97.2257, 78.8146, 42.2854, 0, 16925.5, 5.90823e-05, 0.000295412, 0.00023898, 0.0011949);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:00:49', 2.04186, 98.4867, 80.0756, 41.0244, 0, 12660.2, 7.8988e-05, 0.00039494, 0.000317968, 0.00158984);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:01:49', 2.05214, 98.9911, 80.8322, 40.2678, 0, 10636.0, 9.40206e-05, 0.000470103, 0.000411989, 0.00205994);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:02:49', 2.07954, 99.9999, 82.0933, 39.0067, 0, 7955.61, 0.000125697, 0.000628487, 0.000537686, 0.00268843);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:03:49', 2.07954, 101.261, 83.1021, 37.9979, 0, 6306.54, 0.000158566, 0.000792828, 0.000696252, 0.00348126);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:04:49', 2.09667, 102.27, 84.3631, 36.7369, 0, 4717.24, 0.000211988, 0.00105994, 0.00090824, 0.0045412);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:05:49', 2.11038, 103.279, 85.6242, 35.4758, 0, 3528.45, 0.00028341, 0.00141705, 0.00119165, 0.00595825);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:06:49', 2.13093, 104.287, 86.633, 34.467, 0, 2797.06, 0.000357518, 0.00178759, 0.00154917, 0.00774584);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:07:49', 2.14463, 105.548, 87.894, 33.206, 0, 2092.18, 0.000477971, 0.00238986, 0.00202714, 0.0101357);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:08:49', 2.15834, 106.305, 88.9028, 32.1972, 0, 1658.5, 0.000602954, 0.00301477, 0.00263009, 0.0131505);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:10:43', 2.18917, 108.323, 91.4249, 29.6751, 0, 927.917, 0.00107768, 0.00538841, 0.00370778, 0.0185389);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:10:49', 2.18917, 108.575, 91.1727, 29.9273, 0, 983.399, 0.00101688, 0.00508441, 0.00472466, 0.0236233);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:11:49', 2.2063, 108.827, 92.4337, 28.6663, 0, 735.574, 0.00135948, 0.00679741, 0.00608414, 0.0304207);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:12:49', 2.20972, 108.827, 93.6948, 27.4052, 0, 550.203, 0.00181751, 0.00908755, 0.00790165, 0.0395083);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:13:49', 2.21315, 109.332, 94.4514, 26.6486, 0, 462.233, 0.00216341, 0.0108171, 0.0100651, 0.0503254);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:14:46', 1.81918, 110.593, 95.208, 25.892, 0, 388.328, 0.00257514, 0.0128757, 0.0126402, 0.0632011);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:15:46', 1.99047, 108.827, 96.469, 24.631, 0, 290.466, 0.00344274, 0.0172137, 0.0160829, 0.0804148);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:16:46', 1.99732, 109.332, 96.9735, 24.1265, 0, 258.615, 0.00386675, 0.0193337, 0.0199496, 0.0997485);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:17:46', 2.0076, 110.593, 98.2345, 22.8655, 0, 193.442, 0.0051695, 0.0258475, 0.0251191, 0.125596);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:18:46', 2.01103, 110.845, 99.4955, 21.6045, 0, 144.693, 0.00691118, 0.0345559, 0.0320303, 0.160152);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:19:46', 2.01103, 111.097, 99.9999, 21.1001, 0, 128.827, 0.00776236, 0.0388118, 0.0397927, 0.198964);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:20:46', 2.01445, 111.349, 100.757, 20.3434, 0, 108.229, 0.00923965, 0.0461982, 0.0490323, 0.245162);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:21:46', 2.01445, 111.854, 101.513, 19.5868, 0, 90.9248, 0.0109981, 0.0549905, 0.0600304, 0.300152);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:22:46', 2.0213, 112.862, 102.27, 18.8302, 0, 76.3872, 0.0130912, 0.065456, 0.0731216, 0.365608);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:23:46', 2.0213, 112.862, 103.026, 18.0736, 0, 64.1739, 0.0155827, 0.0779133, 0.0887043, 0.443521);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:24:46', 2.02473, 113.115, 104.035, 17.0648, 0, 50.8717, 0.0196573, 0.0982866, 0.108362, 0.541808);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:25:46', 2.02473, 112.862, 104.287, 16.8126, 0, 48.0015, 0.0208327, 0.104163, 0.129195, 0.645971);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:26:46', 2.02473, 113.115, 105.044, 16.0559, 0, 40.3267, 0.0247974, 0.123987, 0.153992, 0.769958);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:27:46', 2.01788, 113.115, 105.801, 15.2993, 0, 33.879, 0.0295168, 0.147584, 0.183509, 0.917542);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:28:46', 2.0213, 113.115, 106.305, 14.7949, 0, 30.1641, 0.033152, 0.16576, 0.216661, 1.0833);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:29:46', 2.0213, 113.115, 106.557, 14.5427, 0, 28.4622, 0.0351343, 0.175671, 0.251795, 1.25897);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:30:46', 2.0213, 113.619, 107.314, 13.7861, 0, 23.9115, 0.0418208, 0.209104, 0.293616, 1.46807);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:31:46', 2.0213, 114.124, 107.566, 13.5339, 0, 22.5625, 0.0443214, 0.221607, 0.337937, 1.68968);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:32:46', 2.03158, 114.376, 108.323, 12.7772, 0, 18.955, 0.0527564, 0.263782, 0.390693, 1.95346);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:33:46', 2.02816, 114.628, 108.827, 12.2728, 0, 16.8765, 0.0592539, 0.296269, 0.449947, 2.24973);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:34:46', 2.02816, 114.376, 109.079, 12.0206, 0, 15.9244, 0.0627968, 0.313984, 0.512744, 2.56371);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:35:46', 2.03158, 114.628, 109.584, 11.5162, 0, 14.1782, 0.0705308, 0.352654, 0.583275, 2.91636);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:36:46', 2.02816, 114.628, 110.088, 11.0118, 0, 12.6235, 0.0792173, 0.396087, 0.662492, 3.31245);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:37:46', 2.03843, 115.385, 110.34, 10.7596, 0, 11.9113, 0.0839539, 0.419769, 0.746446, 3.73222);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:38:46', 2.04186, 115.637, 110.593, 10.5074, 0, 11.2393, 0.0889737, 0.444868, 0.83542, 4.17709);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:39:46', 2.04529, 115.889, 111.097, 10.003, 0, 10.0068, 0.0999316, 0.499658, 0.935352, 4.67675);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:40:46', 2.04871, 116.141, 111.349, 9.75077, 0, 9.44227, 0.105907, 0.529533, 1.04126, 5.20628);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:41:46', 2.04871, 116.141, 111.854, 9.24635, 0, 8.40689, 0.11895, 0.59475, 1.16021, 5.80103);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:42:46', 2.04529, 115.889, 111.854, 9.24635, 0, 8.40689, 0.11895, 0.59475, 1.27916, 6.39578);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:43:46', 2.04871, 116.393, 112.358, 8.74194, 0, 7.48504, 0.1336, 0.667999, 1.41276, 7.06378);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:44:46', 2.05214, 116.646, 112.61, 8.48973, 0, 7.06274, 0.141588, 0.70794, 1.55435, 7.77172);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:45:46', 2.04871, 116.393, 112.862, 8.23753, 0, 6.66427, 0.150054, 0.75027, 1.7044, 8.52199);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:46:46', 2.04871, 116.646, 113.367, 7.73311, 0, 5.9335, 0.168534, 0.842672, 1.87293, 9.36466);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:47:46', 2.04871, 116.898, 113.871, 7.2287, 0, 5.28287, 0.189291, 0.946455, 2.06222, 10.3111);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:48:46', 2.05214, 116.646, 113.619, 7.48091, 0, 5.59874, 0.178611, 0.893057, 2.24083, 11.2042);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:49:46', 2.05214, 116.898, 114.124, 6.97649, 0, 4.98482, 0.200609, 1.00305, 2.44144, 12.2072);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:50:46', 2.05214, 116.646, 114.124, 6.97649, 0, 4.98482, 0.200609, 1.00305, 2.64205, 13.2102);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:51:46', 2.04529, 116.898, 114.628, 6.47208, 0, 4.43821, 0.225316, 1.12658, 2.86737, 14.3368);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:52:46', 2.03501, 116.393, 114.376, 6.72429, 0, 4.70358, 0.212604, 1.06302, 3.07997, 15.3998);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:53:46', 2.04871, 116.646, 114.88, 6.21987, 0, 4.18781, 0.238788, 1.19394, 3.31876, 16.5937);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:54:46', 2.04529, 116.393, 114.88, 6.21987, 0, 4.18781, 0.238788, 1.19394, 3.55755, 17.7876);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:55:46', 2.04529, 116.393, 115.132, 5.96767, 0, 3.95154, 0.253066, 1.26533, 3.81062, 19.0529);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:56:46', 2.04186, 116.141, 115.385, 5.71546, 0, 3.7286, 0.268197, 1.34099, 4.07882, 20.3939);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:57:46', 2.04186, 116.393, 115.637, 5.46325, 0, 3.51824, 0.284233, 1.42117, 4.36305, 21.8151);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:58:46', 2.03843, 116.393, 115.889, 5.21105, 0, 3.31974, 0.301228, 1.50614, 4.66428, 23.3212);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 13:59:46', 1.93223, 107.566, 116.393, 4.70663, 0, 2.95572, 0.338327, 1.69164, 5.00261, 25.0128);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:00:46', 1.84659, 102.018, 115.637, 5.46325, 0, 3.51824, 0.284233, 1.42117, 5.28684, 26.434);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:01:46', 2.08982, 93.4426, 115.889, 5.21105, 0, 3.31974, 0.301228, 1.50614, 5.58807, 27.9401);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:02:46', 1.85001, 87.3896, 115.132, 5.96767, 0, 3.95154, 0.253066, 1.26533, 5.84114, 29.2054);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:03:46', 1.81576, 83.8587, 114.88, 6.21987, 0, 4.18781, 0.238788, 1.19394, 6.07993, 30.3993);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:04:46', 1.79177, 77.5535, 113.871, 7.2287, 0, 5.28287, 0.189291, 0.946455, 6.26922, 31.3458);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:05:46', 1.77465, 74.7793, 112.862, 8.23753, 0, 6.66427, 0.150054, 0.75027, 6.41927, 32.0961);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:06:46', 1.7815, 70.744, 111.097, 10.003, 0, 10.0068, 0.0999316, 0.499658, 6.5192, 32.5958);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:07:46', 1.7815, 67.7175, 109.079, 12.0206, 0, 15.9244, 0.0627968, 0.313984, 6.582, 32.9098);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:08:46', 1.78835, 62.9256, 107.062, 14.0383, 0, 25.3412, 0.0394614, 0.197307, 6.62146, 33.1071);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:09:46', 1.8089, 60.4035, 105.044, 16.0559, 0, 40.3267, 0.0247974, 0.123987, 6.64626, 33.2311);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:10:46', 1.80548, 59.6469, 102.774, 18.3258, 0, 68.011, 0.0147035, 0.0735175, 6.66096, 33.3046);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:11:46', 1.81918, 55.8638, 100.252, 20.8479, 0, 121.559, 0.00822648, 0.0411324, 6.66919, 33.3457);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:12:46', 1.8089, 53.5939, 97.7301, 23.3699, 0, 217.266, 0.00460265, 0.0230132, 6.67379, 33.3687);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:13:46', 1.82261, 51.5763, 95.208, 25.892, 0, 388.328, 0.00257514, 0.0128757, 6.67637, 33.3816);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:14:46', 1.83288, 49.8108, 92.4337, 28.6663, 0, 735.574, 0.00135948, 0.00679741, 6.67773, 33.3884);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:15:46', 1.82603, 48.2976, 89.9117, 31.1883, 0, 1314.72, 0.000760619, 0.00380309, 6.67849, 33.3922);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:16:46', 1.81918, 47.0365, 87.1374, 33.9626, 0, 2490.35, 0.00040155, 0.00200775, 6.67889, 33.3942);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:17:46', 1.82603, 45.5233, 84.3631, 36.7369, 0, 4717.24, 0.000211988, 0.00105994, 6.6791, 33.3953);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:18:46', 1.81918, 43.7579, 81.5888, 39.5112, 0, 8935.42, 0.000111914, 0.000559571, 6.67921, 33.3959);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:19:46', 1.83631, 43.0012, 79.319, 41.781, 0, 15069.6, 6.63589e-05, 0.000331794, 6.67928, 33.3962);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:20:46', 1.83974, 41.7402, 76.5447, 44.5553, 0, 28544.9, 3.50325e-05, 0.000175163, 6.67932, 33.3964);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:21:46', 1.83631, 40.227, 73.7704, 47.3296, 0, 54069.9, 1.84946e-05, 9.24729e-05, 6.67934, 33.3965);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:22:46', 1.83288, 38.7137, 71.2484, 49.8516, 0, 96641.2, 1.03476e-05, 5.17378e-05, 6.67935, 33.3966);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:23:46', 1.83974, 37.9571, 68.7263, 52.3737, 0, 172730.0, 5.78937e-06, 2.89468e-05, 6.67936, 33.3966);
INSERT INTO ProcessLog (processId, timestamp, pressure, temp, tempK, dTemp, state, Dr, Fr, r, sumFr, sumr)
VALUES (55, '2024-11-05 14:24:46', 1.84316, 36.9483, 66.7087, 54.3913, 0, 274874.0, 3.63803e-06, 1.81901e-05, 6.67936, 33.3966);