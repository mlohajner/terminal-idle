# Terminal-Idle — ASCII terminal screensaver with minimal CPU & memory footprint

A system that detects inactivity in open terminals and launches a custom
`cmatrix` / `asciiquarium` / `cbonsai` as a screensaver.  
**Without resident (background) process constantly polling or listening for input events.**

## The idea

Instead of the classic approach (a process that sits permanently in memory
listening for events), this system uses a **periodic, cheap check**: the
service wakes up every `RestartSec` seconds, does a few checks (a few
milliseconds of CPU), then shuts itself down. Between two wake-ups — the
process doesn't exist: zero CPU, zero memory.

The **cost** of this saving is deliberate imprecision: the screensaver
activates somewhere between `T` and `2T` after the last activity (not
exactly at `T`), depending on when within the interval the activity
stopped. For a screensaver use case, this cost is negligible compared to
the benefit of having **no resident process**.

## How it works (sequence)

1. The systemd service starts, runs the script to completion, and shuts
   down. Systemd itself counts down 300s to the next run — no `sleep`
   loop, no other process "waiting."

2. **The script** (`terminal-idle.sh`) goes through open terminals:
   - determines the time of last activity in a reliable, zero-overhead way
   - if `idle < INACTIVITY_PERIOD`, skips it (recent activity detected)
   - checks whether that process is named `bash` / `zsh` / `sh`
   - checks whether that shell **has a registered handler**
       - if the handler is registered, **sends the signal**
       - if not, the signal is **not sent**


3. **`.bashrc`** for that shell has a trap registered:

   ```bash
   terminal_idle() {
       local cmds=(
           "cmatrix -b -u 2"
           "cbonsai -l -i"
           "asciiquarium"
       )
       printf '\e]11;#000000\a'
       local cmd=${cmds[RANDOM % ${#cmds[@]}]}
       eval "$cmd"
       printf '\e]111\a'
   }
   trap terminal_idle SIGALRM
   ```

   The signal reaches the shell, which catches it and runs
   `ascii_screensaver` as its own foreground job — that's why Ctrl-C exits
   it completely naturally and returns to that shell's normal prompt.

## Files

| Path | Role |
|---|---|
| `~/.local/bin/terminal-idle.sh` | Main logic: idle check + signal sending |
| `~/.config/systemd/user/terminal-idle.service` | Periodic script execution (no `.timer`, via `Restart=`) |
| `~/.bashrc` | Trap for `SIGALRM` that runs `ascii_screensaver` |

## Installation

```bash
# save the script as ~/.local/bin/terminal-idle.sh
chmod +x ~/.local/bin/terminal-idle.sh

mkdir -p ~/.local/bin ~/.config/systemd/user
# save the service file as ~/.config/systemd/user/terminal-idle.service

# add the trap to ~/.bashrc (see above) — then reload the shell
# (source ~/.bashrc or open a new terminal) so the trap gets registered

systemctl --user daemon-reload
systemctl --user enable --now terminal-idle.service
systemctl --user status terminal-idle.service
```

## Configuration

- `INACTIVITY_PERIOD` in the script is the time after which a terminal is
  considered idle.
- `RestartSec` in the `.service` file is the check frequency/interval.
- (currently both `300` = 5 min).

The real activation range for the screensaver is `INACTIVITY_PERIOD` to
`2 × INACTIVITY_PERIOD`, depending on when within the cycle the last
activity stopped.

## Tips and Tricks

* **Precision**: Customize the accuracy to your liking. 
* **Resolution**: `RestartSec` defines your time resolution.
* **Timeout**: `INACTIVITY_PERIOD` sets the inactivity timeout.
* **Instant Action**: You do not have to wait. Run `terminal_idle` to trigger the `idle` state immediately.
