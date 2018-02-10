NEW
10  GR: COLOR=15: PLOT 0, 0
20  RI = 23: P = 1: DIM DB(RI): DIM XB(500): DIM YB(500):DIM VB(500)
30  PRINT "const beamdepth = "; RI: PRINT: PRINT "byte dbeam = 0";: FOR R = 1 TO RI
50    X = R * .7071: Y = R * .7071
60    SX = INT(X + .5): SY = INT(Y + .5): IF SCRN(SX,SY) <> 0 THEN 80
70    PLOT SX,SY: XB(P) = SX: YB(P) = SY: P = P + 1
80    X = X - .25
90    Y = SQR(R * R - X * X)
100   IF X > 0 THEN 60
110   DB(R) = P - 1
120   PRINT  ","; DB(R);
130 NEXT
140 PRINT: PRINT "const beampts = "; P: PRINT

200 PRINT "byte[] xbeam": PRINT "byte = 0": FOR R = 1 TO RI
210 PRINT "byte = "; 
220   FOR L = DB(R-1)+1 TO DB(R)
230     PRINT XB(L);: IF L <> DB(R) THEN PRINT ","; 
240   NEXT: PRINT
250 NEXT
260 PRINT

300 PRINT "byte[] ybeam": PRINT "byte = 0": FOR R = 1 TO RI
310 PRINT "byte = ";
320   FOR L = DB(R-1)+1 TO DB(R)
330     PRINT YB(L);: IF L <> DB(R) THEN PRINT ",";
340   NEXT: PRINT
350 NEXT
360 PRINT

400 PRINT "byte[] vbeam": PRINT "byte = 0": FOR R = 1 TO RI
410   PRINT "byte = ";: FOR L = DB(R-1)+1 TO DB(R)
420     M = 0: IF YB(L) <> 0 THEN M = XB(L)/YB(L)
430     SY = INT(YB(L) - 1): SX = INT(XB(L) - M + .25): REM ADJ OFFSET TO PEEK AROUND CORNERS
440     FOR P = 0 TO L
450       IF SY = YB(P) AND SX = XB(P) THEN VB(L) = P: P = L
460     NEXT
470     PRINT VB(L);: IF L <> DB(R) THEN PRINT ",";
480   NEXT: PRINT
490 NEXT
500 PRINT

999 END

RUN
]
