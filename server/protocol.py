import subprocess
import json
import logging
import threading


class Agent:
    def __init__(self, executable, name):
        self.executable = executable
        self.logger = logging.getLogger('{}.{}'.format(__name__, name))

    def __enter__(self):
        self.subprocess = subprocess.Popen(
            self.executable,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            shell=True,
            bufsize=1,
        )
        self.logger_thread = threading.Thread(
            target=self.pull_logs
        )
        self.logger_thread.start()
        return self

    def __exit__(self, *args):
        self.subprocess.stdin.close()
        self.subprocess.wait(1)
        self.logger_thread.join(1)

    def pull_logs(self):
        self.logger.info('starting to pull logs')
        for line in map(str.strip, self.subprocess.stderr):
            self.logger.info('subprocess log: {}'.format(line))

    def call(self, method, **kwargs):
        request = json.dumps({
            'jsonrpc': '2.0',
            'id': 0,
            'method': str(method),
            'params': kwargs,
        })
        self.logger.debug('sending {}'.format(request))
        self.subprocess.stdin.write(request + '\n')
        self.subprocess.stdin.flush()
        response = self.subprocess.stdout.readline()
        self.logger.debug('received {}'.format(response))
        response = json.loads(response)
        if 'result' in response:
            return response['result']
        elif 'error' in response and isinstance(response['error'], dict):
            error = response['error']
            self.logger.error('agent error: {} {}: {}'.format(
                error.get('code'), error.get('message'), error.get('data')))
            raise RuntimeError
        else:
            self.logger.error('agent error: invalid response: {}'.format(response))
            raise RuntimeError
