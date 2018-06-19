#!/bin/sh

if [ $# -le 0 ]
then
echo "Script must get at least one parameter, exit"
exit 0
fi

if [ $# -eq 1 ]
then
echo -en 'system\npp\n'$1'\n' >/tmp/pp_cli_batch.sh
fi

if [ $# -eq 2 ]
then
if [ "$1" = "gbe" -o "$1" = "qos" ]
then
echo -en 'system\npp\n'$1'\n'$2'\n' >/tmp/pp_cli_batch.sh
else
echo -en 'system\npp\n'$1' '$2'\n' >/tmp/pp_cli_batch.sh
fi
fi

if [ $# -eq 3 ]
then
echo -en 'system\npp\n'$1'\n'$2' '$3'\n' >/tmp/pp_cli_batch.sh
fi

pp_cli batch /tmp/pp_cli_batch.sh

rm /tmp/pp_cli_batch.sh
