#!/bin/bash -fxv
# NOTE #!/bin/bash -fxv prints all executed commands.

# SPDX-FileCopyrightText: Lukas Sommer <sommerluk@gmail.com>
# SPDX-License-Identifier: BSD-2-Clause OR MIT





# -e exits on error,
# -u errors on undefined variables,
# and -o (for option) pipefail exits on command pipe failures
set -euo pipefail
errorcount=0

scripts/static-codecheck.sh &> artifact_staticcodecheck.txt
[ -s artifact_staticcodecheck.txt ] && ((errorcount++))

echo Terminating ci-staticcodecheck.sh with exit code $errorcount.
# NOTE The exit code of the last command is available with $? in the shell.
exit $errorcount
