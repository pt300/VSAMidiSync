# VSAMidiSync

Small program used to synchronise time on MIDI bus with one in Visual Show Automation software,
using ActiveX control (VSA Console.ocx). Also has few extras.

## Features
- Synchronises time of chosen MIDI output with one provided by VSA.
- Interfaces VSA through ActiveX control.
- GUI for quick interactions with VSA, such as play, pause, stop.
- Ability to define own list of subtracks.
- Ability to loop certain subtrack.
- Idle loop, subtrack that is looped after playback of any other subtrack ends.
- Ability to save and load list of subtracks (Saved in .vsb file).
- Super small size. 251kb raw and only 71kb when stripped.

## Possible problems and how to resolve them
- "Unable to get hold of VSA ActiveX control"

    VSA Console control most likely wasn't registered properly.
    To solve that problem you need to find where exactly file `VSA Console.ocx` is located and open terminal window there.
    Now run `regsvr32 "VSA Console.ocx"`. That's all.
    
## TODO
- [ ] Add colours to indicate playing subtrack and idle loop one in list.
- [ ] Rethink bottom bar.
- [ ] Setting idle loop with RMB menu.