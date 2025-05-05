new

rem Rod's Colors

10  goto 300

100 for w = 3 to 50
110   for i = 1 to 19
120     for j = 0 to 19
130       k  = i + j
140       mi = 40 - i
150       mk = 40 - k
160       color = (j * 3) / (i + 3) + i * w / 12
170       plot i,  k 
180       plot k,  i 
190       plot mi, mk 
200       plot mk, mi 
210       plot k,  mi 
220       plot mi, k 
230       plot i,  mk 
240       plot mk, i 
250       if peek(-16384) > 127 then poke -16368, 0 : return
260     next
270   next
280 next
290 goto 100

300 gr : home
310 htab 11 : vtab 22
320 print "Press any key to exit."
330 gosub 100
340 text : home
350 print "That's all, folks!"
360 end

run