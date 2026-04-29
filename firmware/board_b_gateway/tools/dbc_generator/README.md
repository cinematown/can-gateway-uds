# DBC Generator

Helpers for converting CAN database sources into DBC files.

## PQ35/PQ46 KMatrix Excel to DBC

```sh
python3 tools/dbc_generator/kmatrix_xlsx_to_dbc.py \
  docs/872521829-PQ35-46-ACAN-KMatrix-V5-20-6F-20160530-MH.xlsx \
  docs/Golf_6_PQ35.dbc
```

Current manual outputs:

- `docs/VW_Passat_B6.dbc`
- `docs/Golf_6_PQ35.dbc`
- `docs/signal_db.txt`
- `common/signal_db.h`
