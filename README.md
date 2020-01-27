# inkbird_mqtt
Inkbird Btle devices to MQTT via ESP#@

Why does this exist ?

I wrote the inkbird integration with hassio/home assistant, and everything was good. However, I had frequent times when the measuring would stop. It would start on one device but, within a few cycles, all the devices would go offline. For home automation, and writing rules to turn on/off heaters etc, this obviously wasn't good. At first, I suspected the inkbird devices. Maybe they were going offline because of some collision in BTLE-land or.. something. I paired them with my phone, and left my phone plugged in. That's when I noticed that my raspberry pi's bluetooth was not seeing my phone when the inkbird devices went off. It, the raspberry pi's bluetooth stack, was jst going MIA. 

To that end, I quickly grabbed one of the nearby esp32's, and decided that it could jst gather the btle signals for me and pipe them into MQTT. The esp32 has been plugged in for going on 9 days now, and no drops. I therefore am considering the raspberry pi's bluetooth (or maybe it's the stack ha uses) to be "bad". 
