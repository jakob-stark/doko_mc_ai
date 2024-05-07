import json
import random
import logging

import game


logging.basicConfig(level=logging.INFO)


def run(args):
    g = game.Game(args)
    g.run()


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser('dokod')
    parser.add_argument(
        '--mode', '-m', choices=['team', 'shuffle'], default='shuffle')
    parser.add_argument(
        '--number', '-n', type=int, default=100
    )
    parser.add_argument('agent1', type=str)
    parser.add_argument('agent2', type=str)

    args = parser.parse_args()
    run(args)
