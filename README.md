[![Build Status](https://travis-ci.org/BradyBrenot/huestaceand.svg?branch=master)](https://travis-ci.org/BradyBrenot/huestaceand)

This is the daemon component of Huestacean; see [Huestacean's repo](https://github.com/BradyBrenot/huestacean) for more information.

## Building

Have Qt 5.10.x installed.

### Windows

Use Visual Studio 2017. File -> Open -> CMake...

There are too many targets picked up by VS from the cmakelists.txt (because I gave up on adding externals "the right way"; need to look into this again later, or find a cmake guru to help), so add a file .vs/launch.vs.json with contents.

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
    },
    {
      "type": "default",
      "name": "1 huestaceand_tests.exe",
      "project": "CMakeLists.txt",
      "projectTarget": "huestaceand_tests.exe",
      "args": [ "" ]
    }
  ]
}
```

and set "0 huestaceand.exe" (now at the top of the list) as startup target.