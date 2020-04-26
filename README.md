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
if it steps into the  3 SSR again when  the  1st relay is still activated it will turn on the next relay on the 5th cycle  and  will do so until all relays are on  as long as it is in SSR3  firing position. then once all the relays are exahusted  it will continue on using the final ssr.. and it will do it in reverse turning off relays once it steps in SSR1 realm then it will turn off  relay 4  and proceed  turnoff  all relays  in order  as power production  decreases or consumption increases 

you can use the  the relay function to turn on an eletric car charging system.  it you want it  kick off later you can disable the step down for  relay one  in "if Stat==0)"    then it will only turn off once the grid goes positive again. but will  turn on again once it  passes  SSR2  again.. you can move   step up function found in if (stat ==2)  to  if ( stat ==1)   but you will have to disable all step downs in  if (stat ==0) --  then the relays will only turn off once the grid goes  positive again..

you can control up to 7 diversion .  i find you get the best results using smaller diversions.. 1500 watts and under.. you can use in on larger   diversion but accracy drops .  ie ~110 usable steps =  per SSR  4000 kw element =  each step is  40 watts--   if 1100 watt element  each step is 10 watts..  and in the case of cascading  SSR  4000 kw of diversion with 10 watt accuracy   plus 765 possible steps..    if using the relays  try to keep them smaller then the 2nd SSR that way  when in kick on it does not drop into the 1st SSR realm and start kicking down . if not possible then disable  step down in ssr1 and only use the  disable at  going positive ..



![daughter shield for emontx shield ](https://github.com/krywenko/energy-monitor-and-diverter-espeasy/blob/master/energydivertershield.png)


![movie of data sent to the espeasy firmware](https://raw.githubusercontent.com/krywenko/energy-monitor-and-diverter-espeasy/master/simplescreenrecorder-111.mp4)


 
more info here -- https://community.openenergymonitor.org/t/auto-hertz-energy-monitor-and-diversion-with-howto-instructions/

or

https://community.openenergymonitor.org/t/cascading-diversion-with-espeasy-wemosr3-and-emontx-shield/13689/

Update functionality:
Added more functionality to my energy monitor…

manual control of SSR 1 output for scheduled purposes

You can use espeasy firmware or home automation software to schedule events such as once a week heat your hot water tank to 70 C - temperature control by either espeasy or a mechanical switch on your tank ( this would be my preferred method for safety reasons )

automation software__

http://your_ip/control?cmd=pwm,12,0           sets ssr1 to auto for diversion and off
http://your_ip/control?cmd=pwm,12,1023      (  10 - 1023 - manually sets ssr1 at a fixed pulse for  for scheduled events

within espeasy firmware in the rules__

 On Clock#Time=Sun,18:25 do 
  pwm,12,1023
 endon

 On Clock#Time=Sun,19:25 do  
  pwm,12,0
 endon

manual and automatic control of the relays via home automation software or espeasy firmware–

there is auto function- this is determined by the diversion it will cascade into each switch as more load is required. if enabled it is saved into the eeprom to maintain setting after reboot. there are 4 relays that can be enabled or disabled. if disabling auto function disable from 4 -1…
and manual control for use via espeasy firmware or home automation software …

pulse setting are as follows

switch1  auto(355) off(325) on(295)
switch2  auto(265) off(235) on(205)
switch3  auto(175) off(145) on(115)
switch4 auto(85) off(55)  on(25)

there are 0 - 900 so there is alot of room for other switching configurations if you would like more options

example :
home automation

http://your_ip/control?cmd=pwm,04,25   turns on switch4 
http://your_ip/control?cmd=pwm,04,55   turns off switch4
http://your_ip/control?cmd=pwm,04,355   turns auto function on for switch1

example espeasy firmware

On Clock#Time=All,1:30 do   // every day at 1:30 hours  turns on  switch 2...
  pwm,4,205  
 endon

On Clock#Time=All,2:30 do   // every day at 1:30 hours  turns off  switch 2...
  pwm,4,235  
 endon

when setting up espeay firmware under hardware set gpio-12 and gpio-04 to low output
in devices as above add a task 3 named outputs with single input and interval of 1 second
and insert this into the rules

on outputs#switch>0 do
pwm,4,0
endon

this rule turns off the pwm once the switching info is received

on the wemos R3 (uno/wifi)
you will need to run a lead from gpio-12 on the esp header to pin 6 on uno board this is the pwm override control
and you will need to run another lead from gpio-04 to pin5 on the uno board.
