.open .glgrib.db
CREATE TABLE CLNOMA2PALETTE (CLNOMA CHAR (16) PRIMARY KEY, palette CHAR (32) NOT NULL, min FLOAT NOT NULL, max FLOAT NOT NULL);
CREATE TABLE GRIB2PALETTE (discipline INTEGER NOT NULL, parameterCategory INTEGER NOT NULL, parameterNumber INTEGER NOT NULL, 
                           min FLOAT NOT NULL, max FLOAT NOT NULL, palette CHAR (32) NOT NULL, 
                           UNIQUE (discipline, parameterCategory, parameterNumber));
CREATE INDEX GRIB2PALETTE_IDX ON GRIB2PALETTE (discipline, parameterCategory, parameterNumber);

INSERT INTO CLNOMA2PALETTE VALUES ("SURFNEBUL.TOTALE", "cloud", 0., 100.);

