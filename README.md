# Bikeshed VCV Rack plugin

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

# License and Copyright

This software is licensed under the Gnu General Public License v3 or later. The license is included
in the file "LICENSE" in this repository.

It is a derived work of VCVRack (which is GPL3 or later) and its dependencies.

It contains the [JetBrains Mono](https://www.jetbrains.com/lp/mono) font and a modified
[League Mono](https://github.com/theleagueof/league-mono) font, both of which are licensed under
the [SIL Open Font License 1.1](https://openfontlicense.org/open-font-license-official-text).

The SVGs used (in res) are distributed under the Creative Commons CC-BY-NC-SA-4.0 and are copyright
the authors with authorship indicated by the GitHub transaction log.

Copyright to this software is held by the authors with authorship indicated by the GitHub
transaction log.

## TODO

* Github build for all platforms
* Manual
* Readme
* Plugin name, config
* Publish
