# Yosen
Yosen programming language

[![Build Status](https://github.com/python/cpython/workflows/Tests/badge.svg)](https://github.com/FlareCoding/Yosen/actions)

Yosen is a general purpose, high-level language similar to Python that compiles the source code to a high-level bytecode and then interprets it.
Yosen supports both procedural and object-oriented programming approaches giving users the flexibility within their programs. Unlike Python, Yosen doesn't pass parameters or return variables by reference. It is similar to C++ in this regard, it is strictly pass-by-value, with the only exception being when the "self" parameter gets passed to class member functions. Additionally, Yosen offers a large and growing standard library that is powered by Yosen's easy to use framework for writing native modules, which anyone can do and distribute them as shared library files!

## Contents
- [Platform](#platform)
- [Building](#building)
- [Installation](#installation)
- [Tests](#tests)
- [Contributing](#contributing)
- [License](#license)


## Platform

| Linux | Windows | MacOS |
|:--------:| :-: | :-: |
| ✓    | ✓ | ✓


## Building

After cloning the repo, open git bash in the repo root directory and execute the following commands:

Debug:
```
cd scripts
./GenerateProject.sh
cd ../build
cmake --build .
```

Release:
```
cd scripts
./GenerateProjectRelease.sh
cd ../build
cmake --build .
```

[Note] On Windows, you would use .bat building and installation scripts, while on Linux and MacOS you would use .sh scripts.

## Installation

Open git bash in the repo root directory and execute the following commands:

```
cd scripts
sudo ./install.sh
```

Or to uninstall:
```
cd scripts
sudo ./uninstall.sh
```

## Tests

The ```tests``` folder contains various tests and examples to get the user familiar with the syntax of Yosen.
It also provides a blackjack game to play if you get bored :)

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

## License
[MIT](https://opensource.org/licenses/MIT)
