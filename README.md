# Bikeshed VCV Rack plugin

## [Entropy Pool](./doc/EntropyPool.md) - Indexable randomness
[<img src="./doc/EntropyPool.png" height="480" title="Entropy Pool">](./doc/EntropyPool.md)

## [Entropy Puddle](./doc/EntropyPuddle.md) - A bit less indexable randomness
[<img src="./doc/EntropyPuddle.png" height="480" title="Entropy Puddle">](./doc/EntropyPuddle.md)

# Development

## Local compilation

1. Download the [Rack SDK](https://vcvrack.com/downloads/) for your platform
2. Set the `RACK_DIR` environment variable to its path
3. `make && make install`

## Cross-platform compilation with Docker

1. `docker compose run --rm build`
    * Generates plugins in `./plugin-build`

## Manual cross-platform compilation

If you want to use the [VCV Rack Plugin Toolchain](https://github.com/VCVRack/rack-plugin-toolchain)
yourself instead of the above docker command (which is using an image with the toolchain already),
then be aware that the Linux build requires OpenSSL to be compiled and added as a dependency to the
Linux toolchain. You may need to adjust some paths, but that looks like:

```
cd /tmp
wget https://github.com/openssl/openssl/releases/download/openssl-3.0.15/openssl-3.0.15.tar.gz
tar -xf openssl-3.0.15.tar.gz
cd openssl-3.0.15

./Configure linux-x86_64 no-shared \
  --cross-compile-prefix=/home/build/rack-plugin-toolchain/local/x86_64-ubuntu16.04-linux-gnu/bin/x86_64-ubuntu16.04-linux-gnu- \
  --prefix=/home/build/rack-plugin-toolchain/Rack-SDK-lin-x64/dep

make -j$(nproc)
make install_sw
```

The above should result in `libssl.a` and `libcrpyto.a` being placed in
`/home/build/rack-plugin-toolchain/Rack-SDK-lin-x64/dep/lib64`, at which point the normal toolchain
commands should work for all platforms.

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
