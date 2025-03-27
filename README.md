<!--
SPDX-License-Identifier: Apache-2.0
-->

# Zephyr application for the FRDM-MCXN947

## Build

```
west build -b frdm_mcxn947/mcxn947/cpu0 frdm-demo/application
```

## Install debugger support

Install programmer and debugger support which is arm64 compatible

```
source .venv/bin/activate
pyocd pack install MCXN947VDF
```

```
west flash --runner pyocd
```
