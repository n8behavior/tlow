# The Lie of Winterhaven


```
        _    .  ,   .           .
    *  / \_ *  / \_      _  *        *   /\'__        *
      /    \  /    \,   ((        .    _/  /  \  *'.
 .   /\/\  /\/ :' __ \_  `          _^/  ^/    `--.
    /    \/  \  _/  \-'\      *    /.' ^_   \_   .'\  *
  /\  .-   `. \/     \ /==~=-=~=-=-;.  _/ \ -. `_/   \
 /  `-.__ ^   / .-'.--\ =-=~_=-=~=^/  _ `--./ .-'  `-
/jgs     `.  / /       `.~-^=-=~=^=.-'      '-._ `._
```
<sup>Art by Joan Stark<sup>

## Getting started
You can compile the game from source (see below) or pull the current build from GitHub.
### Just get the game
If you just want to play/test the game, download the artifact bundle from GitHub

Go here https://github.com/n8behavior/tlow/actions and click any successful build (recommend latest) and download the zip file in the artifacts section

Unzip and make the `tlow` (or `tests`) binary executable (i.e. `chmod +x tlow`) and run it, `./tlow`. If don't have a map file in the same directory as the `tlow` binary, you will start with a blank map.  Pressing `<ctrl><s>` in-game will save your current map to `test.map` in the current directory.

### Build the game
The easier way is to use the Ubuntu latest Docker image, but a VM or your local/physical machine is fine.  Any Linux or Windows machine should work, if you know how to setup a normal c++/cmake build environment--if not, stick with Ubuntu.

Follow the steps in the `.github/workflows/ci.yml`.  Currently that looks like this, but check the CI build to be sure it hasn't changed

```shell
export DEBIAN_FRONTEND=noninteractive
apt update && apt dist-upgrade -y -q
# Install build tools
apt install -y -q --no-install-recommends git ca-certificates cmake build-essential
# Install build deps
apt install -y -q --no-install-recommends libpng-dev libx11-dev libgl1-mesa-dev libboost-serialization-dev

git clone <this repo OR your clone of this repo>
cd <the repo>
mkdir build && cd build
cmake .. && make

# run the game
cd src/
./tlow

# run the tests
cd test
./tests
```
