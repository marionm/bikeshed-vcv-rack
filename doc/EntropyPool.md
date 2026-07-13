# Entropy Pool manual

[<img src="./EntropyPool.png" height="480" title="Entropy Pool">](./EntropyPool.md)

240 indexed values to slice and dice.

## Values

The value at each index is a number between zero and one, and are rendered as squares:
* A green square means the index is range and unmuted - more green = bigger value
* A gray square means the index is out of range or muted
* An outline means the value at the index is excluded by the filter parameter

To modifiy the value at a particular index, interact with the square:
* Click and drag up/down to increase/decrease the value
* Click to toggle muting
* Double click to reset the value
* Right click to open a menu for precise editing

The active range of indices is enclosed in brackets.

The current index is denoted with a dot.

## Parameters

### Clock (clk)

Move the current index by one by sending a trigger signal or pressing the button.

Does nothing if run is toggled off.

### Run

Toggle on or off with a trigger signal or button press.

### Reset (rst)

Set the current index to the start index with a trigger signal or button press.

### Start index (left bracket)

Change the start index with the large knob.

Modulate the start index with attenuverted CV.

### Filter (dot)

Filter values with the large knob. From vertical:
* Clockwise sets an increasing minimum value
* Counterclockwise sets a decreasing maximum value

Modulate the filter with attenuverted CV.

### Sequence length (right bracket)

Change the length with the large knob. The sequence can wrap. From vertical:
* Clockwise sets the length
* Countclockwise sets a negative length - a clock signal will move the current index backwards

Modulate the length with attenuverted CV.

## Outputs

### End of sequence (eos)

Outputs a trigger when the current index moves back to the start index.

### Trigger (trg)

Outputs a trigger when the current index changes.

### Gate

Outputs a gate when the current index changes, with a duration of the value at the current index
multiplied by the duration of the previous clock cycle.

### CV

Outputs the current value as CV, scaled by the small scale knob. From vertical:
* Clockwise scales to 0V/10V
* Counterclockwise scales to -5V/5V

## Context menu

### Values

Comma-separated list of values - editable.

### Seed

The seed most recently used to generate random values - editable.

### Use GitHub activity

Just for fun (and the original inspiration for this):

Uses a given GitHub token to load contribution history, normalize it, and use it as values.
* Using your token will load your activity - to load someone else's activity, prefix with `<username>@`
* To load private activity, use a classic token with the `repo` and `read:user` permissions

The token is *not* saved with patches, but the loaded values are, so you don't need to re-enter the
token, while saved patches remain safe to distribute.
