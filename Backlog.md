# Intro

This file is where I plan to sketch out ideas for features.
These are unlikely to be complete specs, but rather serve as a brain dump of ideas - depending on how bothered I am at the time.

-----

**CC Config**

Note; MVP is latching 0-127. Consider configuring latch/momentary and value to send on the fly. Perhaps a kind of inverse MIDI learn?
- [x] I want to enter a setup mode by pressing the setup button so that I can configure my CCs on the fly
- [x] I want to select one of the circle "play" buttons on the right hand side when I am in the setup mode so that I can then apply a CC value to it
- [ ] I want a selected circle "play" button to flash between two active states so that my attention is drawn to the fact that I'm currently editing a CC
- [x] I want the LEDs under square buttons 11-18, 21-24 to light blue, so that I know these buttons can be interacted with to set the CC number "tens"
- [x] I want the LEDs under square buttons 31-38, 41-42 to light purple, so that I know these buttons can be interacted with to set the CC number "units"
- [x] I want the button selection to intercancel within (tens) or (units), so that it's clear what is being entered
- [x] I want the default selection to be 11, 31 (CC #00)
- [ ] I want my CC selection to save to flash when deselected so that I don't have to configure CCs every reboot
- [x] I want the square button LEDs to update when selecting a new circle button so I am always viewing the value for the selected button.
- [ ] I want a numerical display on the spare pixels so I know what number I'm entering using the tens/units system


**Note Velocity Scaling**
- [ ] I want a log scale offset applied to the ADC value so that playing the FSRs feels more expressive than directly piping ADC -> Velocity



