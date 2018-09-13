# fetchtv
Downloads recorded video from Fetch TV STB.

## Usage
```
Usage: fetchtv [options] command id

Options:
  -d, --directory <directory>  Download into <directory>.

Arguments:
  command                      download, lastid, list, help
  id                           ID for download
```

## Libraries
* Qt
* QtUpnp: https://github.com/ptstream/QtUPnP

# Please Note
While it will download video recorded from Fetch TV channels, they are encrypted and won't play on any media player.

Only video recorded via free to air channels can be played on media players.

