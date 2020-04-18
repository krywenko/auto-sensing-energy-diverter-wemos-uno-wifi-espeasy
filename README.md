# auto-sensing-energy-diverter-wemos-uno-wifi-espeasy

auto sensing   hertz  energy monitor  and diverter-  based on: 

wemos D2 uno/wifi  board

emontx arduino shield

espeasy firmware

with use of fortek SSR or home made ssr using MOC3043 isolator and bta40600b triac:

 the  uno firmware  is adjust adjust able to handle LCD screen or with out but if with screen you sacrfice one CT to maxium of 3 CTs
 it will auto detect  grid hertz .  all you need to do is tell it  the size of the grouping on elements across the what number of  cascading SSR.
 
 it will use  the arduino base PWM software  but it will only pulse at 30 hertz minimium which is fine for 60 hertz grids  but allows you to use 4 casscading  SSRs
 if you using PWM.h  found in ardino libary   you can casscade 2 SSR ay a much lower frquency  of 15 hertz for 60hertz grid and 12 hertz for  50 hz grid.   but the  the 3rd SSR will pulse at 30 hz.
..  to compensate for lacking cascading  SSR with PWM.h  it  use  relays once it steps pass the 2 SSR into 3 SSR  on the 5th cycle it turns on a relay.   and this  realy will stay on unti it drops to 1 SSR  then it will step back
if it steps into the  3 SSR again when  the  1st relay is still activated it will turn on the next relay on the 5th cycle  and  will do so until all relays are on  as long as it is in SSR3  firing position. then once all the relays are exahusted  it will continue on using the final ssr..

you can use the  the relay function to turn on an eletric car charging system.  it you want it  kick off later you can disable the step down for  relay one  in "if Stat==0)"    then it will only turn off once the grid goes positive again. but will  turn on again once it  passes  SSR2  again.. you can move   step up function found in if (stat ==2)  to  if ( stat ==1)   but you will have to disable all step downs in  if (stat ==0) --  then the relays will only turn off once the grid goes  positive again..

you can control up to 7 diversion .  i find you get the best results using smaller diversions.. 1500 watts and under.. you can use in on larger   diversion but accracy drops .  ie ~110 usable steps =  per SSR  4000 kw element =  each step is  40 watts--   if 1100 watt element  each step is 10 watts..  and in the case of cascading  SSR  4000 kw of diversion with 10 watt accuracy   plus 765 possible steps..    if using the relays  try to keep them smaller then the 2nd SSR that way  when in kick on it does not drop into the 1st SSR realm and start kicking down . if not possible then disable  step down in ssr! and only use the  disable at  going positive ..



![daughter shield for emontx shield ](https://github.com/krywenko/energy-monitor-and-diverter-espeasy/blob/master/energydivertershield.png)


![movie of data sent to the espeasy firmware](https://raw.githubusercontent.com/krywenko/energy-monitor-and-diverter-espeasy/master/simplescreenrecorder-111.mp4)


 
more info here -- https://community.openenergymonitor.org/t/cascading-diversion-with-espeasy-wemosr3-and-emontx-shield/13689/
