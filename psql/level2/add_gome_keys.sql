CREATE INDEX meta_fresco_absOrbit      ON meta_fresco (absOrbit);
CREATE INDEX meta_fresco_dateTimeStart ON meta_fresco (dateTimeStart);

CREATE INDEX meta_togomi_absOrbit      ON meta_togomi (absOrbit);
CREATE INDEX meta_togomi_dateTimeStart ON meta_togomi (dateTimeStart);

CREATE INDEX tile_fresco_tileinfo ON tile_fresco (fk_tileinfo);
