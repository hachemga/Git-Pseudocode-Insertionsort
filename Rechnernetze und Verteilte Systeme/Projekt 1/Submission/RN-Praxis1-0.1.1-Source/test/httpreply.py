#!/usr/bin/env python3

import contextlib
import random
import re
import subprocess
import time
import socket


class KillOnExit(subprocess.Popen):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __exit__(self, *args):
        self.kill()
        super().__exit__(*args)


NullContext = contextlib.suppress


EXIT_SUCCESS = 0
EXIT_FAILURE = 1
buffer_size = 1024


def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--port', default=random.randint(2**10,2**16), type=int)
    exec_group = parser.add_mutually_exclusive_group(required=True)
    exec_group.add_argument('executable', nargs='?', help="The server to run")
    exec_group.add_argument('--no-spawn', dest='spawn', default=True, action='store_false', help="Do not spawn the server")
    args = parser.parse_args()

    with KillOnExit([args.executable, str(args.port)]) if args.spawn else NullContext():
        conn = socket.create_connection(('localhost', args.port), timeout=2)
        conn.settimeout(.5)
        conn.send('Request\r\n\r\n'.encode())
        time.sleep(.5)  # Attempt to gracefuly handle all kinds of multi-packet replies...
        reply = conn.recv(1024)
        print(reply)
        return EXIT_SUCCESS if re.match(br'HTTP/1.[01] 400.*\r\n\r\n', reply, re.DOTALL) else EXIT_FAILURE


if __name__ == '__main__':
    import sys
    sys.exit(main())
