# sporkbomb VCV Rack plugin

## Entropy Pool

![Entropy Pool](./res/EntropyPool.png)

A sequencer that slices from a pool of random data

### Github integration

The context menu includes an "Integrations..." item, which lets you use a Github token to use a
user's contribution history (the green boxes on their profile page) as the data source.

To load private activity, use a classic token with the `repo` and `read:user` permissions.

The token is *not* saved with patches, but the loaded activity is, so you don't need to re-enter your token every time.

# Development

## Setup

1. Download the Rack SDK to `../Rack-SDK`
  * Or, download anywhere and set the `RACK_DIR` environment variable to its path
2. `make && make install`

## TODO

* NEXT:
    * swap length and filter, so filter in middle
        * start icon = left cap
        * filter icon = dot
        * length icon = right cap
    * swap reset and random
        * run icon = arrow right
        * reset icon = arrow left
        * ^^^ nice for puddle, weirder for pool
    * or maybe, only have start/filter/len icons, and use text for all others?
        * ^^^ nice for pool, weirder for puddle
        * actually, text is really nice for puddle outs... save space w/lowercase words below
        * try both, maybe split
* Scale
    * no scale input! just an attenuator on the cv output
* inputs can move into entropy base - only the widget differs?
    * well.. what about eos/trigger lights? i dropped those...
* output svg highilghting - big box sucks, having nothing sucks
* confirm short github history is fine
* Outputs
    * eos is when going from end to start
* reset/random buttons should light when pressed or triggered
* Replace seed input/output with just raw values string (comma separated)
* knob units/type in labels - also, 0-1 or 0-10? or mix/other?
* standardize ranges
* oh geeze totally messed this up
    * need *another* row of bigger knobs, for direct control, whille small ones are for cv attenuators/attenuverters
    * start offset:
        * main knob should be baseline, always active
            * one way
        * cv attenverter should allow it to wrap in both directions
    * length:
        * main knob should be baseline, always active
            * TWO ways - ccw for reverse steps
        * cv attenuverter should allow it to wrap out of forward/reverse? or no? (i think yes)
    * filter:
        * main knob baseline
            * TWO ways - 12oclock for no filtering, cw for raising high pass cutoff, ccw for low pass cutoff
        * cv attenuverter, should allow wrap
    * scale:
        * main knob baseline
            * TWO ways - 12oclock for 0, cw for 0-10, ccw for -5 to +5
        * cv attenuator - NO wrapping, its always bipolar or not
    * svg changes:
        reset icon is misaligned!
        custom knobs? dont like the shiny ones, at least...
        move grid up to get space
        maybe reduce knob/icon/port vertical spacing
        move title? easier on pool
        big knobs, either need offset or spacing for everything
        do offset with lines like circuit board lines conencting (like the up/45/horizontal/45/up stytle)
        lean into PCB design and/or colors? just for lines? in general? or nah?
        pips on arcsj
* bipolar knobs
    * would need bipolar switch then too for the input jacks to work 0-10 for  
    * filter cw = low cut, ccw = high cut
    * scale cw = 0-10 scale, ccw = -5 to 5 scale
    * length cw = normal, ccw = reverse direction
    * good tooltips on these
    * need pips on arcs - and start position isnt bipolar, so it is different!
    * no scale in puddle... redesign and fit it in? [1]
* Direct sequence value modification?
    * Can i support click/drag or right click modal editing *per* cell?
* value tooltips on hover?
* Another SVG pass
  * [1] individual little boxes for out, not big wrapper (like vcv scope)
      * puddle ouputs up a bit, icons below, clock move down a row, making room for scale?
        * could ditch led (maybe even on both - already have a giant visual above)
  * some subtle texture?
  * custom knobs??
  * any border? (like my memoryman, colored front, bare metal texture border)
  * new font - johnny mnemonic? hackers? something else cool?
  * anything around the border, or some global texture? very flat...
  * get rid of screws?
  * sporkbomb logo? like, a cartoon bomb cutouts to make tines at the bottom (angled)
  * new run icon (too out of place)
  * new filter icon (too sharp) - rename to cutoff? does that help w/icon? (scissors?)
  * new trigger icon (not horrible, but a bit out of place)
* Github build for all platforms
* Upgrade to C++... 17? 20? What is safe w/VCVrack publishing?
* Update readme with final screenshots, more info
* manual? how do those work?
* Publish!
