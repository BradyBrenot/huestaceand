[![Build Status](https://travis-ci.org/BradyBrenot/huestaceand.svg?branch=master)](https://travis-ci.org/BradyBrenot/huestaceand)

This is the daemon component of Huestacean; see [Huestacean's repo](https://github.com/BradyBrenot/huestacean) for more information.

## Building

Have Qt 5.10.x installed.

### Windows

Use Visual Studio 2017. File -> Open -> CMake...

There are too many targets picked up by VS from the cmakelists.txt, so add a file .vs/launch.vs.json with contents

```
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "type": "default",
      "name": "0 huestaceand.exe",
      "project": "CMakeLists.txt",
      "projectTarget": "huestaceand.exe"
    }
  ]
}
```

and set "0 huestaceand.exe" (now at the top of the list) as startup target.