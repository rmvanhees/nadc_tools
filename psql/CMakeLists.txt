set (NADC_PSQL
    inquire_scia_db.py
)

## Installation instructions
install (
   PROGRAMS ${NADC_PSQL} DESTINATION bin
)
