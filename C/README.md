# For even lighter and faster execution C version of terminal-idle script

## Why?
**About 20 times faster and "lighter" for execution than bash version:**

---

Compile using (striped build):  
gcc -O2 -Wall -Wextra -s -o terminal-idle terminal-idle.c

INACTIVITY_PERIOD read from environment variable (default 300)!  
Setup Environment in `terminal-idle.service` [See here...](../binary/terminal-idle.service)
