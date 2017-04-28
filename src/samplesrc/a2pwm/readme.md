# Oscillation Overthruster
## Intro
Oscillation Overthruster (HiLoPWM) is a synthesizer/sequencer for playing around with the Apple II and its limited sound capability. A High Frequency Oscillator (HFO) samples the waveform of a Low Frequency Oscillator (LFO) and applies an Attack/Decay/Sustain/Release envelope to the resulting value which is output as a Pulse Width Modulated (PWM) value. This can create some interesting effects and tones.
## Running
The HILOPWM.SYSTEM file can be launched by booting the lfo.po disk image. The first time, the help screen will be presented with a list of the available commands. Once a PATCH file has been saved, the help screen will only show up by pressing the ESCape key. The main screen displays a graphical representation of the LFO waveform, LFO period and tone duration. The lower panel displays the textual representation of the LFO period, duration, and octave. The Apple II keyboard keys that map a piano keyboard octave is shown in the middle. Macros can be recorded and played back along with playing tones and changing parameters. The macros and parameters can be made persistent by saving them to disk. The next time HILOPWM.SYSTEM is run, the help screen will be skipped and the macros and settings from the previous session will be present. When quitting (CTRL-Q), you will be prompted if want to quit in case it was an accidental keypress, then optionally prompted to save the current state if it has changed since the last save.
## Playing Tones
The Apple II keyboard is mapped to a one octave piano keyboard, from Bn-1 to Cn+1.

    KEY NOTE ('n' is current octave)
    --- ----
     A  Bn-1
     S  Cn
     E  C#n
     D  Dn
     R  D#n
     F  En
     G  Fn
     Y  F#n
     H  Gn
     U  G#n
     J  An
     I  A#n
     K  Bn
     L  Cn+1

The duration of the note is in increments of 0.0255 seconds (the PWM loop is 100 cycles and runs 255 times at ~1.022 MHz). The maximum note length is slight longer than 1 second.    

## Command keys
    KEY            COMMAND
    -------------- --------------------
    ESC            HELP/CANCEL RECORD
    CTRL-Q         QUIT
    1..8           LFO WAVEFORM
    < ,            INCREASE LFO
    > .            DECREASE LFO
    LEFT-ARROW     PREV OCTAVE
    RIGHT-ARROW    NEXT OCTAVE
    + UP-ARROW     INCREASE DURATION
    - DOWN-ARROW   DECREASE DURATION
    CTRL-Z..M      RECORD MACRO
    /              SAVE ABS MACRO
    ?              SAVE REL MACRO
    P              PERSISTANT STATE
    0              TOGGLE SPEAKER PHASE

Command keys and note keys can be interspersed together.
### Recording Macros
One of the most powerful features of the Oscillation Overthruster is macro recording. There are seven macros that can be assigned to the lower row of keys, Z ... M. To record one of the macros, type the key along with the CONTROL key to begin recording. The RECORDING text will show up to the right of the text panel during recording. When recording, other macros can be called, allowing nested calls. However, recursive macros cannot be made, i.e. macro Z calling macro X calling macro Z. If, during recording, you want to cancel the macro, press ESCape. Their are two ways to save the macro: absolute and relative. An absolute macro will retain the state settings at the time the macro is recorded. Octave, duration, and LFO settings are restored before the macro is played, then restored when it is done. A relative macro will use the current settings when playing back. To record a rest, the spacebar will insert a pause for the length of the current duration. You can make the current macros persistent by pressing 'P' to write the macros and current settings to disk for the next time.

## A note about speaker phase
I discovered that the Apple ][ and Apple //e are affected by the initial phase of the speaker when implementing volume control with PWM. Interestingly, the Apple //c and emulators aren't impacted by the speaker phase. The hack was to add a command to flip the speaker phase (the '0' key). If you notice diminished volume when you run Oscillation Overthruster, press '0' to see if it doesn't clear up the problem. I hope to find a solid solution to this someday, but this is the fix for now.
