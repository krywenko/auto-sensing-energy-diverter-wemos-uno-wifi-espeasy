# auto-sensing-energy-diverter-wemos-uno-wifi-espeasy

auto sensing   hertz  energy monitor  and diverter-  based on: 

wemos D2 uno/wifi  board

emontx arduino shield

espeasy firmware

with use of fortek SSR or home made ssr using MOC3043 isolator and bta40600b triac:

 the  uno firmware  is adjust adjust able to handle LCD screen or with out but if with screen you sacrfice one CT to maxium of 3 CTs
 it will auto detect  grid hertz .  all you need to do is tell it  the size of the grouping on elements across the what number of  cascading SSR.
 
 it will use  the base PWM software  but it will only pulse at 30 hertz minimium which is fine for 60 hertz grids  but allows you to use 4 casscading  SSRs
 if you using PWM.h  found in ardino libary   you can casscade 2 SSR ay a much lower frquency  of 15 hertz for 60hertz grid and 12 hertz for  50 hz grid.   but the  the 3rd SSR will pulse at 30 hz.
..  to compensate for lacking SSR with PWM.h  it  use  relays once it steps pass the 2 SSR into 3 SSR  on the 5th cycle it turns on a relay.   and this  realy will stay on unti it drops to 1 SSR  then it will step back
if it steps into the  3 SSR again when  the relay is activated it will turn on the next relay on the 5th cycle  and  will do so until all relays are on  as long as it is in SSR3  firing position. then once all the relays are exahusted  it will continue on using the final ssr..



![daughter shield for emontx shield ](https://github.com/krywenko/energy-monitor-and-diverter-espeasy/blob/master/energydivertershield.png)


![movie of data sent to the espeasy firmware](https://raw.githubusercontent.com/krywenko/energy-monitor-and-diverter-espeasy/master/simplescreenrecorder-111.mp4)


 
