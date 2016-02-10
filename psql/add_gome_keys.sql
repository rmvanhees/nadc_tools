CREATE INDEX meta__1P_absOrbit      ON meta__1P (absOrbit);
CREATE INDEX meta__1P_dateTimeStart ON meta__1P (dateTimeStart);

CREATE INDEX meta__2P_absOrbit      ON meta__2P (absOrbit);
CREATE INDEX meta__2P_dateTimeStart ON meta__2P (dateTimeStart);

CREATE INDEX tileinfo_meta__1P_meta ON tileinfo_meta__1P (fk_meta);
CREATE INDEX tileinfo_meta__2P_meta ON tileinfo_meta__2P (fk_meta);
