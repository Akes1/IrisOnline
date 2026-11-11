for /f "tokens=2" %%B in ('p4 changelists -m 1 //depot/IRIS_JPN_OPEN/...#have') do echo #define BUILD_NUM %%B > number.ini
ping  -n 3  localhost >nul